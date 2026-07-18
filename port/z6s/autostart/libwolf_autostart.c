/*
 * EasyUI startup-library proxy for the NOVA3D Z6S.
 *
 * The host GUI is linked against musl.  To avoid mixing a glibc shared object
 * into that process this file is built without libc; its only dynamic imports
 * are dlopen() and dlsym(), already provided by the host loader.  Small process
 * operations use the ARM EABI Linux syscall interface directly.
 */

typedef unsigned long zsize_t;

extern void *dlopen(const char *filename, int flags);
extern void *dlsym(void *handle, const char *symbol);

#define RTLD_NOW    2
#define RTLD_GLOBAL 0x100

#define SYS_exit      1
#define SYS_fork      2
#define SYS_write     4
#define SYS_open      5
#define SYS_close     6
#define SYS_execve   11
#define SYS_setsid   66
#define SYS_nanosleep 162

#define O_WRONLY 1
#define O_CREAT  0100
#define O_APPEND 02000

struct z6s_timespec
{
    long tv_sec;
    long tv_nsec;
};

typedef int (*easyui_hook)(void *, void *, void *, void *);

static void *g_vendor_library;
static easyui_hook g_vendor_init;
static easyui_hook g_vendor_deinit;
static int g_spawned;

static long z6s_syscall0(long number)
{
    register long r7 __asm__("r7") = number;
    register long r0 __asm__("r0");
    __asm__ volatile("svc 0" : "=r"(r0) : "r"(r7) : "memory");
    return r0;
}

static long z6s_syscall1(long number, long a0)
{
    register long r7 __asm__("r7") = number;
    register long r0 __asm__("r0") = a0;
    __asm__ volatile("svc 0" : "+r"(r0) : "r"(r7) : "memory");
    return r0;
}

static long z6s_syscall2(long number, long a0, long a1)
{
    register long r7 __asm__("r7") = number;
    register long r0 __asm__("r0") = a0;
    register long r1 __asm__("r1") = a1;
    __asm__ volatile("svc 0" : "+r"(r0) : "r"(r1), "r"(r7) : "memory");
    return r0;
}

static long z6s_syscall3(long number, long a0, long a1, long a2)
{
    register long r7 __asm__("r7") = number;
    register long r0 __asm__("r0") = a0;
    register long r1 __asm__("r1") = a1;
    register long r2 __asm__("r2") = a2;
    __asm__ volatile("svc 0" : "+r"(r0)
                     : "r"(r1), "r"(r2), "r"(r7) : "memory");
    return r0;
}

static zsize_t z6s_strlen(const char *text)
{
    zsize_t length = 0;
    while(text[length]) ++length;
    return length;
}

static void z6s_log(const char *text)
{
    static const char path[] = "/mnt/extsd/wolf3d/autostart.log";
    long fd = z6s_syscall3(SYS_open, (long) path,
                           O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd < 0) return;
    z6s_syscall3(SYS_write, fd, (long) text, (long) z6s_strlen(text));
    z6s_syscall3(SYS_write, fd, (long) "\n", 1);
    z6s_syscall1(SYS_close, fd);
}

static void z6s_resolve_vendor(void)
{
    if(g_vendor_library) return;
    g_vendor_library = dlopen("/res/lib/libzkgui.so", RTLD_NOW | RTLD_GLOBAL);
    if(!g_vendor_library)
    {
        z6s_log("cannot load /res/lib/libzkgui.so");
        return;
    }
    g_vendor_init = (easyui_hook) dlsym(g_vendor_library, "onEasyUIInit");
    g_vendor_deinit = (easyui_hook) dlsym(g_vendor_library,
                                          "onEasyUIDeinit");
}

static void z6s_start_launcher(void)
{
    static char busybox[] = "/mnt/extsd/busybox";
    static char argv0[] = "busybox";
    static char argv1[] = "sh";
    static char script[] = "/mnt/extsd/wolf3d/run_wolf3d.sh";
    static char env_path[] =
        "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/mnt/extsd";
    static char env_home[] = "HOME=/mnt/extsd/wolf3d";
    char *argv[] = {argv0, argv1, script, (char *) 0};
    char *envp[] = {env_path, env_home, (char *) 0};
    struct z6s_timespec delay;

    if(g_spawned) return;
    g_spawned = 1;
    long pid = z6s_syscall0(SYS_fork);
    if(pid != 0)
    {
        if(pid < 0) z6s_log("fork failed");
        else z6s_log("launcher child created");
        return;
    }

    z6s_syscall0(SYS_setsid);
    delay.tv_sec = 8;
    delay.tv_nsec = 0;
    z6s_syscall2(SYS_nanosleep, (long) &delay, 0);
    z6s_syscall3(SYS_execve, (long) busybox, (long) argv, (long) envp);
    z6s_log("execve busybox launcher failed");
    z6s_syscall1(SYS_exit, 127);
    for(;;) { }
}

__attribute__((visibility("default")))
int onEasyUIInit(void *a0, void *a1, void *a2, void *a3)
{
    int result = 0;
    z6s_resolve_vendor();
    if(g_vendor_init) result = g_vendor_init(a0, a1, a2, a3);
    z6s_start_launcher();
    return result;
}

__attribute__((visibility("default")))
int onEasyUIDeinit(void *a0, void *a1, void *a2, void *a3)
{
    z6s_resolve_vendor();
    if(g_vendor_deinit) return g_vendor_deinit(a0, a1, a2, a3);
    return 0;
}
