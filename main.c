/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Main Kernel Entry point
 */

#include "spede.h"
#include "global.h"
#include "kernel.h"
#include "kisr.h"
#include "kproc.h"
#include "queue.h"
#include "string.h"
#include "user_proc.h"

// Local function definitions
void kdata_init();
void idt_init();
void idt_entry_add(int entry_num, func_ptr_t func_ptr);

/**
 * Kernel data structures
 */
// Current system time
int system_time;

// Current running process ID
int run_pid;

// Process queues
queue_t available_q;
queue_t run_q;
queue_t idle_q;
queue_t sleep_q;
queue_t semaphore_q;

// Process table
pcb_t pcb[PROC_MAX];

//Semaphore Array
semaphore_t semaphores[SEMAPHORE_MAX];

//Mailbox table
mailbox_t mailboxes[MBOX_MAX];

// runtime stacks of processes
char stack[PROC_MAX][PROC_STACK_SIZE];

// Interrupt descriptor table
struct i386_gate *idt_p;

/**
 * Main entry point for our kernel/operating system
 * This function initializes all kernel data and hardware then begins
 * process scheduling and operations.
 */
int main() {
    // Initialize kernel data structures
    kdata_init();

    // Initialize the IDT
    idt_init();

    // Launch the kernel idle task
    kproc_exec("ktask_idle", &ktask_idle, &idle_q);
    kproc_exec("dispatcher_proc", &dispatcher_proc, &run_q);
    kproc_exec("printer_proc", &printer_proc, &run_q);

    // Start the process scheduler
    kproc_schedule();

    // Load the first scheduled process (effectively: the idle task)
    kproc_load(pcb[run_pid].trapframe_p);

    // should never be reached
    return 0;
}

/**
 * Kernel data structure initialization
 * Initializes all kernel data structures and variables
 */
void kdata_init() {
    int i;
    // Initialize all of our kernel queues
	sp_memset((char *)&available_q, 0, sizeof(queue_t));
	sp_memset((char *)&run_q, 0, sizeof(queue_t));
	sp_memset((char *)&idle_q, 0, sizeof(queue_t));
	sp_memset((char *)&sleep_q, 0, sizeof(queue_t));
    sp_memset((char *)&pcb, 0, sizeof(pcb));
    sp_memset((char *)&stack, 0, sizeof(stack));
    sp_memset((char *)&semaphore_q, 0, sizeof(queue_t));
    sp_memset((char *)&mailboxes, 0, sizeof(mailbox_t));
    sp_memset((char *)&semaphores, 0, sizeof(semaphore_t));

    // Ensure that all processes are initially in our available queue
    // State of processes should be AVAILABLE
	for(i = 0; i < PROC_MAX; i++){
		enqueue(&available_q, i);
		pcb[i].state = AVAILABLE;
	}

    //Initialize semaphore queue with semaphore indexes
    for(i = 0; i < SEMAPHORE_MAX; i++) {
        enqueue(&semaphore_q, i);
    }
    // Initialize system time
    // Initiallize the running pid
    system_time = 0;
    run_pid = -1;
}

/**
 * Interrupt Descriptor Table initialization
 * This adds entries to the IDT and then enables interrupts
 */
void idt_init() {
    // Get the IDT base address
    idt_p = get_idt_base();

    // Add an entry for each interrupt into the IDT
    idt_entry_add(TIMER_INTR, kisr_entry_timer);
    idt_entry_add(SYSCALL_INTR, kisr_entry_syscall);
    // Clear the PIC mask to enable interrupts
    outportb(0x21, ~1);
}


/**
 * Adds an entry to the IDT
 * @param  entry_num - interrupt/entry number
 * @param  func_ptr  - pointer to the function (interrupt service routine) when
 *                     the interrupt is triggered.
 */
void idt_entry_add(int entry_num, func_ptr_t func_ptr) {
    struct i386_gate *gateptr;

    // Ensure that we have a valid entry
    if (entry_num < 0 || entry_num > 255) {
        panic("Invalid interrupt");
    }

    // Obtain the gate pointer for the entry
    gateptr = &idt_p[entry_num];

    // Fill the gate
    fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}

/**
 * Kernel run loop
 *  - Process interrupts
 *  - Run the process scheduler
 *  - Load processes to run
 *
 * @param  trapframe - pointer to the current trapframe
 */
void kernel_run(trapframe_t *trapframe) {
    char key;
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID!");
    }

    // save the trapframe into the PCB of the currently running process
    pcb[run_pid].trapframe_p = trapframe;

    // Process the current interrupt and call the appropriate service routine
    switch (trapframe->interrupt) {
        // Timer interrupt
        case TIMER_INTR:
            kisr_timer();
            break;

        case SYSCALL_INTR:
            kisr_syscall();
            break;

        default:
            panic("Invalid interrupt");
            break;
    }

    // Process special developer/debug commands
    if (cons_kbhit()) {
        key = cons_getchar();

        switch (key) {
            case 'b':
                // Set a breakpoint
                breakpoint();
                break;

            case 'n':
                // Create a new process
                kproc_exec("user_proc", &user_proc, &run_q);
                break;

            case 'p':
                // Trigger a panic (aborts)
                panic("User requested panic!");
                break;

            case 'x':
                // Exit the currently running process
                kproc_exit();
                break;

            case 'q':
                // Exit our kernel
                cons_printf("Exiting!!!\n");
                debug_printf("Exiting!!!\n");
                exit(0);
                break;

            default:
                // Display a warning (no abort)
                panic_warn("Unknown command entered");
                break;
        }
    }

    // Run the process scheduler
    kproc_schedule();

    // Load the next process
    kproc_load(pcb[run_pid].trapframe_p);
}
