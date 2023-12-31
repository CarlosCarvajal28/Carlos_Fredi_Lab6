#include <stdio.h>
#include <zephyr.h>
#include <arch/cpu.h>
#include <unity.h>
#include "helpers.c"

#define STACKSIZE 2000

K_THREAD_STACK_DEFINE(super_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(primary_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(secondary_stack, STACKSIZE);

int super_prio = -CONFIG_NUM_COOP_PRIORITIES;

void setUp(void) {}

void tearDown(void) {}

void super_entry(struct k_thread *primary_thread, struct k_thread *secondary_thread)
{
    k_thread_suspend(primary_thread);
    k_thread_suspend(secondary_thread);
}


void test_coop(void)
{
    struct k_thread primary_thread, secondary_thread, super_thread;
    uint64_t start, primary, secondary, end, elapsed;
    k_thread_runtime_stats_t pri_stats, sec_stats, start_stats, end_stats;

    k_thread_runtime_stats_all_get(&start_stats);
    // Meta-IRQ is enabled, forcing this to always preempt.
    k_thread_create(&super_thread, 
                    super_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) super_entry,
                    &primary_thread,
                    &secondary_thread,
                    NULL,
                    super_prio,
                    0,
                    K_MSEC(100)
                    );

    k_thread_create(&primary_thread, 
                    primary_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) busy_yield,
                    NULL,
                    NULL,
                    NULL,
                    K_PRIO_COOP(2),
                    0,
                    K_MSEC(10)
                    );

    k_thread_create(&secondary_thread, 
                    secondary_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) busy_sleep,
                    NULL,
                    NULL,
                    NULL,
                    K_PRIO_COOP(2),
                    0,
                    K_MSEC(15)
                    );

    k_thread_join(&super_thread, K_MSEC(15000));

    k_thread_runtime_stats_get(&primary_thread, &pri_stats);
    k_thread_runtime_stats_get(&secondary_thread, &sec_stats);
    k_thread_runtime_stats_all_get(&end_stats);

    start = timing_cycles_to_ns(start_stats.execution_cycles) / 1000;
    primary = timing_cycles_to_ns(pri_stats.execution_cycles) / 1000;
    secondary = timing_cycles_to_ns(sec_stats.execution_cycles) / 1000;
    end = timing_cycles_to_ns(end_stats.execution_cycles) / 1000;
    elapsed = end - start;

    k_thread_abort(&primary_thread);
    k_thread_abort(&secondary_thread);
}

void test_preempt(void)
{
    struct k_thread primary_thread, secondary_thread, super_thread;
    uint64_t start, primary, secondary, end, elapsed;
    k_thread_runtime_stats_t pri_stats, sec_stats, start_stats, end_stats;

    k_thread_runtime_stats_all_get(&start_stats);
    // Meta-IRQ is enabled, forcing this to always preempt.
    k_thread_create(&super_thread, 
                    super_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) super_entry,
                    &primary_thread,
                    &secondary_thread,
                    NULL,
                    super_prio,
                    0,
                    K_MSEC(100)
                    );

    k_thread_create(&primary_thread, 
                    primary_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) busy_yield,
                    NULL,
                    NULL,
                    NULL,
                    K_PRIO_PREEMPT(2),
                    0,
                    K_MSEC(10)
                    );

    k_thread_create(&secondary_thread, 
                    secondary_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) busy_yield,
                    NULL,
                    NULL,
                    NULL,
                    K_PRIO_PREEMPT(2),
                    0,
                    K_MSEC(15)
                    );

    k_thread_join(&super_thread, K_MSEC(10000));

    k_thread_runtime_stats_get(&primary_thread, &pri_stats);
    k_thread_runtime_stats_get(&secondary_thread, &sec_stats);
    k_thread_runtime_stats_all_get(&end_stats);

    start = timing_cycles_to_ns(start_stats.execution_cycles) / 1000;
    primary = timing_cycles_to_ns(pri_stats.execution_cycles) / 1000;
    secondary = timing_cycles_to_ns(sec_stats.execution_cycles) / 1000;
    end = timing_cycles_to_ns(end_stats.execution_cycles) / 1000;
    elapsed = end - start;

    k_thread_abort(&primary_thread);
    k_thread_abort(&secondary_thread);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_coop);
    RUN_TEST(test_preempt);

    return UNITY_END();
}