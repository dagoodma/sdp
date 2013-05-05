/*
 * File: Error.h
 *
 * An error handling module.
 *
 */
#ifndef Error_H
#define Error_H

#include <stdint.h>

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/


#define GET_ERROR_MESSAGE(code)     ((const char*)ERROR_MESSAGE[code])


/**********************************************************************
 * PUBLIC VARIABLES                                                   *
 **********************************************************************/

typedef enum {
    ERROR_NONE = 0x0,
    ERROR_UNKNOWN,
    ERROR_NO_ACKNOWLEDGEMENT,
    ERROR_GPS_DISCONNECTED,
    ERROR_GPS_NOFIX,
    ERROR_NO_ORIGIN,
    ERROR_NO_STATION,
    ERROR_NO_ALTITUDE,
    ERROR_NO_HEARTBEAT,
} error_t;


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



/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

#endif

