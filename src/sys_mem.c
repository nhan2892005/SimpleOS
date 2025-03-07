/*
 * Copyright (C) 2025 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: The authors hereby grant to Licensee 
 * personal permission to use and modify the Licensed Source Code 
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "syscall.h"
#include "libmem.h"
#include "mm.h"

int __sys_memmap(struct pcb_t *caller, struct sc_regs* regs)
{
   int memop = regs->a1;
printf("DEBUG in %s memop=%d\n", __func__, memop);

   switch (memop) {
   case SYSMEM_MAP_OP:
            /* Reserved process case*/
            break;
   case SYSMEM_INC_OP:
            inc_vma_limit(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_SWP_OP:
            __mm_swap_page(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_IO_READ:
printf("DEBUG in %s memop=%d addr=%d\n", __func__, memop, regs->a2);
            BYTE value;
            MEMPHY_read(caller->mram, regs->a2, &value);
            regs->a3 = value;
printf("DEBUG in %s:%d memop=%d addr=%d after read a3=%d value=%d\n", __func__, __LINE__, memop, regs->a2, regs->a3, value);
            break;
   case SYSMEM_IO_WRITE:
printf("DEBUG in %s memop=%d addr=%d\n", __func__, memop, regs->a2);
            MEMPHY_write(caller->mram, regs->a2, regs->a3);
printf("DEBUG in %s memop=%d addr=%d after write a3=%d\n", __func__, memop, regs->a2, regs->a3);
            break;
   default:
            printf("Memop code: %d\n", memop);
            break;
   }
   
   return 0;
}


