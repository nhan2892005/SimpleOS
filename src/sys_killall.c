// /*
//  * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
//  */

// /* Sierra release
//  * Source Code License Grant: The authors hereby grant to Licensee
//  * personal permission to use and modify the Licensed Source Code
//  * for the sole purpose of studying while attending the course CO2018.
//  */
#include "queue.h"
#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "string.h"
#include "stdlib.h"
#include <ctype.h>
#include <stdbool.h>


int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

//     //hardcode for demo only
    uint32_t memrg = regs->a1;
    /* TODO: Get name of the target proc */
    // proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queu
    
    for (int i = 0; i < caller->running_list->size; i++) {
        struct pcb_t *proc = caller->running_list->proc[i];
        if (strcmp(proc->path, proc_name) == 0) {
            printf("Kill process PID=%d, name=\"%s\" from running_list\n", proc->pid, proc_name);
            for (int j = i; j < caller->running_list->size - 1; j++) {
                caller->running_list->proc[j] = caller->running_list->proc[j + 1];
            }
            caller->running_list->size--;
            } else {
            i++;
        }
    }
    #ifdef MLQ_SCHED
    printf("MLQ_SCHED is defined\n");
    if (caller->mlq_ready_queue != NULL) {
        for (int i = 0; i < MAX_PRIO; i++) {
            struct queue_t *q = &caller->mlq_ready_queue[i];
            for (int j = 0; j < q->size; j++) {
                struct pcb_t *proc = q->proc[j];
                if (strcmp(proc->path, proc_name) == 0) {
                    printf("Kill process PID=%d, name=\"%s\" from mlq_ready_queue[%d]\n", proc->pid, proc->path, i);
                    for (int k = j; k < q->size - 1; k++) {
                        q->proc[k] = q->proc[k + 1];
                    }
                    q->size--;
                } else {
                    j++;
                }
            }
        }
    }
    #endif
    return 0; 
}

 
