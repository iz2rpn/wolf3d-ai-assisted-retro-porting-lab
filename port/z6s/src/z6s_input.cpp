#include "z6s_input.h"

#include <SDL.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Z6S_MAX_INPUTS 12
#define Z6S_BITS_PER_LONG (sizeof(unsigned long) * 8)
#define Z6S_NBITS(x) (((x) + Z6S_BITS_PER_LONG - 1) / Z6S_BITS_PER_LONG)

enum DeviceKind { DEV_NONE, DEV_KEYBOARD, DEV_TOUCH };

struct InputDevice
{
    int fd;
    DeviceKind kind;
    char path[32];
    int xcode;
    int ycode;
    input_absinfo xi;
    input_absinfo yi;
};

static InputDevice devices[Z6S_MAX_INPUTS];
static int input_mode = Z6S_INPUT_GAME;
static int initialized;
static unsigned long last_scan_ms;

static int touch_raw_x;
static int touch_raw_y;
static int touch_x;
static int touch_y;
static int touch_start_x;
static int touch_start_y;
static int touch_down;
static int touch_report_down;
static int touch_key = SDLK_UNKNOWN;
static unsigned long touch_start_ms;
static int menu_tap_pending;
static int menu_tap_x;
static int menu_tap_y;

static unsigned long now_ms(void)
{
    timeval tv;
    gettimeofday(&tv, 0);
    return (unsigned long)tv.tv_sec * 1000UL + (unsigned long)tv.tv_usec / 1000UL;
}

static int bit_is_set(unsigned int bit, const unsigned long *bits)
{
    return (bits[bit / Z6S_BITS_PER_LONG] >> (bit % Z6S_BITS_PER_LONG)) & 1UL;
}

static void log_device(const char *kind, const char *path, const char *name)
{
    FILE *fp = fopen("/mnt/extsd/wolf3d/input.log", "a");
    if(!fp) return;
    fprintf(fp, "%s %s %s\n", kind, path, name);
    fclose(fp);
}

static int path_is_open(const char *path)
{
    for(int i = 0; i < Z6S_MAX_INPUTS; ++i)
        if(devices[i].fd >= 0 && !strcmp(devices[i].path, path))
            return 1;
    return 0;
}

static int free_slot(void)
{
    for(int i = 0; i < Z6S_MAX_INPUTS; ++i)
        if(devices[i].fd < 0) return i;
    return -1;
}

static void scan_inputs(void)
{
    for(int n = 0; n < 16; ++n)
    {
        char path[32];
        snprintf(path, sizeof(path), "/dev/input/event%d", n);
        if(path_is_open(path)) continue;

        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if(fd < 0) continue;

        unsigned long evbits[Z6S_NBITS(EV_MAX + 1)];
        unsigned long keybits[Z6S_NBITS(KEY_MAX + 1)];
        unsigned long absbits[Z6S_NBITS(ABS_MAX + 1)];
        memset(evbits, 0, sizeof(evbits));
        memset(keybits, 0, sizeof(keybits));
        memset(absbits, 0, sizeof(absbits));
        ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits);
        if(bit_is_set(EV_KEY, evbits))
            ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits);
        if(bit_is_set(EV_ABS, evbits))
            ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits);

        DeviceKind kind = DEV_NONE;
        if(bit_is_set(EV_KEY, evbits) && bit_is_set(KEY_A, keybits) &&
           bit_is_set(KEY_ENTER, keybits))
            kind = DEV_KEYBOARD;
        else if(bit_is_set(EV_ABS, evbits) &&
                (bit_is_set(ABS_MT_POSITION_X, absbits) ||
                 bit_is_set(ABS_X, absbits)))
            kind = DEV_TOUCH;

        const char *touchEnabled = getenv("Z6S_TOUCH_INPUT");
        if(kind == DEV_TOUCH && touchEnabled && !strcmp(touchEnabled, "0"))
            kind = DEV_NONE;

        if(kind == DEV_NONE)
        {
            close(fd);
            continue;
        }

        int slot = free_slot();
        if(slot < 0)
        {
            close(fd);
            return;
        }

        InputDevice &d = devices[slot];
        memset(&d, 0, sizeof(d));
        d.fd = fd;
        d.kind = kind;
        snprintf(d.path, sizeof(d.path), "%s", path);
        if(kind == DEV_TOUCH)
        {
            d.xcode = bit_is_set(ABS_MT_POSITION_X, absbits) ?
                      ABS_MT_POSITION_X : ABS_X;
            d.ycode = bit_is_set(ABS_MT_POSITION_Y, absbits) ?
                      ABS_MT_POSITION_Y : ABS_Y;
            if(ioctl(fd, EVIOCGABS(d.xcode), &d.xi) < 0)
            {
                d.xi.minimum = 0; d.xi.maximum = 4095;
            }
            if(ioctl(fd, EVIOCGABS(d.ycode), &d.yi) < 0)
            {
                d.yi.minimum = 0; d.yi.maximum = 4095;
            }
        }

        char name[128];
        memset(name, 0, sizeof(name));
        ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name);
        log_device(kind == DEV_KEYBOARD ? "keyboard" : "touch", path, name);
    }
}

static SDLKey linux_key_to_sdl(unsigned int code)
{
    switch(code)
    {
        case KEY_ESC: return SDLK_ESCAPE;
        case KEY_1: return SDLK_1; case KEY_2: return SDLK_2;
        case KEY_3: return SDLK_3; case KEY_4: return SDLK_4;
        case KEY_5: return SDLK_5; case KEY_6: return SDLK_6;
        case KEY_7: return SDLK_7; case KEY_8: return SDLK_8;
        case KEY_9: return SDLK_9; case KEY_0: return SDLK_0;
        case KEY_MINUS: return SDLK_MINUS; case KEY_EQUAL: return SDLK_EQUALS;
        case KEY_BACKSPACE: return SDLK_BACKSPACE; case KEY_TAB: return SDLK_TAB;
        case KEY_Q: return SDLK_q; case KEY_W: return SDLK_w;
        case KEY_E: return SDLK_e; case KEY_R: return SDLK_r;
        case KEY_T: return SDLK_t; case KEY_Y: return SDLK_y;
        case KEY_U: return SDLK_u; case KEY_I: return SDLK_i;
        case KEY_O: return SDLK_o; case KEY_P: return SDLK_p;
        case KEY_LEFTBRACE: return SDLK_LEFTBRACKET;
        case KEY_RIGHTBRACE: return SDLK_RIGHTBRACKET;
        case KEY_ENTER: return SDLK_RETURN;
        case KEY_LEFTCTRL: return SDLK_LCTRL;
        case KEY_A: return SDLK_a; case KEY_S: return SDLK_s;
        case KEY_D: return SDLK_d; case KEY_F: return SDLK_f;
        case KEY_G: return SDLK_g; case KEY_H: return SDLK_h;
        case KEY_J: return SDLK_j; case KEY_K: return SDLK_k;
        case KEY_L: return SDLK_l; case KEY_SEMICOLON: return SDLK_SEMICOLON;
        case KEY_APOSTROPHE: return SDLK_QUOTE; case KEY_GRAVE: return SDLK_BACKQUOTE;
        case KEY_LEFTSHIFT: return SDLK_LSHIFT; case KEY_BACKSLASH: return SDLK_BACKSLASH;
        case KEY_Z: return SDLK_z; case KEY_X: return SDLK_x;
        case KEY_C: return SDLK_c; case KEY_V: return SDLK_v;
        case KEY_B: return SDLK_b; case KEY_N: return SDLK_n;
        case KEY_M: return SDLK_m; case KEY_COMMA: return SDLK_COMMA;
        case KEY_DOT: return SDLK_PERIOD; case KEY_SLASH: return SDLK_SLASH;
        case KEY_RIGHTSHIFT: return SDLK_RSHIFT;
        case KEY_LEFTALT: return SDLK_LALT; case KEY_SPACE: return SDLK_SPACE;
        case KEY_CAPSLOCK: return SDLK_CAPSLOCK;
        case KEY_F1: return SDLK_F1; case KEY_F2: return SDLK_F2;
        case KEY_F3: return SDLK_F3; case KEY_F4: return SDLK_F4;
        case KEY_F5: return SDLK_F5; case KEY_F6: return SDLK_F6;
        case KEY_F7: return SDLK_F7; case KEY_F8: return SDLK_F8;
        case KEY_F9: return SDLK_F9; case KEY_F10: return SDLK_F10;
        case KEY_F11: return SDLK_F11; case KEY_F12: return SDLK_F12;
        case KEY_RIGHTCTRL: return SDLK_RCTRL; case KEY_RIGHTALT: return SDLK_RALT;
        case KEY_HOME: return SDLK_HOME; case KEY_END: return SDLK_END;
        case KEY_PAGEUP: return SDLK_PAGEUP; case KEY_PAGEDOWN: return SDLK_PAGEDOWN;
        case KEY_INSERT: return SDLK_INSERT; case KEY_DELETE: return SDLK_DELETE;
        case KEY_UP: return SDLK_UP; case KEY_DOWN: return SDLK_DOWN;
        case KEY_LEFT: return SDLK_LEFT; case KEY_RIGHT: return SDLK_RIGHT;
        case KEY_KPENTER: return SDLK_KP_ENTER; case KEY_KP0: return SDLK_KP0;
        case KEY_KP1: return SDLK_KP1; case KEY_KP2: return SDLK_KP2;
        case KEY_KP3: return SDLK_KP3; case KEY_KP4: return SDLK_KP4;
        case KEY_KP5: return SDLK_KP5; case KEY_KP6: return SDLK_KP6;
        case KEY_KP7: return SDLK_KP7; case KEY_KP8: return SDLK_KP8;
        case KEY_KP9: return SDLK_KP9;
        default: return SDLK_UNKNOWN;
    }
}

static void push_key(SDLKey key, int down, unsigned int scancode)
{
    if(key == SDLK_UNKNOWN) return;
    SDL_Event event;
    memset(&event, 0, sizeof(event));
    event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.type = event.type;
    event.key.state = down ? SDL_PRESSED : SDL_RELEASED;
    event.key.keysym.scancode = (Uint8)(scancode & 0xff);
    event.key.keysym.sym = key;
    event.key.keysym.mod = KMOD_NONE;
    event.key.keysym.unicode = 0;
    SDL_PushEvent(&event);
}

static int scale_axis(int value, const input_absinfo &info, int logical_max)
{
    int span = info.maximum - info.minimum;
    if(span <= 0) return 0;
    int result = (value - info.minimum) * logical_max / span;
    if(result < 0) result = 0;
    if(result > logical_max) result = logical_max;
    return result;
}

static SDLKey game_touch_key(int x, int y)
{
    int col = x * 4 / 320;
    int row = y * 3 / 200;
    if(col < 0) col = 0; if(col > 3) col = 3;
    if(row < 0) row = 0; if(row > 2) row = 2;
    static const SDLKey grid[3][4] = {
        { SDLK_1, SDLK_2, SDLK_3, SDLK_4 },
        { SDLK_LEFT, SDLK_UP, SDLK_RIGHT, SDLK_SPACE },
        { SDLK_ESCAPE, SDLK_LCTRL, SDLK_DOWN, SDLK_RETURN }
    };
    return grid[row][col];
}

static void update_touch_position(const InputDevice &d)
{
    int x = scale_axis(touch_raw_x, d.xi, 319);
    int y = scale_axis(touch_raw_y, d.yi, 199);
    if(getenv("Z6S_TOUCH_SWAP_XY")) { int t = x; x = y; y = t; }
    if(getenv("Z6S_TOUCH_INVERT_X")) x = 319 - x;
    if(getenv("Z6S_TOUCH_INVERT_Y")) y = 199 - y;
    touch_x = x;
    touch_y = y;
}

static void touch_report(const InputDevice &d)
{
    update_touch_position(d);
    if(touch_report_down && !touch_down)
    {
        touch_down = 1;
        touch_start_x = touch_x;
        touch_start_y = touch_y;
        touch_start_ms = now_ms();
        if(input_mode == Z6S_INPUT_GAME)
        {
            touch_key = game_touch_key(touch_x, touch_y);
            push_key((SDLKey)touch_key, 1, 0);
        }
    }
    else if(touch_report_down && touch_down && input_mode == Z6S_INPUT_GAME)
    {
        SDLKey next = game_touch_key(touch_x, touch_y);
        if(next != touch_key)
        {
            push_key((SDLKey)touch_key, 0, 0);
            touch_key = next;
            push_key((SDLKey)touch_key, 1, 0);
        }
    }
    else if(!touch_report_down && touch_down)
    {
        if(input_mode == Z6S_INPUT_GAME)
        {
            push_key((SDLKey)touch_key, 0, 0);
            touch_key = SDLK_UNKNOWN;
        }
        else
        {
            int dx = touch_x - touch_start_x;
            int dy = touch_y - touch_start_y;
            unsigned long elapsed = now_ms() - touch_start_ms;
            if(dx * dx + dy * dy <= 18 * 18 && elapsed < 2000)
            {
                menu_tap_x = touch_x;
                menu_tap_y = touch_y;
                menu_tap_pending = 1;
            }
        }
        touch_down = 0;
    }
}

static void process_touch_event(InputDevice &d, const input_event &ev)
{
    if(ev.type == EV_ABS)
    {
        if(ev.code == d.xcode) touch_raw_x = ev.value;
        else if(ev.code == d.ycode) touch_raw_y = ev.value;
        else if(ev.code == ABS_MT_TRACKING_ID)
            touch_report_down = ev.value >= 0;
    }
    else if(ev.type == EV_KEY && ev.code == BTN_TOUCH)
        touch_report_down = ev.value != 0;
    else if(ev.type == EV_SYN && ev.code == SYN_REPORT)
        touch_report(d);
}

static void process_device(InputDevice &d)
{
    input_event events[16];
    for(;;)
    {
        ssize_t got = read(d.fd, events, sizeof(events));
        if(got < 0)
        {
            if(errno == EAGAIN || errno == EINTR) return;
            close(d.fd);
            d.fd = -1;
            d.kind = DEV_NONE;
            return;
        }
        if(got == 0) return;
        int count = (int)(got / sizeof(input_event));
        for(int i = 0; i < count; ++i)
        {
            if(d.kind == DEV_KEYBOARD && events[i].type == EV_KEY)
            {
                if(events[i].value == 0 || events[i].value == 1)
                    push_key(linux_key_to_sdl(events[i].code),
                             events[i].value == 1, events[i].code);
            }
            else if(d.kind == DEV_TOUCH)
                process_touch_event(d, events[i]);
        }
        if(got < (ssize_t)sizeof(events)) return;
    }
}

void Z6S_InputPump(void)
{
    if(!initialized)
    {
        for(int i = 0; i < Z6S_MAX_INPUTS; ++i)
            devices[i].fd = -1;
        initialized = 1;
        scan_inputs();
        last_scan_ms = now_ms();
    }

    unsigned long now = now_ms();
    if(now - last_scan_ms >= 2000)
    {
        scan_inputs();
        last_scan_ms = now;
    }

    for(int i = 0; i < Z6S_MAX_INPUTS; ++i)
        if(devices[i].fd >= 0)
            process_device(devices[i]);
}

void Z6S_InputSetMode(int mode)
{
    if(mode == input_mode) return;
    if(touch_key != SDLK_UNKNOWN)
    {
        push_key((SDLKey)touch_key, 0, 0);
        touch_key = SDLK_UNKNOWN;
    }
    touch_down = 0;
    touch_report_down = 0;
    menu_tap_pending = 0;
    input_mode = mode;
}

int Z6S_InputTakeMenuTap(int *x, int *y)
{
    if(!menu_tap_pending) return 0;
    if(x) *x = menu_tap_x;
    if(y) *y = menu_tap_y;
    menu_tap_pending = 0;
    return 1;
}
