/*
 * Copyright (C) 2025 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: The authors hereby grant to Licensee 
 * personal permission to use and modify the Licensed Source Code 
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    /* TODO: Get name of the target proc */
    //int memrg = regs.a1;
    //proc_name = libread..
    uint32_t memrg = 1;
    
    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queu

    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid 1 is \"%s\"\n", proc_name);

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */

    return 0; 
}
