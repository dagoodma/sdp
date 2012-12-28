/**********************************************************************
 Module
   Queue.c

 Revision
   1.0.0

 Description
   This is a FIFO queue whose items are pointers to anything.
   
 Notes

 History
 When           Who         What/Why
 -------------- ---         --------
 12-8-12 12:33  dagoodma    Created file.
***********************************************************************/

#include <stdlib.h>
#include "Error.h"
#include "Queue.h"
#include "Util.h"
#include "Item.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


/**********************************************************************
 * Function: Queue_create()
 * @param none
 * @return The new Queue object.
 * @remark A Queue is actually a pointer to a QUEUE.
 **********************************************************************/
Queue Queue_create() {
    Queue q;
    q = (Queue) malloc(sizeof(QUEUE));

    if (q == NULL) {
        error(ERROR_NO_MEMORY);
        return NULL;
    }

    q->count = 0;
    q->front = NULL;
    q->last = NULL;
    return(q);
}

/**********************************************************************
 * Function: Queue_destroy()
 * @param A Queue to destroy.
 * @return none
 * @remark Destructor for a Queue.
 **********************************************************************/
void Queue_destroy(Queue q) { 
    Queue_clear(q);

    free(q);
}

/**********************************************************************
 * Function: Queue_isFull()
 * @param A Queue to check the capacity of.
 * @return A true or false value indicating whether the Queue is full
 *         or not.
 * @remark Checks if the Queue is full.
 **********************************************************************/
bool Queue_isFull (Queue q) { return(q->count >= QUEUE_MAX_COUNT); }

/**********************************************************************
 * Function: Queue_isEmpty()
 * @param A queue to check.
 * @return A true or false value indicating whether the Queue is empty.
 * @remark Determines whether the Queue is empty or not.
 **********************************************************************/
bool Queue_isEmpty (Queue q) { return(q->count == 0); }


/**********************************************************************
 * Function: Queue_enqueue()
 * @param A queue to add to.
 * @param A pointer to something to enqueue.
 * @return A true or false value whether the item was added.
 * @remark Adds the item to the Queue.
 **********************************************************************/
bool Queue_enqueue (Queue q, Item item) {
    if (Queue_isFull(q))
        return FAILURE;

    // Clear the item's next and last, just incase
    Item_clearNext(item);
    Item_clearLast(item);

    if (Queue_isEmpty(q)) {
        q->front = item;
        q->last = item;
    }
    else {
        // Put this new item behind the last item in the queue
        Item_setLast(q->last,item);
        q->last = item;
    }

    q->count += 1;
    //q->item[(q->count)++] = item;

    

    return SUCCESS;
}

/**********************************************************************
 * Function: Queue_dequeue()
 * @param A queue to dequeue from.
 * @return The item at the front of the queue, or failure if empty.
 * @remark Removes the item at the front (bottom) of the queue.
 **********************************************************************/
Item Queue_dequeue (Queue q) {
    if (Queue_isEmpty(q))
        return NULL;
    
    Item item = q->front;
    Item next = Item_getLast(item);
    if (item == q->last || next == NULL) {
        // Last item
        q->front = NULL;
        q->last = NULL;
    } 
    else {
        q->front = next;
        Item_clearLast(item);
    }

    q->count -= 1;

    //Item item = q->item[0];
    //q->item[(q->count)--] = '\0';
    return item;
}

/**********************************************************************
 * Function: Queue_clear()
 * @param A queue to clear.
 * @return none
 * @remark Clears the queue.
 **********************************************************************/
void Queue_clear (Queue q) {
    if (Queue_isEmpty(q))
        return;

    //q->front = NULL;
    //q->last = NULL;

    // Free up the items
    while(! Queue_isEmpty(q) ) {
        Item item = Queue_dequeue(q);
        Item_destroy(item);
    }

    //(q->item)[0] = '\0';
    //q->count = 0;
}

/**********************************************************************
 * Function: Queue_getCount()
 * @param A queue to check the count of.
 * @return The number of items in the queue.
 * @remark none
 **********************************************************************/
uint8_t Queue_getCount (Queue q) {
    return q->count;
}

/**********************************************************************
 * Function: Queue_peek()
 * @param A queue.
 * @return Returns the item at the front of the queue without removing it.
 * @remark none
 **********************************************************************/
Item Queue_peek(Queue q) {
    return q->front;
}


