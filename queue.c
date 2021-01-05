
/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Queue Utilities
 */

#include "queue.h"
#include "spede.h"

/**
 * Adds an item to the end of a queue
 * @param  queue - pointer to the queue
 * @param  item  - the item to add
 * @return -1 on error; 0 on success
 */
int enqueue(queue_t *queue, int item) {
    // Return an error if the queue is full
     if(queue->size == QUEUE_SIZE){
	    return -1;
     }
    // Add the item to the tail of the queue
    //gets to tail of queue in items and adds int item in there
	queue->items[queue->tail] = item;
    // Move the tail forward
	queue->tail = queue->tail + 1;
    // If we are at the end of the array, move the tail to the beginning
	if(queue->tail == QUEUE_SIZE){
		//when tail is equal to the queue size then tail has to be set to 0
		queue->tail = 0;
	}
    // Increment size (since we just added an item to the queue)
	queue->size = queue->size + 1;

    return 0;
}

/**
 * Pulls an item from the specified queue
 * @param  queue - pointer to the queue
 * @param  item - pointer to item variable
 * @return -1 on error; 0 on success
 */
int dequeue(queue_t *queue, int *item) {
    // Return an error if queue is empty
	//returns -1 fi the size of the queue is 0 meaning nothing is in there
	if(queue->size == 0){
		return -1;
	}
    // Get the item from the head of the queue
	//getitem gets the item in the head of the queue

	*item = queue->items[queue->head];
    // Move the head forward
	queue->head = queue->head + 1;
    // If we are at the end of the array, move the head to the beginning
	//check if head is at the end of the queue if it is then set it to the beginning
	if(queue->head == QUEUE_SIZE){
		queue->head = 0;
	}
    // Decrement size (since we just removed an item from the queue)
	queue->size = queue->size - 1;
    return 0;
}
