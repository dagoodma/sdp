/*
 * File: Item.h
 *
 * An item that goes into a queue. Implemented similarly to a node in a
 * linked list.
 *
 */
#ifndef Item_H
#define Item_H

#include <stdint.h>
#include "Util.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

#define QUEUE_MAX_SIZE 60 

// Types

typedef struct ITEM {

    void *data, *next, *last;
    size_t size;

} ITEM;

// avoid using asterisks everywhere
typedef ITEM *Item;

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: Item_create()
 * @param A pointer to an object or some data.
 * @param The sizeof that object or data.
 * @return The new item.
 * @remark none
 **********************************************************************/
Item Item_create(void* data, size_t size);

/**********************************************************************
 * Function: Item_getData()
 * @param An item.
 * @return The item's data field.
 * @remark none
 **********************************************************************/
void* Item_getData (Item i);


/**********************************************************************
 * Function: Item_create()
 * @param A pointer to an object or some data.
 * @param The sizeof that object or data.
 * @return The new item.
 * @remark none
 **********************************************************************/
Item Item_create(void* data, size_t size);

/**********************************************************************
 * Function: Item_destroy(Item i)
 * @param An item to destroy.
 * @return none
 * @remark Destructor for an Item.
 **********************************************************************/
void Item_destroy(Item i);

/**********************************************************************
 * Function: Item_getSize(Item i)
 * @param A Item to check the size of.
 * @return The size of the item and its data.
 * @remark none
 **********************************************************************/
size_t Item_getSize (Item i);

/**********************************************************************
 * Function: Item_getNext()
 * @param An item.
 * @return The item in front of this one.
 * @remark Returns NULL when this item is at the front of the line.
 **********************************************************************/
Item Item_getNext(Item i);

/**********************************************************************
 * Function: Item_getLast()
 * @param An item.
 * @return The item behind this one.
 * @remark Returns NULL when this item is the last in line.
 **********************************************************************/
Item Item_getLast(Item i);

/**********************************************************************
 * Function: Item_setNext()
 * @param An item.
 * @param An item to put in front of this one.
 * @return none
 * @remark Sets the next item, and sets the next item's last item.
 **********************************************************************/
void Item_setNext (Item i, Item next);

/**********************************************************************
 * Function: Item_setLast()
 * @param An item.
 * @param An item to add behind this one.
 * @return none
 * @remark Sets the last item, and sets the last item's next item.
 **********************************************************************/
void Item_setLast (Item i, Item last);

/**********************************************************************
 * Function: Item_clearNext()
 * @param An item.
 * @return none
 * @remark Removes the next item.
 **********************************************************************/
void Item_clearNext(Item i);

/**********************************************************************
 * Function: Item_clearLast()
 * @param An item.
 * @return none
 * @remark Removes the last item.
 **********************************************************************/
void Item_clearLast(Item i);


#endif


