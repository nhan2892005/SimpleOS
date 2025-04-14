#include "syscall.h"
#include "common.h"
#include <stdio.h>
#include "libmem.h"
#include "string.h"
#include <stdint.h>
#include <queue.h>
int __sys_xxxhandler(struct pcb_t *caller, struct sc_regs *reg) {
    long a = reg->a2;
    long b = reg->a3;

    long sum = a + b;
    reg->a4 = sum;
    // int size = caller->running_list->size;
    // for (int i = 0; i < 100; i++) {
        // char ch = caller->path[i];
        // if (ch == '\0') break;
        // printf("%c", ch);
    // }
    printf("syscall_404: %ld + %ld = %ld\n", a, b, sum);
    return 0;
}
