#include "syscall.h"
#include "common.h"
#include "stdio.h"
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

int check[100] = {0}; // Kiểm tra xem PID đã được đặt báo thức chưa
int __sys_settimer(struct pcb_t *caller, struct sc_regs *regs) {
    static uint32_t alarm_time[100] = {0};  // Mỗi PID đặt riêng
    uint32_t pid = caller->pid;
    int tick = 0;
    if (regs->a1 > 0 && check[pid] == 0) {
        alarm_time[pid] = tick + regs->a1;
        printf("[PID %d] Alarm set at tick %d (current tick: %d)\n",
               pid, alarm_time[pid], tick);
        for (int i = regs->a1; i > 0; i--) {
            printf("[PID %d] Alarm pending... (%d ticks remaining)\n",
                   pid, i);
            
        }
        check[pid] = 1; // Đánh dấu PID đã được đặt báo thức
        printf("[PID %d] 🔔 Alarm ringing at tick %d!\n", pid, regs->a1);
    }
    return 0;
}
