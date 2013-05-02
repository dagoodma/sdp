/**********************************************************************
 Module
   Interface.c

 Author: John Ash

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
 5-1-13 2:10  PM      jash        Created file.
***********************************************************************/

#include <xc.h>
#include <stdint.h>
#include <ports.h>
#include "Board.h"
#include <stdbool.h>


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
/* SWITCHES */
#define OKAY_TRIS               PORTY07_TRIS // pin 33 J6-16
#define CANCEL_TRIS             PORTY08_TRIS // pin 7 J6-15
#define STOP_TRIS               PORTY09_TRIS // pin 32 J6-14
#define RESCUE_TRIS             PORTY10_TRIS // pin 6 J6-13
#define SETSTATIONKEEP_TRIS     PORTY11_TRIS // pin 31 J6-12

#define READ_OKAY               PORTY07_BIT // pin 33 J6-16
#define READ_CANCEL             PORTY08_BIT // pin 7 J6-15
#define READ_STOP               PORTY09_BIT // pin 32 J6-14
#define READ_RESCUE             PORTY10_BIT // pin 6 J6-13
#define READ_SETSTATIONKEEP     PORTY11_BIT // pin 31 J6-12

/* LEDS */

#define READY_TRIS              PORTY12_TRIS // pin 5 J6-11
#define WAIT_TRIS               PORTZ03_TRIS // pin 30 J6-10
#define ERROR_TRIS              PORTZ04_TRIS // pin 4 J6-09

#define SET_READY               PORTY12_TRIS // pin 5 J6-11
#define SET_WAIT                PORTZ03_TRIS // pin 30 J6-10
#define SET_ERROR               PORTZ04_TRIS // pin 4 J6-09
/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/


/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Interface_init(){
    OKAY_TRIS = 1;
    CANCEL_TRIS = 1;
    STOP_TRIS = 1;
    RESCUE_TRIS = 1;
    SETSTATIONKEEP_TRIS = 1;

    READY_TRIS = 0;
    WAIT_TRIS = 0;
    ERROR_TRIS = 0;
}

void Interface_runSM(){
    
}

bool Interface_cancelWasPressed(){

}
bool Interface_okWasPressed(){

}
bool Interface_stopWasPressed(){

}
bool Interface_rescueWasPessed(){
    
}
bool Interface_setStationWasPessed(){

}

void Interface_readyLedOn(){

}
void Interface_readyLedOff(){

}

void Interface_waitLedOn(){

}
void Interface_waitLedOff(){

}

void Interface_errorLedOn(){

}
void Interface_errorLedOff(){

}


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/
