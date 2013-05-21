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
//#define DEBUG

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

//#define DEBUG_BUTTON_CHECK // spams button statuses in runSM
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
#define RESET_BUTTON_TRIS              PORTX11_TRIS // pin 10 J5-05 (select JP4)

#define OKAY_BUTTON               PORTY03_BIT // pin 35 J5-4
#define CANCEL_BUTTON             PORTY04_BIT // pin 9 J5-3
#define STOP_BUTTON               PORTX12_BIT // pin 36 J5-6
#define RESCUE_BUTTON             PORTX10_BIT // pin 37 J5-8
#define SETSTATION_BUTTON         PORTX08_BIT // pin 38 J5-10
#define RESET_BUTTON              PORTX11_BIT // pin 10 J5-05 (select JP4)

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
static bool usingYawLights;
static bool usingPitchLights;

static struct button_count {
    uint8_t okay;
    uint8_t cancel;
    uint8_t stop;
    uint8_t rescue;
    uint8_t setStation;
    uint8_t reset;
} buttonCount;

static union button_pressed{
    struct {
        unsigned int okay :1;
        unsigned int cancel :1;
        unsigned int stop :1;
        unsigned int rescue :1;
        unsigned int setStation :1;
        unsigned int reset :1;
    } flag;
    unsigned char bytes[BUTTON_BYTE_COUNT];
} buttonPressed;

static void (*timerLightOffFunction)();


const char *INTERFACE_MESSAGE[] = {
  //"....................\n"  <- maxiumum line length
    "Blank message.",
    "Calibration success.",
    "Calibrate system.\nLevel scope with\nhorizon.",
    "Calibrate system.\nPoint scope north\nand at horizon.",
    "Command center is\nready.",
    "Sending boat to\nrescue person.",
    "Boat started rescue.",
    "Boat rescued person.",
    "Are you sure you\nwant to cancel the\nrescue?",
    "Sending boat to\nstation.",
    "Boat is headed\nto station.",
    "Stopping the boat.",
    "Boat has stopped.",
    "Are you sure you\nwant the boat to\nreturn to station?",
    "Saving boat's posit-\nion as new station.",
    "Saved new station.",
    "Set new station.",
    "Setting boat origin.",
    "Set new origin.",
    "Boat is now online.",
    "Resetting boat.",
    "Resetting system.",
    "Are you sure you\nwant to cancel retu-\nrning to station?",
    "Are you sure you\nwant to cancel sett-\ning station?",
    "System will reset.\nHold longer to reset\nthe boat."
};

static message_t currentMsgCode, nextMsgCode;

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


    // Reset button counts
    buttonCount.okay = 0;
    buttonCount.cancel = 0;
    buttonCount.stop = 0;
    buttonCount.rescue = 0;
    buttonCount.setStation = 0;
    buttonCount.reset = 0;
    buttonReadCount = 0;
    
    // Clear button flags
    buttonPressed.bytes[0] = false;

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
        Interface_clearDisplay();
        if (nextMsgCode != NO_MESSAGE) {
            showMessage(nextMsgCode);
            nextMsgCode = NO_MESSAGE;
        }
        Timer_clear(TIMER_LCD_HOLD);
    }
   //printf("HERE! ");
    if(Timer_isExpired(TIMER_INTERFACE)) {

        #if NUMBER_OF_TIMES_TO_CHECK > 1
        if(OKAY_BUTTON == PRESSED)
            buttonCount.okay += 1;
        if(CANCEL_BUTTON == PRESSED)
            buttonCount.cancel += 1;
        if(STOP_BUTTON == PRESSED)
            buttonCount.stop += 1;
        if(RESCUE_BUTTON == PRESSED)
            buttonCount.rescue += 1;
        if(SETSTATION_BUTTON == PRESSED)
            buttonCount.setStation += 1;
        if (RESET_BUTTON == PRESSED)
            buttonCount.reset += 1;
        
        buttonReadCount++;

        if(buttonReadCount >= NUMBER_OF_TIMES_TO_CHECK){
            buttonReadCount = 0;

            // Clear button flag
            buttonPressed.bytes[0] = false;

            if(buttonCount.okay >= MINIMUM_POSITIVES)
                buttonPressed.flag.okay = true;
            if(buttonCount.cancel >= MINIMUM_POSITIVES)
                buttonPressed.flag.cancel = true;
            if(buttonCount.stop >= MINIMUM_POSITIVES)
                buttonPressed.flag.stop = true;
            if(buttonCount.rescue >= MINIMUM_POSITIVES)
                buttonPressed.flag.rescue = true;
            if(buttonCount.setStation >= MINIMUM_POSITIVES)
                buttonPressed.flag.setStation = true;
            if (buttonCount.reset >= MINIMUM_POSITIVES)
                buttonPressed.flag.reset = TRUE;

            // Reset button counts
            buttonCount.okay = 0;
            buttonCount.cancel = 0;
            buttonCount.stop = 0;
            buttonCount.rescue = 0;
            buttonCount.setStation = 0;
            buttonCount.reset = 0;
        }
        #else
        buttonPressed.flag.okay = (OKAY_BUTTON == PRESSED);
        buttonPressed.flag.cancel = (CANCEL_BUTTON == PRESSED);
        buttonPressed.flag.stop = (STOP_BUTTON == PRESSED);
        buttonPressed.flag.rescue = (RESCUE_BUTTON == PRESSED);
        buttonPressed.flag.setStation = (SETSTATION_BUTTON == PRESSED);
        buttonPressed.flag.reset = (RESET_BUTTON == PRESSED);
        #endif
#ifdef DEBUG_BUTTON_CHECK
        Interface_waitLightOnTimer(1000);
        printf("Ok=%d, C=%d, S=%d, R=%d, SS=%d, Reset=%d\n",
            buttonPressed.flag.okay, buttonPressed.flag.cancel,
            buttonPressed.flag.stop, buttonPressed.flag.rescue,
            buttonPressed.flag.setStation, buttonPressed.flag.reset);
#endif
        Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
    }

    // Calibration lights
    if (usingPitchLights) {
        READY_LED = OFF;
        if (Accelerometer_isLevel()) {
            CALIBRATE_FRONT_LED = ON;
            CALIBRATE_BACK_LED = ON;
            READY_LED = ON;
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
        READY_LED = OFF;
        if (Magnetometer_isNorth()) {
            CALIBRATE_FRONT_LED = ON;
            CALIBRATE_BACK_LED = ON;
            READY_LED = ON;
        }
        else {
            CALIBRATE_FRONT_LED = OFF;
            CALIBRATE_BACK_LED = OFF;
        }
    }

    if (Timer_isExpired(TIMER_INTERFACE) || !Timer_isActive(TIMER_INTERFACE))
        Timer_new(TIMER_INTERFACE, WAIT_BETWEEN_CHECKS);
    
}

/**********************************************************************
 * Function: Interface_isCancelPressed
 * @param None.
 * @return TRUE if the cancel button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isCancelPressed(){
    return buttonPressed.flag.cancel;
}
/**********************************************************************
 * Function: Interface_isOkPressed
 * @param None.
 * @return TRUE if the ok button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isOkPressed(){
    return buttonPressed.flag.okay;
}
/**********************************************************************
 * Function: Interface_isStopPressed
 * @param None.
 * @return TRUE if the stop button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isStopPressed(){
    return buttonPressed.flag.stop;
}

/**********************************************************************
 * Function: Interface_isRescuePressed
 * @param None.
 * @return TRUE if the rescue button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isRescuePressed(){
    return buttonPressed.flag.rescue;
}

/**********************************************************************
 * Function: Interface_isSetStationPressed
 * @param None.
 * @return TRUE if the set station button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isSetStationPressed(){
    return buttonPressed.flag.setStation;

}

/**********************************************************************
 * Function: Interface_isResetPressed
 * @param None.
 * @return TRUE if the reset button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isResetPressed(){
    return buttonPressed.flag.reset;

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
    CALIBRATE_FRONT_LED = OFF;
    CALIBRATE_BACK_LED = OFF;
    usingPitchLights = FALSE;
}

/**********************************************************************
 * Function: Interface_yawLightsOff
 * @return None.
 * @remark Turns yaw calibration lights off.
 **********************************************************************/
void Interface_yawLightsOff() {
    CALIBRATE_FRONT_LED = OFF;
    CALIBRATE_BACK_LED = OFF;
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
    LCD_writeString("Error:\n");
    LCD_writeString(getErrorMessage(errorCode));
    currentMsgCode = errorCode;
}

/**********************************************************************
 * Function: Interface_showBoatErrorMessage
 * @param Error code for the error message to print.
 * @return None.
 * @remark Prints a boat error code to the LCD screen, and turns on the
 *  error LED, while clearing all other lights and messages.
 **********************************************************************/
void Interface_showBoatErrorMessage(error_t errorCode) {
    Interface_clearAll();
    Interface_errorLightOn();
    LCD_setPosition(0,0);
    LCD_writeString("Boat error:\n");
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

    Timer_stop(TIMER_LCD_HOLD);
    Timer_stop(TIMER_LIGHT_HOLD);
    Timer_clear(TIMER_LIGHT_HOLD);
    Timer_clear(TIMER_LCD_HOLD);

    // Turn off LEDs
    READY_LED = OFF;
    WAIT_LED = OFF;
    ERROR_LED = OFF;
    CALIBRATE_FRONT_LED = OFF;
    CALIBRATE_BACK_LED = OFF;
    
    LCD_clearDisplay();
}

/**********************************************************************
 * Function: Interface_clearDisplay
 * @param None.
 * @return None.
 * @remark Clear the LCD.
 **********************************************************************/
void Interface_clearDisplay() {
    LCD_clearDisplay();
}

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

static void showMessage(message_t msgCode){
    LCD_clearDisplay();
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



//#define TEST_BUTTONS
#ifdef TEST_BUTTONS

// Pick the I2C_MODULE to initialize
I2C_MODULE      I2C_ID = I2C1;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)

#define STARTUP_DELAY   3500
int main(void) {
    //initializations
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    Interface_init();
    //I2C_init(I2C_ID, I2C_CLOCK_FREQ);

    DELAY(10);
    dbprint("Interface online.\n");

    DELAY(STARTUP_DELAY);
    enum {
        CANCEL  = 1,
        OK,
        STOP,
        RESCUE,
        SETSTATION,
        IDLE,
        RESET
    } state;

    //cycle and check if buttons are pressed, if so, turn light on for 3 seconds
    while(1) {
        //check to see which button is pressed
        if(Interface_isCancelPressed() && state != CANCEL){
            state = CANCEL;
            LCD_setPosition(0,0);
            dbprint("Cancel.\n");
        }else if(Interface_isOkPressed()  && state != OK){
            state = OK;
            LCD_setPosition(0,0);
            dbprint("Ok.\n");
        }else if(Interface_isStopPressed() && state != STOP){
            state = STOP;
            LCD_setPosition(0,0);
            dbprint("Stop.\n");
        }else if(Interface_isRescuePressed() && state != RESCUE){
            state = RESCUE;
            LCD_setPosition(0,0);
            dbprint("Rescue.\n");
        }else if(Interface_isSetStationPressed() && state != SETSTATION){
            state = SETSTATION;
            LCD_setPosition(0,0);
            dbprint("Set station.\n");
        }else if(Interface_isResetPressed() && state != RESET){
            state = RESET;
            LCD_setPosition(0,0);
            dbprint("Reset.\n");
        }else{
            // Nothing
        }

        Interface_runSM();
    }

    return SUCCESS;
}

#endif


//#define TEST_BUTTONS2
#ifdef TEST_BUTTONS2

#define DEBUG_PRINT_DELAY       200
#define DEBUG

// Pick the I2C_MODULE to initialize
I2C_MODULE      I2C_ID = I2C1;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)

#define DO_STUFF

#define STARTUP_DELAY   3500
int main(void) {
    //initializations
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    Interface_init();
    //I2C_init(I2C_ID, I2C_CLOCK_FREQ);

    dbprint("Interface online.\n");
    DELAY(STARTUP_DELAY);

    Timer_new(TIMER_TEST, DEBUG_PRINT_DELAY);
    //cycle and check if buttons are pressed, if so, turn light on for 3 seconds
    while(1) {
#ifdef DO_STUFF
        //check to see which button is pressed
        if(Timer_isExpired(TIMER_TEST)) {
            LCD_setPosition(0,0);
            dbprint("Ok=%d, C=%d, R=%d, S=%x,\n Reset=%x, SS=%x\n",
                Interface_isOkPressed(), Interface_isCancelPressed(),
                Interface_isRescuePressed(), Interface_isStopPressed(),
                Interface_isResetPressed(), Interface_isSetStationPressed());
            Timer_new(TIMER_TEST, DEBUG_PRINT_DELAY);
        }
        Interface_runSM();
#endif
    }

    return SUCCESS;
}

#endif

//#define TEST_LIGHTS
#ifdef TEST_LIGHTS

#define LIGHT_ON_DELAY      2000
#define LIGHT_ON_TIMER_DELAY    700

enum {
    READY_LIGHT  = 0x01,
    READY_LIGHT_TIMER,
    WAIT_LIGHT,
    WAIT_LIGHT_TIMER,
    ERROR_LIGHT,
    ERROR_LIGHT_TIMER,
    FRONT_CALIB_LIGHT,
    BACK_CALIB_LIGHT
} state;

int main(void) {
    //initializations
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    Interface_init();

    DELAY(5);
    dbprint("Interface online.\n");

    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
    state = 1;
    //cycle and check if buttons are pressed, if so, turn light on for 3 seconds
    while(1) {
        switch (state) {
            case READY_LIGHT:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    Interface_readyLightOn();
                    LCD_setPosition(1,0);
                    dbprint("Ready light.       \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case READY_LIGHT_TIMER:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    Interface_readyLightOnTimer(LIGHT_ON_TIMER_DELAY);
                    LCD_setPosition(1,0);
                    dbprint("Ready light timer.\n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case WAIT_LIGHT:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    Interface_waitLightOn();
                    LCD_setPosition(0,0);
                    dbprint("Wait light.        \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case WAIT_LIGHT_TIMER:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    Interface_waitLightOnTimer(LIGHT_ON_TIMER_DELAY);
                    LCD_setPosition(0,0);
                    dbprint("Wait light timer.   \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case ERROR_LIGHT:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    Interface_errorLightOn();
                    LCD_setPosition(0,0);
                    dbprint("Error light.        \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case ERROR_LIGHT_TIMER:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    Interface_errorLightOnTimer(LIGHT_ON_TIMER_DELAY);
                    LCD_setPosition(0,0);
                    dbprint("Error light timer.  \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case FRONT_CALIB_LIGHT:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    CALIBRATE_FRONT_LED = ON;
                    LCD_setPosition(0,0);
                    dbprint("Front calib. light.   \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state++;
                }
                break;
            case BACK_CALIB_LIGHT:
                if (Timer_isExpired(TIMER_TEST)) {
                    Interface_clearAll();
                    CALIBRATE_BACK_LED = ON;
                    LCD_setPosition(0,0);
                    dbprint("Back calib. light.   \n");
                    Timer_new(TIMER_TEST, LIGHT_ON_DELAY);
                    state = 1;
                }
                break;
        } // switch
        
        Interface_runSM();
    }

    return SUCCESS;
}

#endif


//#define TEST_QUICK
#ifdef TEST_QUICK
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
            printf("%X -- C:%X F:%X\n",Interface_isOkPressed(), buttonCount.okay, buttonPressed.flag.okay);
            Timer_new(TIMER_TEST, 100);
        }
        Interface_runSM();
    }
}
#endif


//#define TEST_CALIBRATE
#ifdef TEST_CALIBRATE

#include "I2C.h"
#include "Accelerometer.h"
#include "Magnetometer.h"

#define DEBUG_MAGNETOMETER

#define PRINT_DELAY             500
#define STARTUP_DELAY           2000
#define CALIBRATE_HOLD_DELAY    3000

// Pick the I2C_MODULE to initialize
I2C_MODULE      I2C_ID = I2C1;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)



enum {
    CALIBRATE_PITCH,
    CALIBRATE_YAW
} state;

int main(void) {
    // Initializations
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    I2C_init(I2C_ID, I2C_CLOCK_FREQ);
    Magnetometer_init();
    Interface_init();
    DELAY(5);
    if (Accelerometer_init() != SUCCESS) {
        Interface_errorLightOn();
        dbprint("Failed accel. init.\n");
        return FAILURE;
    }
    dbprint("Ready to calibrate.\n");
    Interface_readyLightOn();
    DELAY(STARTUP_DELAY);

    LCD_setPosition(0,0);
    dbprint("Please level scope.\n");
    Interface_pitchLightsOn();
    state = CALIBRATE_PITCH;
    Timer_new(TIMER_TEST2, PRINT_DELAY);
    bool holdingCalibrate = FALSE;

    while (1) {
        switch (state) {
            case CALIBRATE_PITCH:
                if (Accelerometer_isLevel() && !holdingCalibrate) {
                    holdingCalibrate = TRUE;
                    Timer_new(TIMER_TEST, CALIBRATE_HOLD_DELAY);
                    Interface_readyLightOff();
                    Interface_waitLightOn();
                    LCD_setPosition(1,0);
                    dbprint("Hold level...\n");
                }
                if (Accelerometer_isLevel() && holdingCalibrate && Timer_isExpired(TIMER_TEST)) {
                    LCD_setPosition(1,0);
                    dbprint("Pitch done!\n");
                    Interface_pitchLightsOff();
                    Interface_readyLightOn();
                    Interface_waitLightOff();
                    holdingCalibrate = FALSE;
                    DELAY(STARTUP_DELAY);

                    state = CALIBRATE_YAW;
                    LCD_setPosition(0,0);
                    dbprint("Turn scope north.\n                    \n");
                    Interface_yawLightsOn();
                }
                else if (!Accelerometer_isLevel() && holdingCalibrate) {
                    holdingCalibrate = FALSE;
                    Timer_stop(TIMER_TEST);
                    LCD_setPosition(1,0);
                    dbprint("               \n");
                    Interface_readyLightOn();
                    Interface_waitLightOff();
                }
                break;
            case CALIBRATE_YAW:
                if (Magnetometer_isNorth() && !holdingCalibrate) {
                    holdingCalibrate = TRUE;
                    Timer_new(TIMER_TEST, CALIBRATE_HOLD_DELAY);
                    Interface_readyLightOff();
                    Interface_waitLightOn();
                    LCD_setPosition(1,0);
                    dbprint("Hold north + level..\n");
                }
                if (Magnetometer_isNorth() && holdingCalibrate && Timer_isExpired(TIMER_TEST)) {
                    LCD_setPosition(1,0);
                    dbprint("Yaw done!\n");
                    Interface_yawLightsOff();
                    Interface_readyLightOn();
                    Interface_waitLightOff();
                    holdingCalibrate = FALSE;
                    DELAY(STARTUP_DELAY);
                    return SUCCESS;
                }
                else if (!Magnetometer_isNorth() && holdingCalibrate) {
                    holdingCalibrate = FALSE;
                    Timer_stop(TIMER_TEST);
                    LCD_setPosition(1,0);
                    dbprint("               \n");
                    Interface_readyLightOn();
                    Interface_waitLightOff();
                }
                #ifdef DEBUG_MAGNETOMETER
                if (Timer_isExpired(TIMER_TEST2)) {
                    LCD_setPosition(3,0);
                    dbprint("Mag: %.2f\n",Magnetometer_getHeading());
                    Timer_new(TIMER_TEST2, PRINT_DELAY);
                }
                #endif
                break;
        }
        Magnetometer_runSM();
        Accelerometer_runSM();
        Interface_runSM();
    }

  

    return (SUCCESS);
}

#endif

//#define ENCODER_ZERO_TEST
#ifdef ENCODER_ZERO_TEST

#include "I2C.h"
#include "Encoder.h"

// Pick the I2C_MODULE to initialize
I2C_MODULE      I2C_ID = I2C1;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)


#define PRINT_DELAY     50
#define STARTUP_DELAY   2000
#define CALIBRATE_HOLD_DELAY    3000

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    dbprint("Starting encoders...\n");
    I2C_init(I2C_ID, I2C_CLOCK_FREQ);
    Encoder_init();
    Interface_init();
    Timer_new(TIMER_TEST, PRINT_DELAY );
    while (!Timer_isExpired(TIMER_TEST)) {
        Encoder_runSM();
    }

    // Not working?
    //Encoder_setZeroPitch();
    //Encoder_setZeroYaw();

    dbprint("Encoders initialized.\n");
    DELAY(STARTUP_DELAY)
    Timer_new(TIMER_TEST, PRINT_DELAY );

    bool clearedCalibrateMessage = TRUE;
    Timer_new(TIMER_TEST2,CALIBRATE_HOLD_DELAY);
    //Interface_waitLightOnTimer(CALIBRATE_HOLD_DELAY);
    Interface_waitLightOnTimer(CALIBRATE_HOLD_DELAY);

    LCD_setPosition(0,0);
    dbprint("Encoders:\n");
    while(1) {
        if (Timer_isExpired(TIMER_TEST)) {
            LCD_setPosition(1,0);
            dbprint(" P=%.1f,\n Y=%.1f\n",Encoder_getPitch(), Encoder_getYaw());

            Timer_new(TIMER_TEST, PRINT_DELAY );
        }
        if (Interface_isOkPressed() && Timer_isExpired(TIMER_TEST2)) {
            Encoder_setZeroPitch();
            Encoder_setZeroYaw();
            LCD_setPosition(3,0);
            dbprint("Zeroed encoders.\n");
            clearedCalibrateMessage = FALSE;
            Interface_waitLightOnTimer(CALIBRATE_HOLD_DELAY);
            Interface_readyLightOff();
            Timer_new(TIMER_TEST2,CALIBRATE_HOLD_DELAY);
        }
        if (Timer_isExpired(TIMER_TEST2) && !clearedCalibrateMessage) {
            clearedCalibrateMessage = TRUE;
            LCD_setPosition(3,0);
            dbprint("                     ");
            Interface_readyLightOn();
        }
        Encoder_runSM();
        Interface_runSM();
    }

    return (SUCCESS);
}

#endif
