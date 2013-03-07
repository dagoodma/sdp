/**********************************************************************
 Module
   Compass.c

 Author: John Ash

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
2/25/2013   11:10PM     jash        Initial Creation
***********************************************************************/

#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include <ports.h>
#include "Board.h"
#include "Uart.h"
#include "Mavlink.h"
#include "Timer.h"
#include "Xbee.h"
#include "Compass.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Compass_message_recieve_start_resuce(mavlink_start_rescue_t* packet){
    printf("Lat: %d Long: %d",packet->latitude,packet->longitude);
}
/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/
