/**
 * @file    Logger.h
 * @author  David Goodman
 *
 * @brief
 * Interface for OpenLog SD card logger.
 *
 * @details
 * Writes strings to an SD card using OpenLog format. Currently, this
 * module only works in New File mode, which creates a new file every
 * time the device is powered up, and logs any data to that file.
 *
 * @note See: https://github.com/sparkfun/OpenLog/wiki/Command-Set
 * 
 * @date April 6, 2013, 4:49 PM     -- Created
 */

#ifndef Logger_H
#define	Logger_H

#include <xc.h>
#include <stdint.h>


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/


/**********************************************************************
 * Function: Logger_init
 * @return none
 * @remark Initializes the OpenLog logger. Expects new file mode. File names
 *  look like: LOG#####.TXT. The ##### number increases every time you power
 *  up OpenLog. This number is stored in EEPROM, and must be reset once 65533
 *  is reached, or else the logger will fail to initialize.
 *  TODO: Add handling for resetting the counter.
 ***********************************************************************/
char Logger_init();


/**********************************************************************
 * Function: Logger_write
 * @param String to write to the log.
 * @return none
 * @remark Data is logged to a file created when the device is powed on.
  **********************************************************************/
void Logger_write(char *str);

#endif
