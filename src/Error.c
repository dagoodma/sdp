/**********************************************************************
 Module
   Error.c

 Revision
   1.0.0

 Description
   Error handling module.
   
 Notes

 History
 When           Who         What/Why
 -------------- ---         --------
 12-28-12 12:33 dagoodma    Created file.
***********************************************************************/

#include "Error.h"
#include "Util.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

static uint8_t errorCode = 0;

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: error()
 * @param An error code to set.
 * @return none
 * @remark none
 **********************************************************************/
void error(uint8_t code) {
    errorCode = code;
}

/**********************************************************************
 * Function: get_error()
 * @return The currently set error code, clearing it.
 * @remark none
 **********************************************************************/
uint8_t get_error() {
    uint8_t code = errorCode;
    errorCode = 0;

    return code;
}

/**********************************************************************
 * Function: clear_error()
 * @return none
 * @remark Clears the error code.
 **********************************************************************/
void clear_error() {
    errorCode = 0;
}


/**********************************************************************
 * Function: has_error()
 * @return Returns a true value if an error code has been set.
 * @remark none
 **********************************************************************/
uint8_t has_error() {
    return errorCode != 0;
}


