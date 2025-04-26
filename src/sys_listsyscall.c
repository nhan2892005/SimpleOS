/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */
/*
* Assignment - Operating System
* CSE - HCMUT
* Semester 242
* Group Member: 
     Nguyễn Phúc Nhân   2312438     Alloc, Free
     Cao Thành Lộc      2311942     Read - Write
*    Nguyễn Ngọc Ngữ    2312401     Syscall 
	 Phan Đức Nhã       2312410     PutAllTogether, Report 
     Đỗ Quang Long      2311896     Scheduler 
*/

#include "syscall.h"

int __sys_listsyscall(struct pcb_t *caller, struct sc_regs* reg)
{
   for (int i = 0; i < syscall_table_size; i++)
       printf("%s\n",sys_call_table[i]); 

   return 0;
}
