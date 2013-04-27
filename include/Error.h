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

#define ERROR_NONE                  0
#define ERROR_GPS_DISCONNECTED      1
#define ERROR_GPS_NOFIX             2
#define ERROR_INITIALIZE_TIMEDOUT   3

#define GET_ERROR_MESSAGE(code)     ((const char*)errorMessage[code])

/**********************************************************************
 * PUBLIC VARIABLES                                                   *
 **********************************************************************/

const char ERROR_MESSAGE[][] = { 
    "None",
    "GPS was disconnected",
    "GPS cannot obtain fix",
    "Initialization timed out"
};


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

#endif

