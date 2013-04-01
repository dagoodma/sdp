/**********************************************************************
 Module
 Atlas.c

 Author: David Goodman

 History
 When                   Who         What/Why
 --------------         ---         --------
3/27/2013   11:10PM     dagoodma    Created project.
***********************************************************************/
#define IS_ATLAS
#define DEBUG
//#define DEBUG_VERBOSE

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>

#include "Board.h"
#include "Serial.h"
#include "Ports.h"
#include "Magnetometer.h"
#include "Drive.h"

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

static enum {
    STATE_INITIALIZE  = 0x0, // Initializing and obtaining instructions
    STATE_STATIONKEEP = 0x1, // Maintaining station coordinates
    STATE_OVERRIDE = 0x2,   // Overriden with remote control
    STATE_RESCUE = 0x3, // Rescuing a drowing person
} state;

 
/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

/***********************************************************************
 * PRIVATE FUNCTIONS                                                   *
 ***********************************************************************/


/**
 * Function: main
 * @return SUCCESS or FAILURE.
 * @remark Entry point for command center (COMPAS).
 * @author David Goodman
 * @date 2013.03.10  */
#ifdef USE_MAIN
int main(void) {
    initMasterSM();
    printf("Atlas ready for use. \n\n");
    while(1){
        runMasterSM();
    }
    return (SUCCESS);
}
#endif


/**
 * Function: initMasterSM
 * @return None.
 * @remark Initializes the master state machine for the boat.
 * @author David Goodman
 * @date 2013.03.28  */
void initMasterSM() {
    Board_init();
    Serial_init();
    Timer_init();

    #ifdef USE_MOTORS
    Drive_init();
    #endif

    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);

    #ifdef USE_MAGNETOMETER
    Magnetometer_init();
    #endif

    #ifdef USE_NAVIGATION
    GPS_init();
    Navigation_init();
    #endif
    
    startInitializeState();
}

/**
 * Function: runMasterSM
 * @return None.
 * @remark Executes one cycle of the boat's state machine.
 * @author David Goodman
 * @date 2013.03.28  */
void runMasterSM() {
    //Serial_runSM(); // for non-blocking state machine
    Magnetometer_runSM();
    Drive_runSM();

}


void startInitializeState() {
}

