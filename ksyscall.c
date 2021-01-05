/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * System call APIs - Kernel Side
 */
#include "spede.h"
#include "kernel.h"
#include "kproc.h"
#include "string.h"
#include "queue.h"
#include "ksyscall.h"
// add ipc.h and declare mailing queues

#include "ipc.h"
int mbox_enqueue(msg_t *msg, int mbox_num);
int mbox_dequeue(msg_t *msg, int mbox_num);
/**
 * System call kernel handler: get_sys_time
 * Returns the current system time (in seconds) --> ticks/CLK_TCK
 */
void ksyscall_get_sys_time() {
    // Don't do anything if the running PID is invalid
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }
    // Copy the system time from the kernel to the
    // eax register via the running process' trapframe
    pcb[run_pid].trapframe_p->ebx = system_time/CLK_TCK;
}

/**
 * System call kernel handler: get_proc_id
 * Returns the currently running process ID
 */
void ksyscall_get_proc_pid() {
    // Don't do anything if the running PID is invalid
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }

    // Copy the running pid from the kernel to the
    // eax register via the running process' trapframe
    pcb[run_pid].trapframe_p->ebx = run_pid;
}

/**
 * System call kernel handler: get_proc_name
 * Returns the currently running process name
 */
void ksyscall_get_proc_name() {
    char *ptr;
    int rc = -1; //Default to error

    // Don't do anything if the running PID is invalid
    if((run_pid > PID_MAX) || (run_pid < 0)){
        panic("Invalid PID");
    }

    //Set pointer to the address passed in via EBX
    ptr = (char *)pcb[run_pid].trapframe_p->ebx;

    if (ptr != NULL) {

        // Copy the string name from the PCB to the destination
        sp_strcpy(ptr, pcb[run_pid].name);

        //Indicate success
        rc = 0;
    }

    //Set the return code
    pcb[run_pid].trapframe_p->eax = rc;
}

/**
 * System call kernel handler: sleep
 * Puts the currently running process to sleep
 */
void ksyscall_sleep() {
    int calc_wake_value = 0;
    // Don't do anything if the running PID is invalid
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }
    // Calculate the wake time for the currently running process
    // Store this time in the pcb's wake_time member
    calc_wake_value = system_time + CLK_TCK * pcb[run_pid].trapframe_p->ebx;
    pcb[run_pid].wake_time = calc_wake_value;
    // Move the currently running process to the sleep queue
    enqueue(&sleep_q, run_pid);
    // Change the running process state to SLEEP
    pcb[run_pid].state = SLEEPING;
    // Clear the running PID so the process scheduler will run
    run_pid = -1;
}

void ksyscall_sem_init() {
	int num;
	
	int *sem_pointer;
	//check for valid pid
	if(run_pid < 0 || run_pid > PID_MAX){
		panic("PID IS INVALID");
	}
	//check if initialized if not then will set it and gets semaphore id
	sem_pointer = (int *)pcb[run_pid].trapframe_p->ebx;
	
	if(*sem_pointer == SEMAPHORE_UNINITIALIZED){

		if(dequeue(&semaphore_q, &num) != 0){
			panic("CAN'T GET SEMAPHORE");
		}
		*sem_pointer = num;
	} else {
		num = *sem_pointer;
	}
	semaphores[num].count = 0;
	semaphores[num].init = SEMAPHORE_INITIALIZED;
	
}

void ksyscall_sem_wait() {
	int num;

	if(run_pid < 0 || run_pid > PID_MAX){
		panic("PID IS INVALID");
	}
	
	num = *(int *)pcb[run_pid].trapframe_p->ebx;
	
	if(num < 0 || num > SEMAPHORE_MAX){
		panic("SEMPAPHORE IS INVALID");
	}
	//sets process to waiting.. if there is one in wait queue it should be unscheduled
	if(semaphores[num].count > 0) {
		if(enqueue(&semaphores[num].wait_q, run_pid) != 0){
			panic("CAN'T PROCESS QUEUE");
		}
		pcb[run_pid].state = WAITING;
		run_pid = -1;
	}
	//increment everytime there is a call
	semaphores[num].count++;
}

void ksyscall_sem_post() {
	int num;
	int pid = -1;

	if(run_pid < 0 || run_pid > PID_MAX){
		panic("PID IS INVALID");
	}

	num = *(int *)pcb[run_pid].trapframe_p->ebx;
	
	if(num < 0 || num > SEMAPHORE_MAX){
		panic("SEMPAPHORE IS INVALID");
	}
	//move from wait to ready
	if(semaphores[num].wait_q.size > 0){
		if(dequeue(&semaphores[num].wait_q, &pid) != 0){
			panic("DEQUEUE CAN'T PROCESS");
		}
		
		if(enqueue(pcb[pid].queue, pid) != 0) {
			panic("CAN'T PROCESS QUEUE");
		}
		
		pcb[pid].state = READY;
	}
	
	if(semaphores[num].count > 0) {
		semaphores[num].count--;
	}
}

void ksyscall_msg_send() {
	int num;
	msg_t *msg_src = NULL;
	msg_t *msg_dest = NULL;

	int waiting_pid = -1;

	if(run_pid < 0 || run_pid > PID_MAX){
		panic("PID IS INVALID");
	}

	msg_src = (msg_t *)pcb[run_pid].trapframe_p->ebx;
	num = pcb[run_pid].trapframe_p->ecx;
	
	if(msg_src == NULL){
		panic("MAILBOX POINTER IS INVALID");
	}
	if(num < 0 || num > MBOX_MAX) {
		panic("MAILBOX IDENTIFIER IS INVALID");
	}
	
	if(mbox_enqueue(msg_src, num) != 0){
		panic("MAILBOX CAN'T UNQUEUE");
	}
	//dequeue from wait queue and set to ready
	if(mailboxes[num].wait_q.size > 0) {
		if(dequeue(&mailboxes[num].wait_q, &waiting_pid) != 0){
			panic("WAITING PID CAN'T DEQUEUE");
		}
		if(enqueue(pcb[waiting_pid].queue, waiting_pid) != 0){
			panic("CAN'T ENQUEUE WATING PID TO RUN QUEUE");
		}

		pcb[waiting_pid].state = READY;
		//msg pointer then dequeue
		msg_dest = (msg_t *)pcb[waiting_pid].trapframe_p->ebx;
		
		mbox_dequeue(msg_dest, num);
	}	 
}

void ksyscall_msg_recv() {
	int num;	
	msg_t *msg_dest = NULL;

	if(run_pid < 0 || run_pid > PID_MAX){
		panic("PID IS INVALID");
	}

	msg_dest = (msg_t *)pcb[run_pid].trapframe_p->ebx;	
	num = pcb[run_pid].trapframe_p->ecx;
	
	if(msg_dest == NULL){
		panic("MAILBOX POINTER IS INVALID");
	}
	if(num < 0 || num > MBOX_MAX) {
		panic("MAILBOX IDENTIFIER IS INVALID");
	}

	
	//dequeue if there is message and if no message set to waiting
	if(mailboxes[num].size > 0){
		if(mbox_dequeue(msg_dest, num) !=0){
			panic("MESSAGE CAN'T BE DEQUEUED");
		}
	} else{
		if(enqueue(&mailboxes[num].wait_q, run_pid) != 0){
			panic("CAN'T ENQUEUE TO WAIT QUEUE");
		}
		pcb[run_pid].state = WAITING;
		//clear run pid so another process can be scheduled
		run_pid = -1;	
	}
	
	



}

//add the mailbox enqueue and dequeue works similar to queue.c
int mbox_enqueue(msg_t *msg, int mbox_num) {

	mailbox_t *mb;
	
	if(msg == NULL){
		panic("MESSAGE IS INVALID");
	}
	if(mbox_num < 0 || mbox_num > MBOX_MAX){
		panic("MAILBOX IDENTIFIER IS INVALID");
	}
	
	

	mb = &mailboxes[mbox_num];

	if(mb->size == MBOX_SIZE){
		return -1;
	}

	msg->sender = run_pid;

	msg->time_sent = (system_time / CLK_TCK);
	
	sp_memcpy(&mb->messages[mb->tail], msg, sizeof(msg_t));

	mb->tail++;

	if(mb->tail == MBOX_SIZE){
		mb->tail = 0;
	}

	mb->size++;
	return 0;
}

int mbox_dequeue(msg_t *msg, int mbox_num){

	mailbox_t *mb;
	if(msg == NULL){
		panic("MESSAGE IS INVALID");
	}
	if(mbox_num < 0 || mbox_num > MBOX_MAX){
		panic("MAILBOX IDENTIFIER IS INVALID");
	}
	
	mb = &mailboxes[mbox_num];
	//empty mb
	if(mb->size == 0){
		return -1;
	}
	sp_memcpy(msg, &mb->messages[mb->head], sizeof(msg_t));

	mb->head++;

	if(mb->head == MBOX_SIZE){
		mb->head = 0;
	}
	mb->size--;

	msg->time_received = (system_time / CLK_TCK);
	
	return 0;

}

