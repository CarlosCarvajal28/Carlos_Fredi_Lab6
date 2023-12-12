#include <stdio.h>
#include <zephyr.h>
#include <arch/cpu.h>
#include <unity.h>

#define STACKSIZE 2000;

K_THREAD_STACK_DEFINE(primary_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(secondary_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(super_stack, STACKSIZE);

struct k_thread primary_thread, secondary_thread, super_thread;
struct k_sem semaphore;

int super_prio = -CONFIG_NUM_COOP_PRIORITIES;

uint64_t start, primary, secondary, end, elapsed;
k_thread_runtime_stats_t pri_stats, sec_stats, start_stats, end_stats;


void super_entry(struct k_thread *primary_thread, struct k_thread *secondary_thread){
    k_thread_suspend(primary_thread);
    k_thread_suspend(secondary_thread);

}

k_sem_init(&semaphore,0,1);

void test_prio_inv(void){
    

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
                    K_SEC(100)
                    )

    k_thread_create(&primary_thread, 
                    primary_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) pri_entry,
                    &semaphore,
                    NULL,
                    NULL,
                    1,
                    K_SEC(1000)
                    );
    k_thread_create(&secondary_thread, 
                    secondary_stack, 
                    STACKSIZE, 
                    (k_thread_entry_t) sec_entry,
                    &semaphore,
                    NULL,
                    NULL,
                    0,
                    K_SEC(500)
                    );
    k_thread_join(&super_thread, K_MSEC(5500));

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

void pri_entry(struct k_sem semaphore){//higher priority
    
    k_sem_take(&semaphore);

}
void sec_entry(struct k_sem semaphore){//lower priority
    
    k_sem_take(&semaphore);

}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_prio_inv);

    return UNITY_END();
}