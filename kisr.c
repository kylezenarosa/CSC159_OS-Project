/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Kernel Interrupt Service Routines
 */

#include "spede.h"
#include "kernel.h"
#include "kisr.h"
#include "kproc.h"
#include "queue.h"
#include "string.h"
#include "ksyscall.h"

void kisr_syscall(){
    int valueFromEAX;
    //chcek for valid pid
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }
    //pull interrupt number from eax trap frame
    valueFromEAX = pcb[run_pid].trapframe_p->eax;
    //case statement for which system kernel function to call
    switch (valueFromEAX)
    {
        case SYSCALL_PROC_EXIT:
            kproc_exit();
            break;
        case SYSCALL_GET_SYS_TIME:
            ksyscall_get_sys_time();
            break;
        case SYSCALL_GET_PROC_PID:
            ksyscall_get_proc_pid();
            break;
        case SYSCALL_GET_PROC_NAME:
            ksyscall_get_proc_name();
            break;
        case SYSCALL_SLEEP:
            ksyscall_sleep();
            break;
        case SYSCALL_SEM_INIT:
            ksyscall_sem_init();
            break;
        case SYSCALL_SEM_WAIT:
            ksyscall_sem_wait();
            break;
        case SYSCALL_SEM_POST:
            ksyscall_sem_post();
            break;
        case SYSCALL_MSG_SEND:
            ksyscall_msg_send();
            break;
        case SYSCALL_MSG_RECV:
            ksyscall_msg_recv();
            break;
        default:
            panic("Bad switch cases in KISR.c\n");
            break;
    }
}

/**
 * Kernel Interrupt Service Routine: Timer (IRQ 0)
 */
void kisr_timer() {
    // Increment the system time
    int tempPid = 0;
    int sleep_size = sleep_q.size;
    system_time += 1;
    while (sleep_size--){
        if (dequeue(&sleep_q, &tempPid) != 0) {
            panic("Error retrieving process from sleep queue");
        }
        // check to see if process needs to be woken up

        // syntax to pull wake_time
        if (pcb[tempPid].wake_time <= system_time){ //still sleeping
            if (enqueue(pcb[tempPid].queue, tempPid) != 0) {
                panic("Error adding process to its run queue");
            }
            pcb[tempPid].state = READY;
        }

        if (pcb[tempPid].wake_time > system_time)
        {
            if (enqueue(&sleep_q, tempPid) != 0) {
                panic("Error adding process to its sleep queue");
            }
        }
    }
    // check to see if PID == 0 (run_q vs idle_q)


    // If the running PID is invalid, just return
    // Since this is a hardware driven interrupt, make sure
    // to dismiss the IRQ
    if (run_pid >= 0 && run_pid < PROC_MAX)
    {

    // Increment the running process' current run time
    // Once the running process has exceeded the maximum
    // number of ticks, it needs to be unscheduled:
    //   set the total run time
    //   reset the current running time
    //   set the state to ready
    //   queue the process back into the running queue
    //   clear the running pid
        pcb[run_pid].time += 1;

        if (pcb[run_pid].time >= PROC_TICKS_MAX){
            pcb[run_pid].total_time += pcb[run_pid].time;
            pcb[run_pid].time = 0;
            pcb[run_pid].state = READY;

            enqueue(pcb[run_pid].queue, run_pid);
            run_pid = -1;
            outportb(0x20, 0x60);
        }

    // Dismiss IRQ 0 (Timer)
    }
   // else
   // {
   //     outportb(0x20, 0x60);
   //     return;
   // }
   outportb(0x20, 0x60);
}
