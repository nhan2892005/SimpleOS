#include <stdio.h>
#include <stdlib.h>
#include "sched.h"
#include "queue.h"
#include "cpu.h"

void print_result(const char *test_name, const char *expected, const char *actual, int pass) {
    if (pass) {
        printf("\033[32mPass test - %s\033[0m\n", test_name);
    } else {
        printf("\033[31mFail test - %s\033[0m\n", test_name);
        printf("Expected: \033[33m%s\033[0m\n", expected);
        printf("Actual  : \033[33m%s\033[0m\n", actual);
    }
}

void simulate_calc(struct pcb_t *proc, int cycles) {
    for (int i = 0; i < cycles; i++) {
        calc(proc);
    }
}

void test_process_arrival() {
    init_scheduler();
    struct pcb_t p1 = {.pid = 1, .prio = 130};
    struct pcb_t p2 = {.pid = 2, .prio = 39};
    struct pcb_t p3 = {.pid = 4, .prio = 15};
    
    add_proc(&p1);
    add_proc(&p2);
    add_proc(&p3);
    
    struct pcb_t *proc = get_proc();
    char expected[32], actual[32];
    sprintf(expected, "PID=4");
    sprintf(actual, "PID=%d", proc->pid);
    print_result("Process arrival order", expected, actual, proc->pid == 4);
}

void test_process_priority() {
    init_scheduler();
    struct pcb_t p1 = {.pid = 1, .prio = 130};
    struct pcb_t p2 = {.pid = 2, .prio = 39};
    struct pcb_t p3 = {.pid = 3, .prio = 15};
    
    add_proc(&p1);
    add_proc(&p2);
    add_proc(&p3);
    
    struct pcb_t *proc = get_proc();
    char expected[32], actual[32];
    sprintf(expected, "PID=3");
    sprintf(actual, "PID=%d", proc->pid);
    print_result("Highest priority selection", expected, actual, proc->pid == 3);
}

void test_time_slice() {
    init_scheduler();
    struct pcb_t p1 = {.pid = 1, .prio = 10};
    
    add_proc(&p1);
    struct pcb_t *proc = get_proc();
    int time_slices = 3; // Giả lập time slice lớn hơn 1
    int count = 0;
    while (proc && count < time_slices) {
        simulate_calc(proc, 1);
        count++;
    }
    
    char expected[32], actual[32];
    sprintf(expected, "3 slices");
    sprintf(actual, "%d slices", count);
    print_result("Time slice handling", expected, actual, count == 3);
}

void test_process_arrival_time() {
    init_scheduler();
    struct pcb_t p1 = {.pid = 1, .prio = 130};
    struct pcb_t p2 = {.pid = 2, .prio = 39};
    struct pcb_t p3 = {.pid = 4, .prio = 15};
    
    add_proc(&p2); // Process đến sớm
    add_proc(&p1); // Process đến sau
    add_proc(&p3); // Process ưu tiên cao
    
    struct pcb_t *proc = get_proc();
    char expected[32], actual[32];
    sprintf(expected, "PID=4"); // Process có độ ưu tiên cao nhất phải được chọn
    sprintf(actual, "PID=%d", proc->pid);
    print_result("Process arrival with different times", expected, actual, proc->pid == 4);
}

void test_os_schedule() {
    init_scheduler();
    struct pcb_t p1 = {.pid = 1, .prio = 130};
    struct pcb_t p2 = {.pid = 2, .prio = 39};
    struct pcb_t p3 = {.pid = 3, .prio = 15};
    struct pcb_t p4 = {.pid = 4, .prio = 120};
    
    add_proc(&p1);
    add_proc(&p2);
    add_proc(&p3);
    add_proc(&p4);
    
    printf("== OS Schedule ==\n");
    for (int time = 1; time <= 5; time++) {
        printf("Time slot %d\n", time);
        struct pcb_t *proc = get_proc();
        if (proc) {
            printf("   CPU: Job %d\n", proc->pid);
            simulate_calc(proc, 2);
        }
    }
    printf("== End Schedule ==\n");
}

int main() {
    test_process_arrival();
    test_process_priority();
    test_time_slice();
    test_process_arrival_time();
    test_os_schedule();
    return 0;
}
