#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>

static pid_t get_tid(void) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_enter (void *this_fn, void *call_site) __attribute__ ((no_instrument_function));
extern "C" void __cyg_profile_func_exit  (void *this_fn, void *call_site) __attribute__ ((no_instrument_function));

static pid_t get_tid(void)
{
    return syscall(SYS_gettid);
}

void __cyg_profile_func_enter (void *this_fn, void *call_site)
{
    printf("e %d %p %p\n", get_tid(), this_fn, call_site);
}

void __cyg_profile_func_exit  (void *this_fn, void *call_site)
{
    printf("x %d %p %p\n", get_tid(), this_fn, call_site);
}
