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
#include "Accelerometer.h"
#include "Magnetometer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define WAIT_BETWEEN_CHECKS 20 // [miliseconds]
#define NUMBER_OF_TIMES_TO_CHECK 1
#define MINIMUM_POSITIVES 1
#define BUTTON_BYTE_COUNT 1

/* SWITCHES */
#define OKAY_BUTTON_TRIS               PORTY03_TRIS // pin 35 J5-4
#define CANCEL_BUTTON_TRIS             PORTY04_TRIS // pin 9 J5-3
#define STOP_BUTTON_TRIS               PORTX12_TRIS // pin 36 J5-6
#define RESCUE_BUTTON_TRIS             PORTX10_TRIS // pin 37 J5-8
#define SETSTATION_BUTTON_TRIS         PORTX08_TRIS // pin 38 J5-10

#define OKAY_BUTTON               PORTY03_BIT // pin 35 J5-4
#define CANCEL_BUTTON             PORTY04_BIT // pin 9 J5-3
#define STOP_BUTTON               PORTX12_BIT // pin 36 J5-6
#define RESCUE_BUTTON             PORTX10_BIT // pin 37 J5-8
#define SETSTATION_BUTTON         PORTX08_BIT // pin 38 J5-10

/* LEDS */

#define READY_LED_TRIS              PORTZ08_TRIS // pin 2 J6-5
#define WAIT_LED_TRIS               PORTZ06_TRIS // pin 3 J6-7
#define ERROR_LED_TRIS              PORTZ04_TRIS // pin 4 J6-09
#define CALIBRATE_BACK_LED_TRIS         PORTY06_TRIS // pin 8 J5-01
#define CALIBRATE_FRONT_LED_TRIS        PORTY05_TRIS // pin 34 J5-02

#define READY_LED               PORTZ08_LAT // pin 2 J6-5
#define WAIT_LED                PORTZ06_LAT // pin 3 J6-7
#define ERROR_LED               PORTZ04_LAT // pin 4 J6-09
#define CALIBRATE_BACK_LED          PORTY06_LAT // pin 8 J5-01
#define CALIBRATE_FRONT_LED         PORTY05_LAT // pin 34 J5-02

#define PRESSED                 1 // buttons active low
/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static void showMessage(message_t msgCode);

/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/

static int buttonReadCount;
bool usingYawLights;
bool usingPitchLights;

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
    "Boat is now online.\n",
    "Resetting boat.\n"
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
    // Set buttons as inputs
    OKAY_BUTTON_TRIS = INPUT;
    CANCEL_BUTTON_TRIS = INPUT;
    STOP_BUTTON_TRIS = INPUT;
    RESCUE_BUTTON_TRIS = INPUT;
    SETSTATION_BUTTON_TRIS = INPUT;

    // Set LEDs as outputs
    READY_LED_TRIS = OUTPUT;
    WAIT_LED_TRIS = OUTPUT;
    ERROR_LED_TRIS = OUTPUT;
    CALIBRATE_BACK_LED_TRIS = OUTPUT;
    CALIBRATE_FRONT_LED_TRIS  = OUTPUT;

    // Turn off all LEDs and all buttons
    Interface_clearAll();

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

        if(OKAY_BUTTON == PRESSED)
            buttonCount.okay += 1;
        if(CANCEL_BUTTON == PRESSED)
            buttonCount.cancel += 1;
        if(STOP_BUTTON == PRESSED)
            buttonCount.stop += 1;
        if(RESCUE_BUTTON == PRESSED)
            buttonCount.rescue += 1;
        if(SETSTATION_BUTTON == PRESSED)
            buttonCount.setStationKeep += 1;
        
        buttonReadCount++;

        if(buttonReadCount >= NUMBER_OF_TIMES_TO_CHECK){
            buttonReadCount = 0;

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

    // Calibration lights
    if (usingPitchLights) {
        if (Accelerometer_isLevel()) {
            CALIBRATE_FRONT_LED = ON;
            CALIBRATE_BACK_LED = ON;
        }
        else if (Accelerometer_getX() > Accelerometer_getY()) {
            CALIBRATE_FRONT_LED = OFF;
            CALIBRATE_BACK_LED = ON;
        }
        else if (Accelerometer_getY() > Accelerometer_getX()) {
            CALIBRATE_FRONT_LED = ON;
            CALIBRATE_BACK_LED = OFF;
        }
    }
    else if (usingYawLights) {
        if (Magnetometer_isNorth()) {
            CALIBRATE_FRONT_LED = ON;
            CALIBRATE_BACK_LED = ON;
        }
        else {
            CALIBRATE_FRONT_LED = OFF;
            CALIBRATE_BACK_LED = OFF;
        }
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
    READY_LED = ON;
}
/**********************************************************************
 * Function: Interface_readyLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_readyLightOff(){
    READY_LED = OFF;
}
/**********************************************************************
 * Function: Interface_waitLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_waitLightOn(){
    WAIT_LED = ON;
}
/**********************************************************************
 * Function: Interface_waitLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_waitLightOff(){
    WAIT_LED = OFF;
}
/**********************************************************************
 * Function: Interface_errorLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_errorLightOn(){
    ERROR_LED = ON;
}
/**********************************************************************
 * Function: Interface_errorLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_errorLightOff(){
    ERROR_LED = OFF;
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
    CALIBRATE_FRONT_LED = 0;
    CALIBRATE_BACK_LED = 0;
    usingPitchLights = FALSE;
}

/**********************************************************************
 * Function: Interface_yawLightsOff
 * @return None.
 * @remark Turns yaw calibration lights off.
 **********************************************************************/
void Interface_yawLightsOff() {
    CALIBRATE_FRONT_LED = 0;
    CALIBRATE_BACK_LED = 0;
    usingYawLights = FALSE;
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
    usingPitchLights = TRUE;
}

/**********************************************************************
 * Function: Interface_yawLightsOn
 * @return None.
 * @remark Turns yaw calibration lights on, which will use both
 *  top calibration LEDs to signal to the user when the scope is facing
 *  true North, by turning both LEDs on.
 **********************************************************************/
void Interface_yawLightsOn() {
    usingYawLights = TRUE;
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
void Interface_clearAll() {
    currentMsgCode = NO_MESSAGE;
    nextMsgCode = NO_MESSAGE;
    void LCD_clearDisplay();
    Timer_stop(TIMER_LCD_HOLD);
    Timer_stop(TIMER_LIGHT_HOLD);
    Timer_clear(TIMER_LIGHT_HOLD);
    Timer_clear(TIMER_LCD_HOLD);

    // Reset button counts
    buttonCount.okay = 0;
    buttonCount.cancel = 0;
    buttonCount.stop = 0;
    buttonCount.rescue = 0;
    buttonCount.setStationKeep = 0;
    buttonReadCount = 0;

    // Clear button flags
    buttonPressed.flags.cancel = false;
    buttonPressed.flags.okay = false;
    buttonPressed.flags.rescue = false;
    buttonPressed.flags.setStationKeep = false;
    buttonPressed.flags.stop = false;

    // Turn off LEDs
    READY_LED = OFF;
    WAIT_LED = OFF;
    ERROR_LED = OFF;
    CALIBRATE_FRONT_LED = OFF;
    CALIBRATE_BACK_LED = OFF;
}

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

static void showMessage(message_t msgCode){
        LCD_setPosition(0,0);
        LCD_writeString(INTERFACE_MESSAGE[msgCode]);
        currentMsgCode = msgCode;
}

/**********************************************************************
 * Function: getMessage
 * @param None.
 * @return None.
 * @remark Returns the interface message for the given message code.
 **********************************************************************/
char *getMessage(message_t code) {
    return (char*)INTERFACE_MESSAGE[code];
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


#define TEST_INTERFACE
#ifdef TEST_INTERFACE
int main(void) {
    //initializations
    Board_init();
    Serial_init();
    Timer_init();
    LCD_init();
    Interface_init();
    
    printf("INITIALIZATIONS COMPLETE\n");
    LCD_setPosition(0,0);
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
/*
#define TEST_CALIBRATE
#ifdef TEST_CALIBRATE

enum {
    CALIBRATE_PITCH,
    CALIBRATE_YAW
} state;
bool startedCalibrate;

int main(void) {
    // Initializations
    Board_init();
    Serial_init();
    Timer_init();
    LCD_init();
    Accelerometer_init();
    Magnetometer_init();
    Interface_init();

    printf("Initialized calibration test.\nCalibrating...");
    LCD_setPosition(0,0);
    LCD_writeString("Calibrating...\n");
    LCD_pitchLightsOn();
    LCD_readyLightOn();
    state = CALIBRATE_PITCH;
    startedCalibrate = FALSE;

    while (1) {
        switch (state) {
            case CALIBRATE_PITCH:
                if (startedCalibrate && Timer_isExpired(TIMER_TEST)) {
                    state = CALIBRATE_YAW;
                    printf("Calibrated pitch.\nCalibrating yaw...\n");
                    LCD_setPosition(0,0);
                    LCD_writeString("Calibrated pitch.\nCalibrating yaw...\n");
                    LCD_pitchLights
                    LCD_yawLightsOn();
                    LCD_readyLightOn();
                    LCD_waitLightOff();
                }
                else if (Accelerometer_isNorth() && !startedCalibrate) {
                    startedCalibrate = TRUE;
                    printf("Hold.\n");
                    LCD_setPosition(0,0);
                    LCD_writeString("Calibrating...\n");
                    LCD_readyLightOff();
                    LCD_waitLightOn();
                }
                break;
            case CALIBRATE_YAW:
                break;
        }
        Magnetometer_runSM();
        Accelerometer_runSM();
        Interface_runSM();
    }

    //

     // Initialize the modules
    Board_init();
    Timer_init();
    Serial_init();
    LCD_init();
    I2C_init(I2C_ID, I2C_CLOCK_FREQ);
    Magnetometer_init();
    Interface_init();

    //printf("Who am I: 0x%X\n", readRegister(WHO_AM_I_ADDRESS));

    if (Accelerometer_init() != SUCCESS) {
        printf("Failed to initialize the accelerometer.\n");
        return FAILURE;
    }
    printf("Initialized the accelerometer.\n");

    printf("Initialized calibration test.\nCalibrating...");
    LCD_setPosition(0,0);
    LCD_writeString("Calibrating...\n");
    LCD_pitchLightsOn();
    LCD_readyLightOn();
    state = CALIBRATE_PITCH;
    startedCalibrate = FALSE;
    // Configure ports as outputs
    /*
    LED_N_TRIS = OUTPUT;
    LED_S_TRIS = OUTPUT;
    LED_E_TRIS = OUTPUT;
    LED_W_TRIS = OUTPUT;


    Timer_new(TIMER_TEST,PRINT_DELAY  );
    while(1) {
    // Convert the raw data to real values
        if (Timer_isExpired(TIMER_TEST)) {
            if (Accelerometer_isLevel() && !calibrating) {
                calibrating = TRUE;
                Timer_new(TIMER_TEST2,CALIBRATE_HOLD_DELAY);
                printf("Calibrating.");
            }
            else if (Accelerometer_isLevel() && calibrating) {
                 if (!Timer_isExpired(TIMER_TEST2)) {
                     printf(".");
                 }
            }
            else if (calibrating && !Accelerometer_isLevel()) {
                Timer_stop(TIMER_TEST2);
                Timer_clear(TIMER_TEST2);
                calibrating = FALSE;
                printf("! -- Failed.\n");
            }
            //printf("X=%d, Y=%d, Z=%d level=%X_%d\n",
            //    Accelerometer_getX(), Accelerometer_getY(), Accelerometer_getZ(), Accelerometer_isLevel(),
            //        abs(gCount.x + gCount.y));
            Timer_new(TIMER_TEST, PRINT_DELAY );
        }

        if (Accelerometer_isLevel() && calibrating && Timer_isExpired(TIMER_TEST2)) {
            printf("! -- Done.\n");
            return SUCCESS;
        }

        Accelerometer_runSM();
    }

    return (SUCCESS);
}
*/