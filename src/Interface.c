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
#include <stdio.h>
#include<stdlib.h>
#include<plib.h>
#include <ports.h>
#include "Board.h"
#include "Timer.h"
#include "Interface.h"
#include "Lcd.h"



/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define WAIT_BETWEEN_CHECKS 200 // [miliseconds]
#define NUMBER_OF_TIMES_TO_CHECK 10
#define MINIMUM_POSITIVES 5
#define BUTTON_BYTE_COUNT 1

/* SWITCHES */
#define OKAY_TRIS               PORTY03_TRIS // pin 35 J5-4
#define CANCEL_TRIS             PORTY04_TRIS // pin 9 J5-3
#define STOP_TRIS               PORTX12_TRIS // pin 36 J5-6
#define RESCUE_TRIS             PORTX10_TRIS // pin 37 J5-8
#define SETSTATION_TRIS         PORTX08_TRIS // pin 38 J5-10

#define READ_OKAY               PORTY03_BIT // pin 35 J5-4
#define READ_CANCEL             PORTY04_BIT // pin 9 J5-3
#define READ_STOP               PORTX12_BIT // pin 36 J5-6
#define READ_RESCUE             PORTX10_BIT // pin 37 J5-8
#define READ_SETSTATION         PORTX08_BIT // pin 38 J5-10

/* LEDS */

#define READY_TRIS              PORTZ08_TRIS // pin 2 J6-5
#define WAIT_TRIS               PORTZ06_TRIS // pin 3 J6-7
#define ERROR_TRIS              PORTZ04_TRIS // pin 4 J6-09

#define SET_READY               PORTZ08_LAT // pin 2 J6-5
#define SET_WAIT                PORTZ06_LAT // pin 3 J6-7
#define SET_ERROR               PORTZ04_LAT // pin 4 J6-09
/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static void showMessage(message_t msgCode);

/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/

struct button_count{
    uint8_t okay;
    uint8_t cancel;
    uint8_t stop;
    uint8_t rescue;
    uint8_t setStationKeep;
}buttonCount;
union button_pressed{
    struct{
        unsigned int okay :1;
        unsigned int cancel :1;
        unsigned int stop :1;
        unsigned int rescue :1;
        unsigned int setStationKeep :1;
    }flags;
    unsigned char bytes[BUTTON_BYTE_COUNT];
}buttonPressed;
void (*timerLightOffFunction)();

message_t currentMsgCode, nextMsgCode;
/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: Interface_init
 * @param None.
 * @return None.
 * @remark Initialzies the pins used by the buttons and LEDs.
 **********************************************************************/
void Interface_init(){
    OKAY_TRIS = 1;
    CANCEL_TRIS = 1;
    STOP_TRIS = 1;
    RESCUE_TRIS = 1;
    SETSTATION_TRIS = 1;

    READY_TRIS = 0;
    WAIT_TRIS = 0;
    ERROR_TRIS = 0;

    buttonCount.okay = 0;
    buttonCount.cancel = 0;
    buttonCount.stop = 0;
    buttonCount.rescue = 0;
    buttonCount.setStationKeep = 0;

    Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
}
/**********************************************************************
 * Function: Interface_runSM
 * @param None.
 * @return None.
 * @remark Must be called every cycle of the Command Center, checks the
 *  timers and reacts.
 **********************************************************************/
void Interface_runSM(){
    static int count;

   if (!Timer_isExpired(TIMER_LIGHT_HOLD)) {
        timerLightOffFunction();
        Timer_clear(TIMER_LIGHT_HOLD);
    }

    if ( Timer_isExpired(TIMER_LCD_HOLD)) {
            if (nextMsgCode != NO_MESSAGE) {
                showMessage(nextMsgCode);
                nextMsgCode = NO_MESSAGE;
            }
            Timer_clear(TIMER_LCD_HOLD);
        }

    if(!Timer_isExpired(TIMER_INTERFACE)){
        if(count > NUMBER_OF_TIMES_TO_CHECK){
            count = 0;

            buttonPressed.flags.cancel = false;
            buttonPressed.flags.okay = false;
            buttonPressed.flags.rescue = false;
            buttonPressed.flags.setStationKeep = false;
            buttonPressed.flags.stop = false;

            if(buttonCount.okay > MINIMUM_POSITIVES)
                buttonPressed.flags.okay = true;
            if(buttonCount.cancel > MINIMUM_POSITIVES)
                buttonPressed.flags.cancel = true;
            if(buttonCount.stop > MINIMUM_POSITIVES)
                buttonPressed.flags.stop = true;
            if(buttonCount.rescue > MINIMUM_POSITIVES)
                buttonPressed.flags.rescue = true;
            if(buttonCount.setStationKeep > MINIMUM_POSITIVES)
                buttonPressed.flags.setStationKeep = true;

            buttonCount.okay = 0;
            buttonCount.cancel = 0;
            buttonCount.stop = 0;
            buttonCount.rescue = 0;
            buttonCount.setStationKeep = 0;
        }

        if(READ_OKAY)
            buttonCount.okay++;
        if(READ_CANCEL)
            buttonCount.cancel++;
        if(READ_STOP)
            buttonCount.stop++;
        if(READ_RESCUE)
            buttonCount.rescue++;
        if(READ_SETSTATION)
            buttonCount.setStationKeep++;

        Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
        count++;
    }
    
    
}
/**********************************************************************
 * Function: Interface_isCancelPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isCancelPressed(){
    return buttonPressed.flags.cancel;
}
/**********************************************************************
 * Function: Interface_isOkPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isOkPressed(){
    return buttonPressed.flags.okay;
}
/**********************************************************************
 * Function: Interface_isStopPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isStopPressed(){
    return buttonPressed.flags.stop;
}
/**********************************************************************
 * Function: Interface_isRescuePessed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isRescuePessed(){
    return buttonPressed.flags.rescue;
}
/**********************************************************************
 * Function: Interface_isSetStationPessed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isSetStationPessed(){
    return buttonPressed.flags.setStationKeep;

}


/**********************************************************************
 * Function: Interface_readyLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_readyLightOn(){
    SET_READY = 1;
}
/**********************************************************************
 * Function: Interface_readyLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_readyLightOff(){
    SET_READY = 0;
}
/**********************************************************************
 * Function: Interface_waitLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_waitLightOn(){
    SET_WAIT = 1;
}
/**********************************************************************
 * Function: Interface_waitLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_waitLightOff(){
    SET_WAIT = 0;
}
/**********************************************************************
 * Function: Interface_errorLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_errorLightOn(){
    SET_ERROR = 1;
}
/**********************************************************************
 * Function: Interface_errorLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_errorLightOff(){
    SET_ERROR = 0;
}


/**********************************************************************
 * Function: Interface_readyLightOnTimer
 * @param amount of time in ms that you want the light to remain on
 * @return None.
 * @remark Turns the LED on for a certain amount of time
 **********************************************************************/
void Interface_readyLightOnTimer(uint16_t ms){
    Timer_new(TIMER_LIGHT_HOLD, ms);
    timerLightOffFunction = &Interface_readyLightOff;
    Interface_readyLightOn();
}

/**********************************************************************
 * Function: Interface_errorLightOnTimer
 * @param amount of time in ms that you want the light to remain on
 * @return None.
 * @remark Turns the LED on for a certain amount of time
 **********************************************************************/
void Interface_errorLightOnTimer(uint16_t ms){
    Timer_new(TIMER_LIGHT_HOLD, ms);
    timerLightOffFunction = &Interface_errorLightOff;
    Interface_errorLightOn();
}

/**********************************************************************
 * Function: Interface_showMessageOnTimer
 * @param message_t you want to display on a timer
 * @param amount of time in ms that you want the message to remain on
 * @return None.
 * @remark After the timer expires the next message will be displayed
 **********************************************************************/
void Interface_showMessageOnTimer(message_t msgCode, uint16_t ms){
    Timer_new(TIMER_LCD_HOLD, ms);
    // Prevent a previously held message from sticking after
    if (nextMsgCode == NO_MESSAGE)
        nextMsgCode = currentMsgCode;
    showMessage(msgCode);
}

/**********************************************************************
 * Function: Interface_showMessage
 * @param message_t you want to display
 * @return None.
 * @remark Message will be printed out to the screen
 **********************************************************************/
void Interface_showMessage(message_t msgCode){
    if ( !Timer_isExpired(TIMER_LCD_HOLD) && !Timer_isActive(TIMER_LCD_HOLD))
        showMessage(msgCode);
    else
        nextMsgCode = msgCode;
}

/**********************************************************************
 * Function: Interface_clearAll
 * @param None.
 * @return None.
 * @remark Clear the LCD and clear all messages on timers
 **********************************************************************/
void Interface_clearAll(){
    currentMsgCode = NULL;
    nextMsgCode = NULL;
    void LCD_clearDisplay();
    Timer_stop(TIMER_LCD_HOLD);
    Timer_stop(TIMER_LIGHT_HOLD);
    Timer_clear(TIMER_LIGHT_HOLD);
    Timer_clear(TIMER_LCD_HOLD);
    Interface_waitLightOff();
    Interface_errorLightOff();
    Interface_readyLightOff();
}

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

static void showMessage(message_t msgCode){
        LCD_setPosition(0,0);
        LCD_writeString(INTERFACE_MESSAGE[msgCode]);
        currentMsgCode = msgCode;
}
