/*
* Assignment - Operating System
* CSE - HCMUT
* Semester 242
* Group Member: 
	Nguyễn Phúc Nhân   2312438     Alloc, Free
	Cao Thành Lộc      2311942     Read - Write
	Nguyễn Ngọc Ngữ    2312401     Syscall 
	Phan Đức Nhã       2312410     PutAllTogether, Report 
*   Đỗ Quang Long      2311896     Scheduler 
*/
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++) {
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i; 
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
! Last modified: 13/04/2025 by Nguyen Quang Long
 */
struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	pthread_mutex_lock(&queue_lock);
	for (int i = 0; i < MAX_PRIO; i++)
	{
		if (!empty(&mlq_ready_queue[i]))
		{
			proc = dequeue(&mlq_ready_queue[i]);
			break;
		}
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;	
}

/*
! Helper function to put a process back to run queue
 * @param proc: process to put back
! Last modified: 11/04/2025 by Nguyen Quang Long
*/
void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

/*
! Helper function to add a new process to ready queue
 * @param proc: process to add
! Last modified: 11/04/2025 by Nguyen Quang Long
*/
void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

/*
! Get a process from ready queue
 * @return: process from queue
! Last modified: 11/04/2025 by Nguyen Quang Long
*/
struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

/*
! Put a process back to run queue
 * @param proc: process to put back
! Last modified: 11/04/2025 by Nguyen Quang Long
*/
void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = & running_list;

	return put_mlq_proc(proc);
}

/*
! Add a new process to ready queue
 * @param proc: process to add
! Last modified: 11/04/2025 by Nguyen Quang Long
*/
void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = & running_list;

	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	proc = dequeue(&ready_queue);
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->running_list = & running_list;

	/* TODO: put running proc to running_list */

	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->running_list = & running_list;

	/* TODO: put running proc to running_list */

	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


