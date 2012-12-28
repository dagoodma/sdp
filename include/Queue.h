/*
 * File: Queue.h
 *
 * Basic FIFO que system with accessors.
 *
 */
#ifndef Queue_H
#define Queue_H

#include "Util.h"
#include "Item.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

#define QUEUE_MAX_COUNT 1

typedef struct QUEUE {

    Item front;
    Item last;
    uint8_t count;

} QUEUE;

// avoid using asterisks everywhere
typedef QUEUE *Queue;


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: Queue_create()
 * @param none
 * @return The new Queue object.
 * @remark A Queue is actually a pointer to a QUEUE.
 **********************************************************************/
Queue Queue_create();

/**********************************************************************
 * Function: Queue_destroy()
 * @param A Queue to destroy.
 * @return none
 * @remark Destructor for a Queue.
 **********************************************************************/
void Queue_destroy(Queue q);

/**********************************************************************
 * Function: Queue_isFull()
 * @param A Queue to check the capacity of.
 * @return A true or false value indicating whether the Queue is full
 *         or not.
 * @remark Checks if the Queue is full.
 **********************************************************************/
bool Queue_isFull (Queue q);

/**********************************************************************
 * Function: Queue_isEmpty()
 * @param A queue to check.
 * @return A true or false value indicating whether the Queue is empty.
 * @remark Determines whether the Queue is empty or not.
 **********************************************************************/
bool Queue_isEmpty (Queue q);

/**********************************************************************
 * Function: Queue_enqueue()
 * @param A queue to add to.
 * @param A pointer to something to enqueue.
 * @return A true or false value whether the item was added.
 * @remark Adds the item to the Queue.
 **********************************************************************/
bool Queue_enqueue (Queue q, Item item);

/**********************************************************************
 * Function: Queue_dequeue()
 * @param A queue to dequeue from.
 * @return The item at the front of the queue, or failure if empty.
 * @remark Removes the item at the front (bottom) of the queue.
 **********************************************************************/
Item Queue_dequeue (Queue q);

/**********************************************************************
 * Function: Queue_clear()
 * @param A queue to clear.
 * @return none
 * @remark Clears the queue.
 **********************************************************************/
void Queue_clear (Queue q);

/**********************************************************************
 * Function: Queue_getCount()
 * @param A queue to check the count of.
 * @return The number of items in the queue.
 * @remark none
 **********************************************************************/
uint8_t Queue_getCount (Queue q);

/**********************************************************************
 * Function: Queue_peek()
 * @param A queue.
 * @return Returns the item at the front of the queue without removing it.
 * @remark none
 **********************************************************************/
Item Queue_peek(Queue q);



#endif

