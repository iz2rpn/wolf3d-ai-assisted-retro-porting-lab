/*
 * Tiny synchronous SDL_mixer subset for Wolf4SDL on the Z6S.
 *
 * The vendor Linux image has an ALSA PCM device, but the recovered SDL build
 * cannot create a working audio thread.  This backend therefore generates one
 * short period at a time on the main loop and writes it through tinyalsa.
 */

#include "SDL_mixer.h"

#include <tinyalsa/pcm.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Z6SChunk
{
    Mix_Chunk pub;
    Uint16 channels;
    Uint16 bits;
    Uint32 rate;
};

struct Z6SChannel
{
    Mix_Chunk *chunk;
    Uint32 position;
    int loops;
    int group;
    Uint8 left;
    Uint8 right;
    unsigned long serial;
};

static struct pcm *g_pcm;
static Z6SChannel g_channels[MIX_CHANNELS];
static int g_channel_count = MIX_CHANNELS;
static int g_reserved;
static int g_rate;
static int g_period_frames;
static Sint16 *g_period;
static unsigned long g_serial;
static void (*g_finished)(int);
static void (*g_music)(void *, Uint8 *, int);
static void *g_music_arg;
static char g_error[192];
static int g_write_error_logged;

static void set_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_error, sizeof(g_error), fmt, ap);
    va_end(ap);
}

static void audio_log(const char *fmt, ...)
{
    FILE *fp = fopen("/mnt/extsd/wolf3d/audio.log", "a");
    if(!fp) return;

    time_t now = time(NULL);
    fprintf(fp, "[%ld] ", (long) now);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fputc('\n', fp);
    fclose(fp);
}

static Uint16 read_le16(const Uint8 *p)
{
    return (Uint16) (p[0] | ((Uint16) p[1] << 8));
}

static Uint32 read_le32(const Uint8 *p)
{
    return (Uint32) p[0] | ((Uint32) p[1] << 8) |
           ((Uint32) p[2] << 16) | ((Uint32) p[3] << 24);
}

static int read_exact(SDL_RWops *rw, void *dst, size_t size)
{
    return SDL_RWread(rw, dst, 1, (int) size) == (int) size;
}

extern "C" const char *Mix_GetError(void)
{
    return g_error[0] ? g_error : "unknown audio error";
}

extern "C" Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc)
{
    Uint8 hdr[12];
    Uint16 wav_format = 0, channels = 0, bits = 0;
    Uint32 rate = 0;
    Uint8 *sample_data = NULL;
    Uint32 sample_len = 0;

    if(!src)
    {
        set_error("Mix_LoadWAV_RW: null SDL_RWops");
        return NULL;
    }

    if(!read_exact(src, hdr, sizeof(hdr)) || memcmp(hdr, "RIFF", 4) ||
       memcmp(hdr + 8, "WAVE", 4))
    {
        set_error("Mix_LoadWAV_RW: invalid RIFF/WAVE header");
        goto fail;
    }

    for(;;)
    {
        Uint8 chunk_hdr[8];
        if(!read_exact(src, chunk_hdr, sizeof(chunk_hdr))) break;
        Uint32 len = read_le32(chunk_hdr + 4);

        if(!memcmp(chunk_hdr, "fmt ", 4))
        {
            Uint8 fmt[16];
            if(len < sizeof(fmt) || !read_exact(src, fmt, sizeof(fmt)))
            {
                set_error("Mix_LoadWAV_RW: truncated fmt chunk");
                goto fail;
            }
            wav_format = read_le16(fmt);
            channels = read_le16(fmt + 2);
            rate = read_le32(fmt + 4);
            bits = read_le16(fmt + 14);
            if(len > sizeof(fmt) && SDL_RWseek(src, len - sizeof(fmt), RW_SEEK_CUR) < 0)
            {
                set_error("Mix_LoadWAV_RW: cannot skip fmt extension");
                goto fail;
            }
        }
        else if(!memcmp(chunk_hdr, "data", 4))
        {
            sample_data = (Uint8 *) malloc(len ? len : 1);
            if(!sample_data)
            {
                set_error("Mix_LoadWAV_RW: out of memory");
                goto fail;
            }
            if(len && !read_exact(src, sample_data, len))
            {
                set_error("Mix_LoadWAV_RW: truncated data chunk");
                goto fail;
            }
            sample_len = len;
        }
        else if(SDL_RWseek(src, len, RW_SEEK_CUR) < 0)
        {
            set_error("Mix_LoadWAV_RW: cannot skip WAV chunk");
            goto fail;
        }

        if(len & 1)
            SDL_RWseek(src, 1, RW_SEEK_CUR);
        if(sample_data && wav_format) break;
    }

    if(wav_format != 1 || (channels != 1 && channels != 2) || bits != 16 ||
       !sample_data)
    {
        set_error("Mix_LoadWAV_RW: only PCM S16 mono/stereo is supported");
        goto fail;
    }

    {
        Z6SChunk *chunk = (Z6SChunk *) calloc(1, sizeof(*chunk));
        if(!chunk)
        {
            set_error("Mix_LoadWAV_RW: out of memory");
            goto fail;
        }
        chunk->pub.allocated = 1;
        chunk->pub.abuf = sample_data;
        chunk->pub.alen = sample_len;
        chunk->pub.volume = MIX_MAX_VOLUME;
        chunk->channels = channels;
        chunk->bits = bits;
        chunk->rate = rate;
        if(freesrc) SDL_FreeRW(src);
        return &chunk->pub;
    }

fail:
    free(sample_data);
    if(freesrc) SDL_FreeRW(src);
    return NULL;
}

static void finish_channel(int channel)
{
    if(channel < 0 || channel >= g_channel_count || !g_channels[channel].chunk)
        return;
    g_channels[channel].chunk = NULL;
    g_channels[channel].position = 0;
    g_channels[channel].loops = 0;
    if(g_finished) g_finished(channel);
}

extern "C" void Mix_FreeChunk(Mix_Chunk *chunk)
{
    if(!chunk) return;
    for(int i = 0; i < g_channel_count; ++i)
        if(g_channels[i].chunk == chunk) finish_channel(i);
    if(chunk->allocated) free(chunk->abuf);
    free((Z6SChunk *) chunk);
}

extern "C" int Mix_AllocateChannels(int numchans)
{
    if(numchans < 0) return g_channel_count;
    if(numchans > MIX_CHANNELS) numchans = MIX_CHANNELS;
    for(int i = numchans; i < g_channel_count; ++i) finish_channel(i);
    g_channel_count = numchans;
    if(g_reserved > g_channel_count) g_reserved = g_channel_count;
    return g_channel_count;
}

extern "C" int Mix_ReserveChannels(int num)
{
    if(num < 0) num = 0;
    if(num > g_channel_count) num = g_channel_count;
    g_reserved = num;
    return g_reserved;
}

extern "C" int Mix_GroupChannels(int from, int to, int tag)
{
    if(from < 0) from = 0;
    if(to >= g_channel_count) to = g_channel_count - 1;
    if(from > to) return 0;
    for(int i = from; i <= to; ++i) g_channels[i].group = tag;
    return to - from + 1;
}

extern "C" int Mix_GroupAvailable(int tag)
{
    for(int i = g_reserved; i < g_channel_count; ++i)
        if(g_channels[i].group == tag && !g_channels[i].chunk) return i;
    return -1;
}

extern "C" int Mix_GroupOldest(int tag)
{
    int oldest = -1;
    unsigned long serial = ~0UL;
    for(int i = g_reserved; i < g_channel_count; ++i)
    {
        if(g_channels[i].group == tag && g_channels[i].chunk &&
           g_channels[i].serial < serial)
        {
            oldest = i;
            serial = g_channels[i].serial;
        }
    }
    return oldest;
}

extern "C" int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops,
                                      int ticks)
{
    (void) ticks;
    if(!chunk || loops < -1)
    {
        set_error("Mix_PlayChannel: invalid chunk or loop count");
        return -1;
    }
    if(channel < 0)
    {
        for(int i = g_reserved; i < g_channel_count; ++i)
            if(!g_channels[i].chunk) { channel = i; break; }
    }
    if(channel < 0 || channel >= g_channel_count)
    {
        set_error("Mix_PlayChannel: no free channel");
        return -1;
    }

    if(g_channels[channel].chunk) finish_channel(channel);
    g_channels[channel].chunk = chunk;
    g_channels[channel].position = 0;
    g_channels[channel].loops = loops;
    g_channels[channel].serial = ++g_serial;
    return channel;
}

extern "C" int Mix_HaltChannel(int channel)
{
    if(channel == -1)
    {
        for(int i = 0; i < g_channel_count; ++i) finish_channel(i);
        return 0;
    }
    if(channel < 0 || channel >= g_channel_count) return -1;
    finish_channel(channel);
    return 0;
}

extern "C" int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
    if(channel < 0 || channel >= g_channel_count) return 0;
    g_channels[channel].left = left;
    g_channels[channel].right = right;
    return 1;
}

extern "C" void Mix_ChannelFinished(void (*channel_finished)(int))
{
    g_finished = channel_finished;
}

extern "C" void Mix_HookMusic(
    void (*mix_func)(void *udata, Uint8 *stream, int len), void *arg)
{
    g_music = mix_func;
    g_music_arg = arg;
}

extern "C" int Mix_OpenAudio(int frequency, Uint16 format, int channels,
                               int chunksize)
{
    if(format != AUDIO_S16LSB || channels != 2 || frequency <= 0)
    {
        set_error("Z6S audio requires signed 16-bit little-endian stereo");
        return -1;
    }

    Mix_CloseAudio();
    g_rate = frequency;
    g_period_frames = chunksize / (channels * (int) sizeof(Sint16));
    if(g_period_frames < 32) g_period_frames = 32;

    pcm_config config;
    memset(&config, 0, sizeof(config));
    config.channels = 2;
    config.rate = (unsigned int) frequency;
    config.period_size = (unsigned int) g_period_frames;
    config.period_count = 4;
    config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = (unsigned long) g_period_frames;
    config.stop_threshold = (unsigned long) g_period_frames * 4;
    config.avail_min = (unsigned long) g_period_frames;

    g_period = (Sint16 *) calloc((size_t) g_period_frames * 2, sizeof(Sint16));
    if(!g_period)
    {
        set_error("cannot allocate audio period");
        return -1;
    }

    g_pcm = pcm_open(0, 0, PCM_OUT, &config);
    if(!g_pcm || !pcm_is_ready(g_pcm))
    {
        set_error("cannot open /dev/snd/pcmC0D0p: %s",
                  g_pcm ? pcm_get_error(g_pcm) : "pcm_open returned null");
        audio_log("ALSA open failed: %s", Mix_GetError());
        if(g_pcm) pcm_close(g_pcm);
        g_pcm = NULL;
        free(g_period);
        g_period = NULL;
        return -1;
    }

    /* Linux 3.10 on the Z6S leaves this old PCM in SETUP after HW/SW params.
       Preparing explicitly avoids EBADFD on the first WRITEI ioctl. */
    if(pcm_prepare(g_pcm) < 0)
    {
        set_error("cannot prepare ALSA playback: %s", pcm_get_error(g_pcm));
        audio_log("ALSA prepare failed: %s", Mix_GetError());
        pcm_close(g_pcm);
        g_pcm = NULL;
        free(g_period);
        g_period = NULL;
        return -1;
    }

    for(int i = 0; i < MIX_CHANNELS; ++i)
    {
        memset(&g_channels[i], 0, sizeof(g_channels[i]));
        g_channels[i].left = 255;
        g_channels[i].right = 255;
    }
    g_error[0] = 0;
    g_write_error_logged = 0;
    audio_log("ALSA opened /dev/snd/pcmC0D0p rate=%d channels=2 period_frames=%d",
              g_rate, g_period_frames);
    audio_log("SDL_CreateThread audio disabled; using main-loop audio pump");
    return 0;
}

extern "C" void Mix_CloseAudio(void)
{
    Mix_HaltChannel(-1);
    if(g_pcm)
    {
        pcm_close(g_pcm);
        g_pcm = NULL;
    }
    free(g_period);
    g_period = NULL;
}

static Sint16 clamp16(int value)
{
    if(value > 32767) return 32767;
    if(value < -32768) return -32768;
    return (Sint16) value;
}

static void mix_effect_channel(int index)
{
    Z6SChannel &channel = g_channels[index];
    if(!channel.chunk) return;
    Z6SChunk *chunk = (Z6SChunk *) channel.chunk;
    const int source_channels = chunk->channels;
    const Uint32 frame_bytes = (Uint32) source_channels * sizeof(Sint16);

    for(int frame = 0; frame < g_period_frames; ++frame)
    {
        if(channel.position + frame_bytes > channel.chunk->alen)
        {
            if(channel.loops == -1 || channel.loops > 0)
            {
                if(channel.loops > 0) --channel.loops;
                channel.position = 0;
            }
            else
            {
                finish_channel(index);
                return;
            }
        }

        const Sint16 *src = (const Sint16 *)
            (const void *) (channel.chunk->abuf + channel.position);
        int src_left = src[0];
        int src_right = source_channels == 2 ? src[1] : src[0];
        int volume = channel.chunk->volume;
        int add_left = src_left * volume * channel.left / (MIX_MAX_VOLUME * 255);
        int add_right = src_right * volume * channel.right / (MIX_MAX_VOLUME * 255);
        g_period[frame * 2] = clamp16((int) g_period[frame * 2] + add_left);
        g_period[frame * 2 + 1] =
            clamp16((int) g_period[frame * 2 + 1] + add_right);
        channel.position += frame_bytes;
    }
}

extern "C" void Mix_Z6S_Pump(void)
{
    if(!g_pcm || !g_period) return;

    const int bytes = g_period_frames * 2 * (int) sizeof(Sint16);
    memset(g_period, 0, (size_t) bytes);
    if(g_music) g_music(g_music_arg, (Uint8 *) g_period, bytes);
    for(int i = 0; i < g_channel_count; ++i) mix_effect_channel(i);

    int written = pcm_writei(g_pcm, g_period, (unsigned int) g_period_frames);
    if(written < 0 && !g_write_error_logged && pcm_prepare(g_pcm) == 0)
        written = pcm_writei(g_pcm, g_period,
                             (unsigned int) g_period_frames);
    if(written < 0)
    {
        if(!g_write_error_logged)
        {
            g_write_error_logged = 1;
            audio_log("pcm_writei failed after recovery: %s; disabling audio",
                      pcm_get_error(g_pcm));
        }
        pcm_close(g_pcm);
        g_pcm = NULL;
    }
}
