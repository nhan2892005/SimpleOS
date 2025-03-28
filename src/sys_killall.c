/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

 #include "common.h"
 #include "syscall.h"
 #include "stdio.h"
 #include "libmem.h"
 #include "queue.h"
 #include <string.h>
 #include <ctype.h>
 
 // Function to check if 'substr' is contained in 'path', ignoring case.
 int path_contains(const char *path, const char *substr) {
     if (!path || !substr)
         return 0;
 
     size_t sublen = strlen(substr);
     if (sublen == 0)
         return 0;  // Empty substring is always found.
 
     for (const char *p = path; *p; p++) {
         // Check if the first character matches (ignoring case).
         if (tolower((unsigned char)*p) == tolower((unsigned char)substr[0])) {
             size_t i;
             for (i = 0; i < sublen && p[i]; i++) {
                 if (tolower((unsigned char)p[i]) != tolower((unsigned char)substr[i]))
                     break;
             }
             if (i == sublen) {
                 return 1;  // Found the substring.
             }
         }
     }
     return 0;  // Substring not found.
 }
 
 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
 {
     char proc_name[100];
     uint32_t data;
 
     //hardcode for demo only
     uint32_t memrg = regs->a1;
     
     /* TODO: Get name of the target proc */
     //proc_name = libread..
     int i = 0;
     data = 0;
     while(data != -1){
         libread(caller, memrg, i, &data);
         proc_name[i]= data;
         if(data == -1) proc_name[i]='\0';
         i++;
     }
     printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
 
     /* 
     TODO: Traverse proclist to terminate the proc
      *       stcmp to check the process match proc_name
      */
     //caller->running_list
     //caller->mlq_ready_queu
 
     /* TODO Maching and terminating 
      *       all processes with given
      *        name in var proc_name
      */
     if (caller->running_list != NULL) {
         int j = 0;
         while (j < caller->running_list->size) {
             struct pcb_t *proc = caller->running_list->proc[j];
             if (path_contains(proc->path, proc_name) == 1) {
                 printf("Killing process PID=%d, name=\"%s\" from running_list\n", proc->pid, proc->path);
                 // Giả lập kill: loại bỏ process khỏi danh sách bằng cách shift các phần tử phía sau lên
                 for (int k = j; k < caller->running_list->size - 1; k++) {
                     caller->running_list->proc[k] = caller->running_list->proc[k + 1];
                 }
                 caller->running_list->size--;
                 // Không tăng j vì phần tử mới chuyển lên vị trí j cần được kiểm tra lại
             } else {
                 j++;
             }
         }
     }
 
 #ifdef MLQ_SCHED
     if (caller->mlq_ready_queue != NULL) {
         for (int p = 0; p < MAX_PRIO; p++) {
             struct queue_t *q = &caller->mlq_ready_queue[p];
             int j = 0;
             while (j < q->size) {
                 struct pcb_t *proc = q->proc[j];
                 if (path_contains(proc->path, proc_name) == 1) {
                     printf("Killing process PID=%d, name=\"%s\" from mlq_ready_queue[%d]\n", proc->pid, proc->path, p);
                     for (int k = j; k < q->size - 1; k++) {
                         q->proc[k] = q->proc[k + 1];
                     }
                     q->size--;
                     // Không tăng j vì cần kiểm tra lại vị trí j mới
                 } else {
                     j++;
                 }
             }
         }
     }
 #endif
 
     return 0; 
 }
 