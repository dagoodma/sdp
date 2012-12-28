/**********************************************************************
 Module
   Item.c

 Revision
   1.0.0

 Description
   This is an item that goes in a queue.
   
 Notes
   Implemented similarly to a node in a linked list.

 History
 When           Who         What/Why
 -------------- ---         --------
 12-8-12 12:33  dagoodma    Created file.
***********************************************************************/

#include <stdlib.h>
#include "Error.h"
#include "Item.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

/**********************************************************************
 * Function: Item_create()
 * @param A pointer to an object or some data.
 * @param The sizeof that object or data.
 * @return The new item.
 * @remark none
 **********************************************************************/
Item Item_create(void* data, size_t size) {
    Item i;
    i = (Item) malloc(sizeof(ITEM));
    if (i == NULL) {
        error(ERROR_NO_MEMORY);
        return NULL;
    }

    i->next = NULL;
    i->last = NULL;
    i->data = data;
    i->size = size;
    return(i);
}

/**********************************************************************
 * Function: Item_destroy()
 * @param An item to destroy.
 * @return none
 * @remark Destructor for an Item.
 **********************************************************************/
void Item_destroy(Item i) {
    Item_clearNext(i);
    Item_clearLast(i);
    i->data = NULL;
    i->size = 0;
    free(i);
}

/**********************************************************************
 * Function: Item_getSize()
 * @param A item.
 * @return The size of the item and its data.
 * @remark none
 **********************************************************************/
size_t Item_getSize (Item i) {
    size_t size = sizeof(ITEM);

    if (i->size != 0)
        size += i->size;

    return size;
}

/**********************************************************************
 * Function: Item_getData()
 * @param An item.
 * @return The item's data field.
 * @remark none
 **********************************************************************/
void* Item_getData (Item i) {
    return i->data;
}

/**********************************************************************
 * Function: Item_getNext()
 * @param An item.
 * @return The item in front of this one.
 * @remark Returns NULL when this item is at the front of the line.
 **********************************************************************/
Item Item_getNext(Item i) {
    return (Item) i->next;
}

/**********************************************************************
 * Function: Item_getLast()
 * @param An item.
 * @return The item behind this one.
 * @remark Returns NULL when this item is the last in line.
 **********************************************************************/
Item Item_getLast(Item i) {
    return (Item) i->last;
}

/**********************************************************************
 * Function: Item_setNext()
 * @param An item.
 * @param An item to put in front of this one.
 * @return none
 * @remark Sets the next item, and sets the next item's last item.
 **********************************************************************/
void Item_setNext (Item i, Item next) {
    i->next = (void*)next;
    next->last = (void*)i;
}

/**********************************************************************
 * Function: Item_setLast()
 * @param An item.
 * @param An item to add behind this one.
 * @return none
 * @remark Sets the last item, and sets the last item's next item.
 **********************************************************************/
void Item_setLast (Item i, Item last) {
    i->last = (void*)last;
    last->next = (void*)i;
}

/**********************************************************************
 * Function: Item_clearNext()
 * @param An item.
 * @return none
 * @remark Removes the next item.
 **********************************************************************/
void Item_clearNext(Item i) {
    if (i->next == NULL)
        return;

    Item next = (Item) i->next;
    i->next = NULL;
    next->last = NULL;
}

/**********************************************************************
 * Function: Item_clearLast()
 * @param An item.
 * @return none
 * @remark Removes the last item.
 **********************************************************************/
void Item_clearLast(Item i) {
    if (i->last == NULL)
        return;

    Item last = (Item) i->last;
    i->last = NULL;
    last->next = NULL;
}

