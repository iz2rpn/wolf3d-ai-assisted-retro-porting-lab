#ifndef Z6S_MINIMAL_SDL_MIXER_H
#define Z6S_MINIMAL_SDL_MIXER_H

/*
 * Minimal SDL_mixer-compatible surface for Wolf4SDL on the NOVA3D Z6S.
 * It intentionally implements only the calls used by id_sd.cpp.
 */

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDL_MIXER_MAJOR_VERSION 1
#define SDL_MIXER_MINOR_VERSION 2
#define SDL_MIXER_PATCHLEVEL    12

#define MIX_CHANNELS       8
#define MIX_DEFAULT_FORMAT AUDIO_S16SYS
#define MIX_MAX_VOLUME     128

typedef struct Mix_Chunk
{
    int allocated;
    Uint8 *abuf;
    Uint32 alen;
    Uint8 volume;
} Mix_Chunk;

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
void Mix_CloseAudio(void);
const char *Mix_GetError(void);

Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
void Mix_FreeChunk(Mix_Chunk *chunk);

int Mix_AllocateChannels(int numchans);
int Mix_ReserveChannels(int num);
int Mix_GroupChannels(int from, int to, int tag);
int Mix_GroupAvailable(int tag);
int Mix_GroupOldest(int tag);

int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);
#define Mix_PlayChannel(channel, chunk, loops) \
    Mix_PlayChannelTimed((channel), (chunk), (loops), -1)

int Mix_HaltChannel(int channel);
int Mix_SetPanning(int channel, Uint8 left, Uint8 right);
void Mix_ChannelFinished(void (*channel_finished)(int channel));
void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len),
                   void *arg);

/* Z6S extension: audio is pumped synchronously from the game event loop. */
void Mix_Z6S_Pump(void);

#ifdef __cplusplus
}
#endif

#endif
