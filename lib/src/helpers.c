#include "helpers.h"
#include <zephyr.h>


void busy_busy(char *name)
{
    for (int i = 0; ; i++);
}

void busy_yield(char *name)
{
    for (int i = 0; ; i++) {
        if (!(i & 0xFF)) {
            k_yield();
        }
    }
}

void busy_sleep(char *name)
{
    k_busy_wait(10000);
    k_sleep(K_MSEC(490));
}