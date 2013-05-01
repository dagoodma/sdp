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
    ERROR_GPS_DISCONNECTED,
    ERROR_GPS_NOFIX,
    ERROR_INITIALIZE_TIMEDOUT,
} error_t;

//typedef enum error_enum  error_t ;

const char *ERROR_MESSAGE[] = {
    "None",
    "GPS was disconnected",
    "GPS cannot obtain fix",
    "Initialization timed out"
};



/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

#endif

