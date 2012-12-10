#include <stdio.h>
extern "C" void __cyg_profile_func_enter (void *this_fn, void *call_site) __attribute__ ((no_instrument_function));
extern "C" void __cyg_profile_func_exit  (void *this_fn, void *call_site) __attribute__ ((no_instrument_function));

void __cyg_profile_func_enter (void *this_fn, void *call_site)
{
    printf("e %p %p\n", this_fn, call_site);
}

void __cyg_profile_func_exit  (void *this_fn, void *call_site)
{
    printf("x %p %p\n", this_fn, call_site);
}
