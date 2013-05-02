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
#include "Timer.h"
#include <stdbool.h>


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define WAIT_BETWEEN_CHECKS 200 // [miliseconds]
#define NUMBER_OF_TIMES_TO_CHECK 10
#define MINIMUM_POSITIVES 5


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

struct button_count{
    uint8_t okay;
    uint8_t cancel;
    uint8_t stop;
    uint8_t rescue;
    uint8_t setStationKeep;
};

struct button_pressed{
    bool okay;
    bool cancel;
    bool stop;
    bool rescue;
    bool setStationKeep;
};
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

    button_count.okay = 0;
    button_count.cancel = 0;
    button_count.stop = 0;
    button_count.rescue = 0;
    button_count.setStationKeep = 0;

    Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
}

void Interface_runSM(){
    static int count;
    if(Timer_isExpired(TIMER_INTERFACE)){
        if(count > NUMBER_OF_TIMES_TO_CHECK){
            count = 0;

            button_pressed.cancel = false;
            button_pressed.okay = false;
            button_pressed.rescue = false;
            button_pressed.setStationKeep = false;
            button_pressed.stop = false;

            if(button_count.okay > MINIMUM_POSITIVES){
                button_pressed.okay = true;
            }
            button_count.okay = 0;
            button_count.cancel = 0;
            button_count.stop = 0;
            button_count.rescue = 0;
            button_count.setStationKeep = 0;
        }

        if(READ_OKAY)
            button_count.okay++;
        if(READ_CANCEL)
            button_count.cancel++;
        if(READ_STOP)
            button_count.stop++;
        if(READ_RESCUE)
            button_count.rescue++;
        if(READ_SETSTATIONKEEP)
            button_count.setStationKeep++;

        Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
        count++;
    }
    
    
}

bool Interface_isCancelPressed(){

}
bool Interface_isOkPressed(){

}
bool Interface_isStopPressed(){

}
bool Interface_isRescuePessed(){
    
}
bool Interface_isSetStationPessed(){

}

void Interface_enableReadyLight(){

}
void Interface_disableReadyLight(){

}

void Interface_enableWaitLight(){

}
void Interface_disableWaitLight(){

}

void Interface_enableErrorLight(){

}
void Interface_disableErrorLight(){

}


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/
