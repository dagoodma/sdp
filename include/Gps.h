/*
 * File: Gps.h
 *
 * State machine for GPS module.
 *
 */
#ifndef Gps_H
#define Gps_H

#include "Error.h"
#include "Util.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


/**********************************************************************
 * Function: GPS_init()
 * @param An options bitfield.
 * @return none
 * @remark Initializes the GPS module.
 **********************************************************************/
void GPS_init(uint8_t options);

/**********************************************************************
 * Function: GPS_isInitialized()
 * @return Whether the GPS was initialized.
 * @remark none
 **********************************************************************/
bool GPS_isInitialized();

#endif
