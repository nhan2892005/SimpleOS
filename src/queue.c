#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/*
! Empty queue check
* @param q: queue to check
* @return: 1 if empty, 0 if not
* @note: NULL queue is considered empty
* @note: This function is not thread-safe
*/
int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

/*
! Add a new process to queue
* @param q: queue to add process to
* @param proc: process to add
*/
void enqueue(struct queue_t * q, struct pcb_t * proc) {
        // * Check queue is full
        if (q->size == MAX_QUEUE_SIZE) 
                return;
        // * Put process to queue
        q->proc[q->size++] = proc;
}

struct pcb_t * dequeue(struct queue_t * q) {
        // * Check queue is empty
        if (empty(q))
        {
                return NULL;
        }

        struct pcb_t *proc = NULL;

        // ! Get the process with the highest priority

        // * Default to the first process
        int high_pri = q->proc[0]->prio;
        int high_pri_idx = 0;

        // * Find the process with the highest priority
        // * and remove it from the queue
        for (int i = 1; i < q->size; i++)
        {
                // * Check if the process is higher priority
                if (q->proc[i]->prio < high_pri)
                {       
                        // * Update the highest priority process
                        high_pri = q->proc[i]->prio;
                        high_pri_idx = i;
                }
        }

        // * Store the process to return
        proc = q->proc[high_pri_idx];

        // * Remove the process from the queue
        for (int i = high_pri_idx; i < q->size - 1; i++)
        {
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;

        // * Return the process
        return proc;
}

