/*
 * Copyright (C) 2025 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: The authors hereby grant to Licensee 
 * personal permission to use and modify the Licensed Source Code 
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "syscall.h"

int __sys_listsyscall(struct pcb_t *caller, struct sc_regs* reg)
{
   printf("DEBUG in sys_list_systemcall.c \n");
   
   for (int i = 0; i < syscall_table_size; i++)
       printf("%s\n",sys_call_table[i]); 

   return 0;
}
