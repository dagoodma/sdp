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

#define ERROR_NO_MEMORY 1
#define ERROR_BAD_CHECKSUM 2
#define ERROR_TIMER_NUMBER 3
#define ERROR_TIMER_OFF 4
#define ERROR_SERIAL_NOTREADY 5
#define ERROR_SERIAL_DISCONNECTED 6

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: error()
 * @param An error code to set.
 * @return none
 * @remark none
 **********************************************************************/
void error(uint8_t code);

/**********************************************************************
 * Function: get_error()
 * @return The currently set error code, clearing it.
 * @remark none
 **********************************************************************/
uint8_t get_error();
    

/**********************************************************************
 * Function: has_error()
 * @return Returns a true value if an error code has been set.
 * @remark none
 **********************************************************************/
uint8_t has_error();

/**********************************************************************
 * Function: clear_error()
 * @return none
 * @remark Clears the error code.
 **********************************************************************/
void clear_error();

#endif

