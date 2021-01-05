/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Kernel Process Handling
 */
#include "spede.h"
#include "kernel.h"
#include "kproc.h"
#include "queue.h"
#include "string.h"

/**
 * Process scheduler
 */
void kproc_schedule() {
    // if we already have an active process that is running, we should simply return
    //
    // dequeue a process from the running queue
    // if it is a valid process
    //   Set the active running PID
    //   Set the process state to RUNNING
    //
    // if a process cannot be dequeued, trigger a panic
    //
    // if we don't have a valid running process, trigger a panic


    //Add logic for if nothing is in run que, pull from idle.
    if (run_pid >= 0) {
        return;
    }
    if (dequeue(&run_q, &run_pid) == 0) {
        pcb[run_pid].state = RUNNING;
    }else if(dequeue(&idle_q, &run_pid) == 0){
	    pcb[run_pid].state = RUNNING;
    } else {
        panic("No tasks scheduled to run");

    }

    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }
    debug_printf("Scheduled process %s (pid=%d)\n", pcb[run_pid].name, run_pid);
}

/**
 * Start a new process
 * @param proc_name The process title
 * @param proc_ptr  function pointer for the process
 * @param queue     the run queue in which this process belongs
 */
void kproc_exec(char *proc_name, void *proc_ptr, queue_t *queue) {
    int pid = 0;
    int status_code = 0;
    // Ensure that valid parameters have been specified

    if(proc_name == 0 || proc_ptr == 0 || queue == 0) {
        panic("Error. Uninitialized Pointers!");
    }
    // Dequeue the process from the available queue
    // If a process cannot be dequeued, trigger a warning
    status_code = dequeue(&available_q, &pid);
    if(status_code != 0) {
        panic_warn("Unable to dequeue process from the available queue.");
        return;
    }
    // Initialize the PCB
    //   Set the process state to READY
    //   Initialize other process control block variables to default values
    //   Copy the process name to the PCB
    //   Ensure the stack for the process is initialized

    sp_memset(&pcb[pid], 0, sizeof(pcb_t));
    pcb[pid].state = READY;
    pcb[pid].time = 0;
    pcb[pid].total_time = 0;
    sp_strncpy(pcb[pid].name, proc_name, PROC_NAME_LEN);
    sp_memset(stack[pid], 0, sizeof(stack[pid]));

    // Allocate the trapframe data
    pcb[pid].trapframe_p = (trapframe_t *)&stack[pid][PROC_STACK_SIZE - sizeof(trapframe_t)];

    // Set the instruction pointer in the trapframe
    pcb[pid].trapframe_p->eip = (unsigned int)proc_ptr;

    // Set INTR flag
    pcb[pid].trapframe_p->eflags = EF_DEFAULT_VALUE | EF_INTR;

    // Set each segment in the trapframe
    pcb[pid].trapframe_p->cs = get_cs();
    pcb[pid].trapframe_p->ds = get_ds();
    pcb[pid].trapframe_p->es = get_es();
    pcb[pid].trapframe_p->fs = get_fs();
    pcb[pid].trapframe_p->gs = get_gs();


    // Set the process run queue (supplied as argument)
    // Move the proces into the associated run queue
    pcb[pid].queue = queue;
    enqueue(pcb[pid].queue, pid);
    debug_printf("Started process %s (pid=%d)\n", pcb[pid].name, pid);
}

/**
 * Exit the currently running process
 */
void kproc_exit() {
    //Panic if we have an invalid PID
    if(run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid process ID!");
    }
    // PID 0 should be our kernel idle task.
    // It should never exit.
    // enqueue back into idle queue
    if(run_pid == 0) {
        enqueue(&idle_q, run_pid);
        pcb[run_pid].state = READY;
    }
    else {
        debug_printf("Exiting process %s (pid=%d)\n", pcb[run_pid].name, run_pid);
        // Change the state of the running process to AVAILABLE
        // Queue it back to the available queue
        pcb[run_pid].state = AVAILABLE;
        enqueue(&available_q, run_pid);
    }
    //clear running pid
    run_pid = -1;
    // Trigger the scheduler to load the next process
    kproc_schedule();
}


/**
 * Kernel idle task
 */
void ktask_idle() {

    // Indicate that the Idle Task has started
    cons_printf("idle_task started\n");

    // Process run loop
    while (1) {
        //hlt instruction
	    asm("hlt");
    }
}
