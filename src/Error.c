/*
 * File:   Error.c
 * Author: David Goodman
 *
 * Created on May 6, 2013, 1:31 AM
 */
#include <xc.h>
#include "Board.h"
#include "Error.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

const char *ERROR_MESSAGE[] = {
    "None",
    "An unknown error occured",
    "Never received acknowledgment from boat",
    "GPS was disconnected",
    "GPS cannot obtain fix",
    "Never received origin from command center",
    "Never received station from command center",
    "Stopped receiving altitude data from boat",
    "Lost connection to boat"
};

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**********************************************************************
 * Function: getErrorMessage
 * @param Error code to retrieve the message for.
 * @return String representing error message.
 * @remark 
 **********************************************************************/
const char *getErrorMessage(error_t errorCode) {
    return ERROR_MESSAGE[errorCode];
}

