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
#include <stdlib.h>
#include <plib.h>
#include "ports.h"
#include "Board.h"
#include "Timer.h"
#include "Interface.h"
#include "Lcd.h"
#include "Error.h"



/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define WAIT_BETWEEN_CHECKS 20 // [miliseconds]
#define NUMBER_OF_TIMES_TO_CHECK 8
#define MINIMUM_POSITIVES 6
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

#define PRESSED                 0 // buttons active low
/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static void showMessage(message_t msgCode);
static int count;

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
    } flags;
    unsigned char bytes[BUTTON_BYTE_COUNT];
} buttonPressed;

void (*timerLightOffFunction)();


const char *INTERFACE_MESSAGE[] = {
  //"....................\n"  <- maxiumum line length
    "Blank message.",
    "Calibration success.",
    "Please calibrate the\npitch by leveling\nwith horizon, until\nboth top lights on.",
    "Please calibrate the\nyaw by being\nlevel and north,\nuntil both lights on.",
    "Command center ready",
    "Sending boat to\nrescue person.",
    "Boat started rescue.",
    "Boat rescued person.",
    "Are you sure you\nwant to cancel the\nrescue?",
    "Sending boat to\nstation.",
    "Boat is headed\nto station.",
    "Stopping the boat.\n",
    "Boat has stopped.\n",
    "Are you sure you\nwant to cancel the\nstop?",
    "Saving boat's posit-\nion as new station.",
    "Saved new station.",
    "Set new station.",
    "Setting boat origin.",
    "Set new origin.",
    "Boat is now online.\n"
};

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
    count = 0;

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

   if (Timer_isExpired(TIMER_LIGHT_HOLD)) {
        timerLightOffFunction();
        Timer_clear(TIMER_LIGHT_HOLD);
    }

    if (Timer_isExpired(TIMER_LCD_HOLD)) {
        if (nextMsgCode != NO_MESSAGE) {
            showMessage(nextMsgCode);
            nextMsgCode = NO_MESSAGE;
        }
        Timer_clear(TIMER_LCD_HOLD);
    }

    if(Timer_isExpired(TIMER_INTERFACE)) {

        if(READ_OKAY == PRESSED)
            buttonCount.okay += 1;
        if(READ_CANCEL == PRESSED)
            buttonCount.cancel += 1;
        if(READ_STOP == PRESSED)
            buttonCount.stop += 1;
        if(READ_RESCUE == PRESSED)
            buttonCount.rescue += 1;
        if(READ_SETSTATION == PRESSED)
            buttonCount.setStationKeep += 1;
        
        count++;

        if(count >= NUMBER_OF_TIMES_TO_CHECK){
            count = 0;

            // Clear button flags
            buttonPressed.flags.cancel = false;
            buttonPressed.flags.okay = false;
            buttonPressed.flags.rescue = false;
            buttonPressed.flags.setStationKeep = false;
            buttonPressed.flags.stop = false;

            if(buttonCount.okay >= MINIMUM_POSITIVES)
                buttonPressed.flags.okay = true;
            if(buttonCount.cancel >= MINIMUM_POSITIVES)
                buttonPressed.flags.cancel = true;
            if(buttonCount.stop >= MINIMUM_POSITIVES)
                buttonPressed.flags.stop = true;
            if(buttonCount.rescue >= MINIMUM_POSITIVES)
                buttonPressed.flags.rescue = true;
            if(buttonCount.setStationKeep >= MINIMUM_POSITIVES)
                buttonPressed.flags.setStationKeep = true;

            // Reset button counts
            buttonCount.okay = 0;
            buttonCount.cancel = 0;
            buttonCount.stop = 0;
            buttonCount.rescue = 0;
            buttonCount.setStationKeep = 0;
        }

        Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
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
 * Function: Interface_isRescuePressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isRescuePressed(){
    return buttonPressed.flags.rescue;
}
/**********************************************************************
 * Function: Interface_isSetStationPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isSetStationPressed(){
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
 * Function: Interface_waitLightOnTimer
 * @param amount of time in ms that you want the light to remain on
 * @return None.
 * @remark Turns the LED on for a certain amount of time
 **********************************************************************/
void Interface_waitLightOnTimer(uint16_t ms){
    Timer_new(TIMER_LIGHT_HOLD, ms);
    timerLightOffFunction = &Interface_waitLightOff;
    Interface_waitLightOn();
}


/**********************************************************************
 * Function: Interface_pitchLightsOff
 * @return None.
 * @remark Turns pitch calibration lights off.
 **********************************************************************/
void Interface_pitchLightsOff() {
    // Does nothing yet
}

/**********************************************************************
 * Function: Interface_yawLightsOff
 * @return None.
 * @remark Turns yaw calibration lights off.
 **********************************************************************/
void Interface_yawLightsOff() {
    // Does nothing yet
}

/**********************************************************************
 * Function: Interface_pitchLightsOn
 * @return None.
 * @remark Turns pitch calibration lights on, which will use both
 *  top calibration LEDs to signal to the user when the scope is level
 *  by lighting both lights. If the scope is not level, the lights
 *  will indicate which way the scope should be pitched.
 **********************************************************************/
void Interface_pitchLightsOn() {
    // Does nothing yet
}

/**********************************************************************
 * Function: Interface_yawLightsOn
 * @return None.
 * @remark Turns yaw calibration lights on, which will use both
 *  top calibration LEDs to signal to the user when the scope is facing
 *  true North, by turning both LEDs on.
 **********************************************************************/
void Interface_yawLightsOn() {
    // Does nothing yet
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
 * Function: Interface_showErrorMessage
 * @param Error code for the error message to print.
 * @return None.
 * @remark Prints an error code to the LCD screen, and turns on the
 *  error LED, while clearing all other lights and messages.
 **********************************************************************/
void Interface_showErrorMessage(error_t errorCode) {
    Interface_clearAll();
    Interface_errorLightOn();
    LCD_setPosition(0,0);
    LCD_writeString(getErrorMessage(errorCode));
    currentMsgCode = errorCode;
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


//TEST MODULES


//#define TEST_PUSHBUTTONS
#ifdef TEST_PUSHBUTTONS
int main(void) {
    //initializations
    Board_init();
    Serial_init();
    Timer_init();
    Interface_init();

    printf("INITIALIZATIONS COMPLETE\n");
    enum {
        CANCEL  = 0x01,
        OK      = 0x02,
        STOP    = 0x03,
        RESCUE  = 0x04,
        SET_STATION = 0x05,
        IDLE        = 0x06,
    } push_state;

    uint16_t onTime = 3000;

    //cycle and check if buttons are pressed, if so, turn light on for 3 seconds
    while(1){
        Interface_runSM();
        //check to see which button is pressed
        if(Interface_isCancelPressed()){
            push_state = CANCEL;
            printf("CANCEL\n");
        }else if(Interface_isOkPressed()){
            push_state = OK;
            printf("OK\n");
        }else if(Interface_isStopPressed()){
            push_state = STOP;
            printf("STOP\n");
        }else if(Interface_isRescuePressed()){
            push_state = RESCUE;
            printf("RESCUE\n");
        }else if(Interface_isSetStationPressed()){
            push_state = SET_STATION;
            printf("SET_STATION\n");
        }else{
            push_state = IDLE;
            //printf("IDLE\n");
        }

        switch(push_state){
            case CANCEL:
                Interface_errorLightOnTimer(onTime);
                printf("BUTTON PRESSED : CANCEL\n\n");
                push_state = IDLE;
                break;

            case OK:
                Interface_readyLightOnTimer(onTime);
                printf("BUTTON PRESSED : OK\n\n");
                push_state = IDLE;
                break;

            case STOP:
                Interface_waitLightOnTimer(onTime);
                printf("BUTTON PRESSED : STOP\n\n");
                push_state = IDLE;
                break;

            case RESCUE:
                Interface_errorLightOnTimer(onTime);
                Interface_readyLightOnTimer(onTime);
                Interface_waitLightOnTimer(onTime);
                printf("BUTTON PRESSED : RESCUE\n\n");
                push_state = IDLE;
                break;

            case SET_STATION:
                Interface_errorLightOnTimer(onTime);
                Interface_waitLightOnTimer(onTime);
                printf("BUTTON PRESSED : SET_STATION\n\n");
                push_state = IDLE;
                break;

            case IDLE:
                ;
                break;
        }


    }


}

#endif


//#define TEST_INTERFACE
#ifdef TEST_INTERFACE
int main(void) {
    //initializations
    Board_init();
    Serial_init();
    Timer_init();
    LCD_init();
    Interface_init();
    
    printf("INITIALIZATIONS COMPLETE\n");
    LCD_writeString("Interface online.\n");

    enum {
        CANCEL  = 0x01,
        OK      = 0x02,
        STOP    = 0x03,
        RESCUE  = 0x04,
        SETSTATION = 0x05,
        IDLE        = 0x06,
    } state;

    //cycle and check if buttons are pressed, if so, turn light on for 3 seconds
    while(1) {
        //check to see which button is pressed
        if(Interface_isCancelPressed() && state != CANCEL){
            state = CANCEL;
            LCD_setPosition(0,0);
            LCD_writeString("Cancel.\n");
            printf("Cancel\n");
        }else if(Interface_isOkPressed()  && state != OK){
            state = OK;
            LCD_setPosition(0,0);
            LCD_writeString("Ok.\n");
            printf("Ok\n");
        }else if(Interface_isStopPressed() && state != STOP){
            state = STOP;
            LCD_setPosition(0,0);
            LCD_writeString("Stop.\n");
            printf("Stop\n");
        }else if(Interface_isRescuePressed() && state != RESCUE){
            state = RESCUE;
            LCD_setPosition(0,0);
            LCD_writeString("Rescue.\n");
            printf("Rescue\n");
        }else if(Interface_isSetStationPressed() && state != SETSTATION){
            state = SETSTATION;
            LCD_setPosition(0,0);
            LCD_writeString("Set station.\n");
            printf("Set cancel\n");
        }else{
            // Nothing
        }

        Interface_runSM();
    }

    return SUCCESS;
}

#endif



//#define TEST_INTERFACE2
#ifdef TEST_INTERFACE2
int main(void) {
    //initializations
    Board_init();
    Serial_init();
    Timer_init();
    LCD_init();
    Interface_init();
    
    printf("INITIALIZATIONS COMPLETE\n");
    LCD_writeString("Interface online.\n");

    Timer_new(TIMER_TEST, 100);
    while (1) {
        if (Timer_isExpired(TIMER_TEST)) {
            printf("%X -- C:%X F:%X\n",Interface_isOkPressed(), buttonCount.okay, buttonPressed.flags.okay);
            Timer_new(TIMER_TEST, 100);
        }
        Interface_runSM();
    }
}
#endif