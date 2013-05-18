/**********************************************************************
 Module
   Compas.c

 Author:
    David Goodman
    John Ash
    Shehadeh Dajani

 Description
    Main state machine for the COMPAS (command center), which acts as an
    interface between the lifeguard and the boat.

 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
4/29/2013   3:08PM      dagoodma    Started new Compas module.
***********************************************************************/
#define IS_COMPAS
#define DEBUG
#define DEBUG_STATE
//#define DEBUG_BLINK
//#define DEBUG_VERBOSE
#define DEBUG_RESCUE
#define USE_INTERFACE
//#define USE_MAGNETOMETER
#define USE_ENCODER
#define USE_ACCELEROMETER
#define USE_BAROMETER
#define USE_GPS
//#define USE_XBEE
#define ENABLE_RESET

//#define REQUIRE_RESCUE_HEIGHT // causes an error if altitude unknown and rescue pressed

#ifdef DEBUG
#ifdef USE_SD_LOGGER
#define DBPRINT(...)   do { char debug[255]; sprintf(debug,__VA_ARGS__); } while(0)
#else
#define DBPRINT(...)   printf(__VA_ARGS__)
#endif
#else
#define DBPRINT(...)   ((int)0)
#endif


#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "Accelerometer.h"
#include "I2C.h"
#include "Serial.h"

#include "Board.h"
#include "Encoder.h"
#include "Ports.h"
#include "TiltCompass.h"
#include "Timer.h"
#include "Xbee.h"
#include "Uart.h"
#include "Gps.h"
#include "Barometer.h"
#include "Override.h"
#include "Error.h"
#include "Interface.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
 
// Timer allocation
#define TIMER_CALIBRATE     TIMER_MAIN
#define TIMER_RESCUE        TIMER_MAIN
#define TIMER_STOP          TIMER_MAIN
#define TIMER_SETORIGIN     TIMER_MAIN
#define TIMER_SETSTATION    TIMER_MAIN
#define TIMER_DEBOUNCE      TIMER_MAIN2
#define TIMER_CANCEL        TIMER_MAIN3


#define TIMER_BAROMETER_LOST            TIMER_BACKGROUND
#define TIMER_HEARTBEAT_CHECK           TIMER_BACKGROUND2
#define TIMER_GPS_CORRECTION            TIMER_BACKGROUND3

// Timer delays
#define CALIBRATE_HOLD_DELAY        3000 // (ms) time to hold calibration
#define BAROMETER_LOST_DELAY	    20000 // (ms) time before timeout error
#define HEARTBEAT_LOST_DELAY         10000// (ms) before timeout error
#define RESEND_MESSAGE_DELAY        4000 // (ms) resend a message
#define GPS_CORRECTION_SEND_DELAY   3750
#define LCD_HOLD_DELAY              3000 // (ms) time for lcd message to linger
#define LED_HOLD_DELAY              1000 // (ms) time for led to stay lit
#define CANCEL_TIMEOUT_DELAY       6500 // (ms) for cancel msg to linger
#define CANCEL_DEBOUNCE_DELAY       500 // (ms) to read cancel button in cancel state
#define RESCUE_DEBOUNCE_DELAY       500 // (ms) to read rescue button in rescue state

#define RESET_HOLD_DELAY            2000 // (ms) to hold before CC reset
#define RESET_LONG_HOLD_DELAY        5000 // (ms) to hold before CC and boat reset

#define BLINK_DELAY                 2000
#define BLINK_ON_DELAY              1500
#define DEBUG_PRINT_DELAY           1000

#define RESEND_MESSAGE_LIMIT        5 // times to resend before failing

#define EVENT_BYTE_COUNT 10 // total number of bytes in event flag union

// Hard-coded geocentric origin location // TODO replace this
// --------------- Center of west lake -------------
/*
#define ECEF_X_ORIGIN -2706922.0f
#define ECEF_Y_ORIGIN -4324246.0f
#define ECEF_Z_ORIGIN 3815364.0f
 * */

// ------- In front of GSH parking lot entrance ------

#define ECEF_X_ORIGIN -2707534.0f
#define ECEF_Y_ORIGIN -4322167.0f
#define ECEF_Z_ORIGIN  3817539.0f

#define I2C_CLOCK_FREQ  50000 // (Hz)


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
I2C_MODULE I2C_BUS_ID   = I2C1;

static enum {
    STATE_CALIBRATE = 0x1,
    STATE_SETSTATION,
    STATE_SETORIGIN,
    STATE_READY,
    STATE_STOP,
    STATE_RESCUE,
    STATE_ERROR
} state, lastState;

static enum {
    STATE_NONE = 0x0,

    /* - Calibrate SM - */
    STATE_CALIBRATE_PITCH,
    STATE_CALIBRATE_YAW,

    /* - Ready SM - */
    // N/A

    /* - SetStation SM - */
    STATE_SETSTATION_SEND,
    STATE_SETSTATION_CONFIRMCANCEL,

    /* - Error SM - */
    // N/A

    /* - Stop SM - */
    STATE_STOP_SEND,
    STATE_STOP_IDLE,
    STATE_STOP_CONFIRMCANCEL,
    STATE_STOP_RETURN,

    /* - Rescue SM - */
    STATE_RESCUE_SEND,
    STATE_RESCUE_WAIT,
    STATE_RESCUE_CONFIRMCANCEL,
    STATE_RESCUE_RETURN
	
} subState, lastSubState;

static union EVENTS {
    struct {
        /* - Interface flags - */
        unsigned int rescueButtonPressed :1;
        unsigned int stopButtonPressed :1;
        unsigned int setStationButtonPressed :1;
        unsigned int okButtonPressed :1;
        unsigned int cancelButtonPressed :1;
        unsigned int resetButtonPressed :1;
        /* - Mavlink message flags - */
        unsigned int haveInitializeMessage :1; // boat starting up
        unsigned int haveErrorMessage :1; // boat had error
        unsigned int haveStartRescueAck :1; // going to rescue
        unsigned int haveSetOriginAck :1; // received origin coordinate
        unsigned int haveSetStationAck :1; // received station coordinate
        unsigned int haveReturnStationAck :1; // boat is returning to station
        unsigned int haveStopAck :1; // received stop command
        unsigned int haveSaveStationAck :1; // boat saved station
        unsigned int haveBarometerMessage :1; // boat's altitude data
        unsigned int haveStatusMessage :1; // used for debug
        unsigned int haveUnknownMessage :1;
        unsigned int haveHeartbeatMessage :1; // talking to boat
        unsigned int haveRescueSuccessMessage :1; // boat rescued drownee
        unsigned int haveReturnStationMessage :1; // boat is heading to station
        unsigned int haveRequestOriginMessage :1;
        unsigned int haveOverrideMessage :1; // boat is in override state
        unsigned int haveBoatOnlineMessage  :1;
        unsigned int readyToPrintMessage :1; // display hold for Ready state expired
        /* - Boat status and error messages - */
        unsigned int lostBoatHeartbeat :1;
        unsigned int haveBoatPositionMessage :1;

        /* - Calibrate events - */
        unsigned int scopeIsLevel :1;
        unsigned int scopeIsNorth :1;

        /* State transition events */
        unsigned int wantCancel :1;
        unsigned int calibrateDone :1;
        unsigned int setStationDone :1;
        unsigned int rescueDone :1;
        unsigned int stopDone :1;
        unsigned int setOriginDone :1;
        unsigned int lastStateWasReady :1;
        unsigned int haveError :1;
        unsigned int haveBoatError :1;
    } flags;
    unsigned char bytes[EVENT_BYTE_COUNT];
} event;

static GeocentricCoordinate ecefPosition; // TODO consider adding position averaging

static LocalCoordinate nedRescueTarget;
static uint8_t resendMessageCount;
static float compasHeight;
static int lastMessageID;
static error_t lastErrorCode;
static error_t lastBoatErrorCode;
static bool isConnectedWithBoat;
static bool haveCompasHeight;
static bool resetPressedShort;

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
static void checkEvents();
static void doCalibrateSM();
static void doReadySM();
static void doErrorSM();
static void doRescueSM();
static void doSetStationSM();
static void doStopSM();
static void doMasterSM();

static void startCalibrateSM();
static void startReadySM();
static void startErrorSM();
static void startRescueSM();
static void startSetStationSM();
static void startSetOriginSM();
static void startStopSM();
static void initializeCompas();

static void setError(error_t errorCode);
static void fatal(error_t code);
static void doBarometerUpdate();
static void gpsCorrectionUpdate();
static void getTargetLocation(LocalCoordinate *targetNed);
static void checkBoatConnection();
static void updatePosition();
static void resetCompas();
static void resetAll();
static void checkReset();

/***********************************************************************
 * PRIVATE FUNCTIONS                                                   *
 ***********************************************************************/

/**********************************************************************
 * Function: checkEvents
 * @return None
 * @remark Checks for various events that can occur, and sets the 
 *  appropriate flags for handling later.
 **********************************************************************/
static void checkEvents() {
    // Clear all event flags
    int i;
    for (i=0; i < EVENT_BYTE_COUNT; i++)
        event.bytes[i] = 0x0;

    // Check for hearbeat
    if (Timer_isExpired(TIMER_HEARTBEAT_CHECK) && isConnectedWithBoat) {
        if (!Mavlink_hasHeartbeat())
            event.flags.lostBoatHeartbeat = TRUE;
    }

    // Calibration flags
    if (state == STATE_CALIBRATE) {
        if (Accelerometer_isLevel())
            event.flags.scopeIsLevel = TRUE;
        #ifdef USE_MAGNETOMETER
        if (Magnetometer_isNorth())
        #else
        if (TRUE)
        #endif
            event.flags.scopeIsNorth = TRUE;
    }


    // Canceling out of the error state chooses Ready or Stop
    if (lastState == STATE_READY)
        event.flags.lastStateWasReady;

    // Check interface
    if (Interface_isOkPressed())
        event.flags.okButtonPressed = TRUE;
    if (Interface_isCancelPressed())
        event.flags.cancelButtonPressed = TRUE;
    if (Interface_isSetStationPressed())
        event.flags.setStationButtonPressed = TRUE;
    if (Interface_isStopPressed())
        event.flags.stopButtonPressed = TRUE;
    if (Interface_isRescuePressed()) {
        #ifdef REQUIRE_RESCUE_HEIGHT
        if (haveCompasHeight)
            event.flags.rescueButtonPressed = TRUE;
        else
            setError(ERROR_NO_ALTITUDE);
        #else
            event.flags.rescueButtonPressed = TRUE;
        #endif
    }
    if (Interface_isResetPressed())
        event.flags.resetButtonPressed = TRUE;

    // XBee messages (from command center)
    if (Mavlink_hasNewMessage()) {
        lastMessageID = Mavlink_getNewMessageID();
        switch (lastMessageID) {
            /*--------------------  Acknowledgement messages ------------------ */
            case MAVLINK_MSG_ID_MAVLINK_ACK:
		// CMD_OTHER
                if (Mavlink_newMessage.ackData.msgID == MAVLINK_MSG_ID_CMD_OTHER) {
                    // Responding to a return to station request
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_RETURN_STATION)
                        event.flags.haveReturnStationAck = TRUE;
                    // Boat is in override state
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_OVERRIDE)
                        event.flags.haveStopAck = TRUE;
                    // Boat saved station
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_SAVE_STATION)
                        event.flags.haveSetStationAck = TRUE;
                }
                // GPS_NED
                if (Mavlink_newMessage.ackData.msgID == MAVLINK_MSG_ID_GPS_NED) {
                    // Responding to a rescue
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_LOCAL_START_RESCUE)
                        event.flags.haveStartRescueAck = TRUE;
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_LOCAL_SET_STATION)
                        event.flags.haveSetStationAck = TRUE;
                }
                // GPS_ECEF
                if (Mavlink_newMessage.ackData.msgID == MAVLINK_MSG_ID_GPS_ECEF) {
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_GEOCENTRIC_ORIGIN)
                        event.flags.haveSetOriginAck = TRUE;
                }
                break;
            /*------------------ CMD_OTHER messages --------------- */
            case MAVLINK_MSG_ID_CMD_OTHER:
                if (Mavlink_newMessage.commandOtherData.command == MAVLINK_REQUEST_ORIGIN)
                    event.flags.haveRequestOriginMessage = TRUE;
                break;
            /*--------------------  GPS messages ------------------ */
            case MAVLINK_MSG_ID_GPS_ECEF:
                // Nothing
                break;
            case MAVLINK_MSG_ID_GPS_NED:	
                // Boat sent position info
                if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_BOAT_POSITION)
                    event.flags.haveBoatPositionMessage = TRUE;
                break;
            /*---------------- Status and Error messages ---------- */
            case MAVLINK_MSG_ID_STATUS_AND_ERROR:
                // ---------- Boat error messages ------------------
                if (Mavlink_newMessage.statusAndErrorData.error != ERROR_NONE) {
                    event.flags.haveBoatError = TRUE;
                    lastBoatErrorCode = Mavlink_newMessage.statusAndErrorData.error;
                }
                else {
                    // ---------- Boat status messages -----------------
                    if (Mavlink_newMessage.statusAndErrorData.status == MAVLINK_STATUS_ONLINE)
                        event.flags.haveBoatOnlineMessage = TRUE;
                }
                break;
            /*----------------  Heartbeat message -------------------------*/
                // This is handled in checkHeartbeat() below
            /*----------------  Barometer altitude message ----------------*/
            case MAVLINK_MSG_ID_BAROMETER:
                event.flags.haveBarometerMessage = TRUE;
                break;
            default:
                // Unknown message ID
                event.flags.haveUnknownMessage = TRUE;
                //DBPRINTF("Mavlink received unhandled message: 0x%X\n");
                break;
        }
    }
} //  checkEvents()


/**********************************************************************
 * Function: doCalibrateSM
 * @return None
 * @remark Executes one cycle of the Calibrate state machine.
 *	To calibrate the COMPAS, the user must first calibrate the pitch,
 *  followed by the yaw. To calibrate the pitch, the user must aim the
 *  scope at the horizon for three seconds while both top lights turn on.
 *  the yaw can then be calibrated. This is done by
 *  pivoting the scope to face true north, until both lights turn on if
 *  the pitch is also level.
 **********************************************************************/
static void doCalibrateSM() {
    switch(subState) {
        case STATE_CALIBRATE_PITCH:
            // Start timer if scope is level
            if (event.flags.scopeIsLevel) {
                if (Timer_isExpired(TIMER_CALIBRATE)) {
                    // Progress to yaw calibration
                    Encoder_setZeroPitch();
                    subState = STATE_CALIBRATE_YAW;
                    Interface_showMessage(CALIBRATE_YAW_MESSAGE);
                    Interface_pitchLightsOff();
                    Interface_yawLightsOn(); // turn both lights on when North
                    Timer_clear(TIMER_CALIBRATE);
                }
                else if (!Timer_isActive(TIMER_CALIBRATE))
                    Timer_new(TIMER_CALIBRATE,CALIBRATE_HOLD_DELAY);
            }
            else {
                    Timer_stop(TIMER_CALIBRATE);
            }

            break;
        case STATE_CALIBRATE_YAW:
            // Start timer if scope is pointed north
            if (event.flags.scopeIsNorth && event.flags.scopeIsLevel) {
                if (Timer_isExpired(TIMER_CALIBRATE)) {
                    // Finished calibrating
                    Encoder_setZeroYaw();
                    Interface_clearDisplay();
                    Interface_showMessageOnTimer(CALIBRATE_SUCCESS_MESSAGE,LCD_HOLD_DELAY);
                    Interface_yawLightsOff();
                    event.flags.calibrateDone = TRUE;
                }
                else if (!Timer_isActive(TIMER_CALIBRATE))
                    Timer_new(TIMER_CALIBRATE,CALIBRATE_HOLD_DELAY);
            }
            else {
                Timer_stop(TIMER_CALIBRATE);
            }
            break;
} // switch
}


/**********************************************************************
 * Function: doRescueSM
 * @return None
 * @remark Executes one cycle of the Rescue state machine.
 **********************************************************************/
static void doRescueSM() {
    switch (subState) {
        case STATE_RESCUE_SEND:
            // Tell boat to rescue someone
            if (event.flags.haveStartRescueAck) {
                // Transition to wait substate
                subState = STATE_RESCUE_WAIT;
                Interface_clearDisplay();
                Interface_showMessage(STARTED_RESCUE_MESSAGE);
                Interface_waitLightOff();
                resendMessageCount = 0;
                Interface_readyLightOn();
                #ifdef DEBUG_RESCUE
                LCD_setPosition(2,0);
                LCD_writeString("R: N=%.2f, E=%.2f\nE: Y=%.2f, P=%.2f",
                    nedRescueTarget.north, nedRescueTarget.east,
                    Encoder_getYaw(), Encoder_getPitch());
                #endif
            }
            else if (Timer_isExpired(TIMER_RESCUE)) {
                // Resend start rescue message on timer
                if (resendMessageCount >= RESEND_MESSAGE_LIMIT) {
                    // Sent too many times
                    setError(ERROR_NO_ACKNOWLEDGEMENT);
                    return;
                }
                else {
                    Mavlink_sendStartRescue(WANT_ACK, &nedRescueTarget);
                    Timer_new(TIMER_RESCUE, RESEND_MESSAGE_DELAY);
                    resendMessageCount++;
                }
            }
            else if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                lastSubState = subState;
                subState = STATE_RESCUE_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_readyLightOn();
                Interface_waitLightOff();
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_RESCUE_MESSAGE);
            }
            break;
        case STATE_RESCUE_WAIT:
            // Wait for boat to rescue someone
            if (event.flags.haveRescueSuccessMessage) {
                // Person was found! Exit to ready
                event.flags.rescueDone = TRUE;
                Interface_clearDisplay();
                Interface_showMessageOnTimer(RESCUE_SUCCESS_MESSAGE, LCD_HOLD_DELAY);
            }
            else if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                lastSubState = subState;
                subState = STATE_RESCUE_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_readyLightOn();
                Interface_waitLightOff();
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_RESCUE_MESSAGE);
            }
            break;
        case STATE_RESCUE_RETURN:
            // Tell boat to return to station
            if (event.flags.haveReturnStationAck) {
                // Boat is headed to station, exits to ready
                event.flags.rescueDone = TRUE;
                Interface_showMessageOnTimer(RETURNING_MESSAGE,LCD_HOLD_DELAY);
                Interface_waitLightOff();
                Interface_readyLightOn();
                resendMessageCount = 0;
            }
            else if (Timer_isExpired(TIMER_RESCUE)) {
                // Resend return to station message on timer
                if (resendMessageCount >= RESEND_MESSAGE_LIMIT) {
                    // Sent too many times
                    setError(ERROR_NO_ACKNOWLEDGEMENT);
                    return;
                }
                else {
                    Mavlink_sendReturnStation(WANT_ACK);
                    Timer_new(TIMER_RESCUE, RESEND_MESSAGE_DELAY);
                    resendMessageCount++;
                }
            }
            else if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                lastSubState = subState;
                subState = STATE_RESCUE_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_readyLightOn();
                Interface_waitLightOff();
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_RETURN_MESSAGE);
            }
            break;
        case STATE_RESCUE_CONFIRMCANCEL:
            // Ask user if they want to cancel rescue
            if (event.flags.haveRescueSuccessMessage) {
                // Person was found! Exit to ready
                event.flags.rescueDone = TRUE;
                Interface_showMessageOnTimer(RESCUE_SUCCESS_MESSAGE, LCD_HOLD_DELAY);
            }
            else if (event.flags.okButtonPressed) {
                // Transition to return substate
                /*
                subState = STATE_RESCUE_RETURN;
                Interface_clearDisplay();
                Interface_showMessage(START_RETURN_MESSAGE);
                Interface_readyLightOff();
                Interface_waitLightOn();
                resendMessageCount = 0;
                Mavlink_sendReturnStation(WANT_ACK);
                Timer_new(TIMER_RESCUE, RESEND_MESSAGE_DELAY);
                 */
                startStopSM();
            }
            else if (Timer_isExpired(TIMER_CANCEL)
                || (Timer_isExpired(TIMER_DEBOUNCE) && event.flags.cancelButtonPressed)) {
                // Transition out of cancel and back into the last state
                Interface_clearDisplay();
                Interface_readyLightOff();
                Interface_waitLightOn();
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                if (lastSubState == STATE_RESCUE_SEND)
                    Interface_showMessage(STARTING_RESCUE_MESSAGE);
                else if (lastSubState == STATE_RESCUE_RETURN)
                    Interface_showMessage(START_RETURN_MESSAGE);
                else if (lastSubState == STATE_RESCUE_WAIT) {
                    Interface_showMessage(STARTED_RESCUE_MESSAGE);
                    Interface_readyLightOn();
                    Interface_waitLightOff();
                }

                subState = lastSubState;
            }
            break;
    }
} // doRescueSM

/**********************************************************************
 * Function: doStopSM
 * @return None
 * @remark Executes one cycle of the Stop state machine.
 **********************************************************************/
static void doStopSM() {
    switch(subState) {
        case STATE_STOP_SEND:
            // Tell boat to stop
            if (event.flags.haveStopAck) {
                // Transition to wait substate
                subState = STATE_STOP_IDLE;
                Interface_showMessage(STOPPED_BOAT_MESSAGE);
                Interface_waitLightOff();
                Interface_readyLightOn();
                resendMessageCount = 0;
            }
            else if (Timer_isExpired(TIMER_STOP)) {
                // Resend start rescue message on timer
                if (resendMessageCount >= RESEND_MESSAGE_LIMIT) {
                    // Sent too many times
                    setError(ERROR_NO_ACKNOWLEDGEMENT);
                    return;
                }
                else {
                    Mavlink_sendOverride(WANT_ACK);
                    Timer_new(TIMER_STOP, RESEND_MESSAGE_DELAY);
                    resendMessageCount++;
                }
            }
            else if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                lastSubState = subState;
                subState = STATE_STOP_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_readyLightOn();
                Interface_waitLightOff();
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_STOP_MESSAGE);
            }
            break;
        case STATE_STOP_IDLE:
            // Wait for something
            if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                lastSubState = subState;
                subState = STATE_STOP_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_STOP_MESSAGE);
            }
            break;
        case STATE_STOP_RETURN:
            // Tell boat to return to station
            if (event.flags.haveReturnStationAck) {
                // Boat is headed to station, exits to ready
                event.flags.stopDone = TRUE;
                Interface_clearDisplay();
                Interface_showMessageOnTimer(RETURNING_MESSAGE, LCD_HOLD_DELAY);
                Interface_waitLightOff();
                Interface_readyLightOn();
                resendMessageCount = 0;
            }
            else if (Timer_isExpired(TIMER_STOP)) {
                // Resend return to station message on timer
                if (resendMessageCount >= RESEND_MESSAGE_LIMIT) {
                    // Sent too many times
                    setError(ERROR_NO_ACKNOWLEDGEMENT);
                    return;
                }
                else {
                    Mavlink_sendReturnStation(WANT_ACK);
                    Timer_new(TIMER_STOP, RESEND_MESSAGE_DELAY);
                    resendMessageCount++;
                }
            }
            else if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                lastSubState = subState;
                subState = STATE_STOP_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_readyLightOn();
                Interface_waitLightOff();
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_RETURN_MESSAGE);
            }
            break;
        case STATE_STOP_CONFIRMCANCEL:
            // Ask user if they want to cancel stop
            if (event.flags.okButtonPressed) {
                // Transition out due to cancellation
                Interface_clearDisplay();
                if (lastSubState == STATE_STOP_IDLE) {
                    subState = STATE_STOP_RETURN;
                    Interface_showMessage(START_RETURN_MESSAGE);
                    Interface_readyLightOff();
                    Interface_waitLightOn();
                    resendMessageCount = 0;
                    Mavlink_sendReturnStation(WANT_ACK);
                    Timer_new(TIMER_STOP, RESEND_MESSAGE_DELAY);
                }
                else if (lastSubState == STATE_STOP_SEND)
                    startReadySM();
                else if (lastSubState == STATE_STOP_RETURN)
                    startStopSM();
            }
            else if (Timer_isExpired(TIMER_CANCEL)
                 || (Timer_isExpired(TIMER_DEBOUNCE) && event.flags.cancelButtonPressed)) {
                // Transition out of cancel and back into the last state
                Interface_clearDisplay();
                Interface_readyLightOff();
                Interface_waitLightOn();
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                if (lastSubState == STATE_STOP_SEND) 
                    Interface_showMessage(STOPPING_BOAT_MESSAGE);
                else if (lastSubState == STATE_STOP_RETURN) 
                    Interface_showMessage(START_RETURN_MESSAGE);
                else if (lastSubState == STATE_STOP_IDLE) {
                    Interface_showMessage(STOPPED_BOAT_MESSAGE);
                    Interface_readyLightOn();
                    Interface_waitLightOff();
                }
                subState = lastSubState;
            }
            break;
    }  // switch

}

/**********************************************************************
 * Function: doSetStationSM
 * @return None
 * @remark Executes one cycle of the Set Station state.
 **********************************************************************/
static void doSetStationSM() {
    switch (subState) {
        case STATE_SETSTATION_SEND:
            if (event.flags.haveSetStationAck) {
                Interface_showMessageOnTimer(SAVED_STATION_MESSAGE, LCD_HOLD_DELAY);
                event.flags.setStationDone = TRUE;
            }
            else if (event.flags.cancelButtonPressed
                    && Timer_isExpired(TIMER_DEBOUNCE)) {
                // Transition to confirmcancel substate
                subState = STATE_SETSTATION_CONFIRMCANCEL;
                Timer_new(TIMER_CANCEL, CANCEL_TIMEOUT_DELAY);
                Timer_new(TIMER_DEBOUNCE, CANCEL_DEBOUNCE_DELAY);
                Interface_readyLightOn();
                Interface_waitLightOff();
                Interface_clearDisplay();
                Interface_showMessage(CANCEL_SETSTATION_MESSAGE);
            }
            break;
        case STATE_SETSTATION_CONFIRMCANCEL:
            // Ask user if they want to cancel stop
            if (event.flags.okButtonPressed) {
                // Transition out due to cancellation
                Interface_clearDisplay();
                startReadySM();
            }
            else if (Timer_isExpired(TIMER_CANCEL)
                 || (Timer_isExpired(TIMER_DEBOUNCE) && event.flags.cancelButtonPressed)) {
                startSetStationSM();
            }
            break;
    }
}

/**********************************************************************
 * Function: doSetOriginSM
 * @return None
 * @remark Executes one cycle of the Set Origin state.
 **********************************************************************/
static void doSetOriginSM() {
    if (event.flags.haveSetOriginAck) {
        Interface_showMessageOnTimer(SET_ORIGIN_MESSAGE, LCD_HOLD_DELAY);
        event.flags.setOriginDone = TRUE;
    }
}


/**********************************************************************
 * Function: doMasterSM
 * @return None
 * @remark Executes one cycle of the ComPAS's master state machine.
 **********************************************************************/
static void doMasterSM() {
    checkEvents();

    #ifdef USE_MAGNETOMETER
    if (state = STATE_CALIBRATE)
        Magnetometer_runSM();
    #endif


    #ifdef USE_ENCODER
    Encoder_runSM();
    #endif

    #ifdef USE_ACCELEROMETER
    if (state == STATE_CALIBRATE) {
        Accelerometer_runSM();
    }
    #endif

    #ifdef USE_GPS
    GPS_runSM();
    #endif

    #ifdef USE_ERROR_CORRECTION
    gpsCorrectionUpdate();
    #endif

    #ifdef USE_XBEE
    Xbee_runSM();
    #endif

    #ifdef USE_BAROMETER
    Barometer_runSM();
    doBarometerUpdate(); // send barometer data
    #endif

    #ifdef USE_INTERFACE
    Interface_runSM();
    #endif

    // Blink on timer
    #ifdef DEBUG_BLINK
    if (Timer_isExpired(TIMER_TEST)) {
        Interface_waitLightOnTimer(BLINK_ON_DELAY);
        Timer_new(TIMER_TEST, BLINK_DELAY);
    }
    #endif

    #ifdef DEBUG_STATE
    if (Timer_isExpired(TIMER_TEST2)) {
        DBPRINT("State: %d, %d\n", state, subState);
        DBPRINT("Ok=%X, Cancel=%X, Rescue=%X, Stop=%X\n",
            Interface_isOkPressed(), Interface_isCancelPressed(),
            Interface_isRescuePressed(), Interface_isStopPressed());
        Timer_new(TIMER_TEST2, DEBUG_PRINT_DELAY);
    }
    #endif

    #ifdef ENABLE_RESET
    checkReset();
    #endif

    switch (state) {
        case STATE_CALIBRATE:
            doCalibrateSM();
            if (event.flags.calibrateDone)
                startReadySM();

            break;
        case STATE_READY:
            // Do nothing special

            break;
        case STATE_ERROR:
            if (event.flags.okButtonPressed
                    || event.flags.cancelButtonPressed) {
                lastErrorCode = ERROR_NONE;
                if (event.flags.lastStateWasReady)
                    startReadySM();
                else
                    startStopSM();
            }
            break;
        case STATE_SETSTATION:
            doSetStationSM();
            if (event.flags.setStationDone)
                startReadySM();

            break;
        case STATE_SETORIGIN:
            doSetOriginSM();
            if (event.flags.setOriginDone)
                startReadySM();

            break;
        case STATE_STOP:
            doStopSM();
            if (event.flags.stopDone)
                startReadySM();
            break;

        case STATE_RESCUE:
            doRescueSM();
            if (event.flags.rescueDone)
                startReadySM();
            break;

    } // switch
    // Check if boat came online or if we lost connection
    checkBoatConnection();

    // Transition out of any state as long as not calibrating..
    if (state != STATE_CALIBRATE) {
        if (event.flags.haveError) {
            startErrorSM();
        }
        else if (state != STATE_SETORIGIN) {
            if (event.flags.stopButtonPressed
                    && (state != STATE_STOP || Timer_isExpired(TIMER_DEBOUNCE)))
                startStopSM();
            else if (event.flags.haveRequestOriginMessage)
                startSetOriginSM();
            else if (event.flags.rescueButtonPressed
                    && (state != STATE_RESCUE || Timer_isExpired(TIMER_DEBOUNCE))) {
                //DBPRINT("Ready button pressed.\n");
                startRescueSM();
            }
            else if (event.flags.setStationButtonPressed
                    && (state != STATE_SETSTATION || Timer_isExpired(TIMER_DEBOUNCE)))
                startSetStationSM();
        }
    }
    //DBPRINT("!\n");
} // doMasterSM()


/**********************************************************************
 * Function: startCalibrateSM
 * @return None
 * @remark Executes one cycle of the Stop state machine.
 **********************************************************************/
static void startCalibrateSM() {
	state = STATE_CALIBRATE;
	subState = STATE_CALIBRATE_PITCH;
	resendMessageCount = 0;
	Interface_clearAll();
        DELAY(5);
	Interface_showMessage(CALIBRATE_PITCH_MESSAGE);
	Interface_pitchLightsOn();
}

/**********************************************************************
 * Function: startReadySM
 * @return None
 * @remark Starts the Ready state.
 **********************************************************************/
static void startReadySM() {
	state = STATE_READY;
	subState = STATE_NONE; // no sub-states in Ready
	resendMessageCount = 0;
	Interface_readyLightOn();
	Interface_errorLightOff();
	Interface_waitLightOff();
	Interface_showMessage(READY_MESSAGE);
}

/**********************************************************************
 * Function: startErrorSM
 * @return None
 * @remark Start Error state.
 **********************************************************************/
static void startErrorSM() {
    state = STATE_ERROR;
    subState = STATE_NONE;

    /* Determine whether the error was from the boat or the compas,
       but display the ComPAS errors over the boat's. */
    if (event.flags.haveError)
        Interface_showErrorMessage(lastErrorCode);
    else if (event.flags.haveBoatError)
        Interface_showBoatErrorMessage(lastBoatErrorCode);

}

/**********************************************************************
 * Function: startSetStationSM
 * @return None
 * @return Transitions into the Set Station state.
 **********************************************************************/
static void startSetStationSM() {
    state = STATE_SETSTATION;
    subState = STATE_SETSTATION_SEND;
    Timer_new(TIMER_DEBOUNCE, RESCUE_DEBOUNCE_DELAY);
 
    Mavlink_sendSaveStation(WANT_ACK);
    Timer_new(TIMER_SETSTATION, RESEND_MESSAGE_DELAY);
    resendMessageCount = 0;

    Interface_clearAll();
    Interface_waitLightOn();
    Interface_showMessage(SAVING_STATION_MESSAGE);
}

/**********************************************************************
 * Function: startSetOriginSM
 * @return None
 * @return Transitions into the Set Origin state.
 **********************************************************************/
static void startSetOriginSM() {
    state = STATE_SETORIGIN;
    subState = STATE_NONE;

    updatePosition();
    Mavlink_sendOrigin(WANT_ACK, &ecefPosition);
    Timer_new(TIMER_SETORIGIN, RESEND_MESSAGE_DELAY);
    resendMessageCount = 0;

    Interface_clearAll();
    Interface_waitLightOn();
    Interface_showMessage(SETTING_ORIGIN_MESSAGE);
}

/**********************************************************************
 * Function: startRescueSM
 * @return None
 * @remark Start the Rescue state machine.
 **********************************************************************/
static void startRescueSM() {
    state = STATE_RESCUE;
    subState = STATE_RESCUE_SEND;
    Timer_new(TIMER_DEBOUNCE, RESCUE_DEBOUNCE_DELAY);

    // Send the target location to the boat and start the resend timer
    getTargetLocation(&nedRescueTarget);
    Mavlink_sendStartRescue(WANT_ACK, &nedRescueTarget);
    Timer_new(TIMER_RESCUE, RESEND_MESSAGE_DELAY);
    resendMessageCount = 0;

    Interface_clearAll();
    Interface_waitLightOn();
    Interface_showMessage(STARTING_RESCUE_MESSAGE);

    #ifdef DEBUG_RESCUE
    LCD_setPosition(2,0);
    char debug[50];
    sprintf(debug, "R: N=%.2f, E=%.2f\nE: Y=%.2f, P=%.2f",
        nedRescueTarget.north, nedRescueTarget.east,
        Encoder_getYaw(), Encoder_getPitch());
    LCD_writeString(debug);
    #endif
}


/**********************************************************************
 * Function: startStopSM
 * @return None
 * @remark Transitions into the Stop state machine.
 **********************************************************************/
static void startStopSM() {
    state = STATE_STOP;
    subState = STATE_STOP_SEND;
    Timer_new(TIMER_DEBOUNCE, RESCUE_DEBOUNCE_DELAY);

    // Send a stop message and resend on timer
    Mavlink_sendOverride(WANT_ACK);
    Timer_new(TIMER_STOP, RESEND_MESSAGE_DELAY);
    resendMessageCount = 0;

    Interface_clearAll();
    Interface_waitLightOn();
    Interface_showMessage(STOPPING_BOAT_MESSAGE);
}

static void updatePosition() {
    #ifdef USE_GPS_ORIGIN
    GPS_getPosition(&ecefPosition);
    #else
    ecefPosition.x = ECEF_X_ORIGIN;
    ecefPosition.y = ECEF_Y_ORIGIN;
    ecefPosition.z = ECEF_Z_ORIGIN;
    #endif
}

/**********************************************************************
 * Function: setError
 * @param Error code for error to set.
 * @return None
 * @remark Triggers an error and sets the current error to the error 
 *  code provided. Note that this function does not start the Error state.
 **********************************************************************/
static void setError(error_t errorCode) {
    event.flags.haveError = TRUE;
    lastErrorCode = errorCode;
}

static void fatal(error_t code) {

    DBPRINT("Fatal error: %s\n", getErrorMessage(code));
    Interface_showErrorMessage(code);
    Interface_errorLightOn();

    while(1) {
        // Lock up
        asm("nop");
    }
}

/**********************************************************************
 * Function: doBarometerUpdate
 * @return None.
 * @remark Handles a barometer message by calculating differential height.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void doBarometerUpdate() {
    if (event.flags.haveBarometerMessage) {
        compasHeight = Barometer_getAltitude()
            - Mavlink_newMessage.barometerData.altitude;

        haveCompasHeight = TRUE;
        Timer_new(TIMER_BAROMETER_LOST, BAROMETER_LOST_DELAY);
    }
    else if (haveCompasHeight && Timer_isExpired(TIMER_BAROMETER_LOST)) {
        setError(ERROR_NO_ALTITUDE);
        haveCompasHeight = FALSE;
    }
}

/**********************************************************************
 * Function: doGpsCorrectionUpdate
 * @return None.
 * @remark Calculates and sends the current GPS error correction data.
 * @author David Goodman
 * @date 2013.05.05
 **********************************************************************/
static void gpsCorrectionUpdate() {
    if (Timer_isExpired(TIMER_GPS_CORRECTION) && GPS_hasPosition()) {
        // Calculate error corrections
        GeocentricCoordinate ecefMeasured;
        GPS_getPosition(&ecefMeasured);

        GeocentricCoordinate ecefError;
        ecefError.x = ECEF_X_ORIGIN - ecefMeasured.x;
        ecefError.y = ECEF_Y_ORIGIN - ecefMeasured.y;
        ecefError.z = ECEF_Z_ORIGIN - ecefMeasured.z;

        // Send error corrections
        Mavlink_sendGeocentricError(&ecefError);

        Timer_new(TIMER_GPS_CORRECTION, GPS_CORRECTION_SEND_DELAY);
    }
}


/**********************************************************************
 * Function: getTargetLocation
 * @param A pointer to a ned coordinate variable to save the result into.
 * @return None
 * @remark Calculates and saves the local coordinate location of target 
 *	in the scope into the given variable.
 **********************************************************************/
static void getTargetLocation(LocalCoordinate *targetNed) {

    projectEulerToNED(targetNed, Encoder_getYaw(),
        Encoder_getPitch(), compasHeight);
}

/**********************************************************************
 * Function: checkBoatConnection
 * @return None
 * @remark Checks the connection to be boat and sets an error if the con-
 *  nection was lost.
 **********************************************************************/
static void checkBoatConnection() {
    if (event.flags.lostBoatHeartbeat) {
        // Lost connection to boat
        setError(ERROR_NO_HEARTBEAT);
    }
    else if (event.flags.haveBoatOnlineMessage && !isConnectedWithBoat) {
        // Boat just came online
        isConnectedWithBoat = TRUE;
        Timer_new(TIMER_HEARTBEAT_CHECK, HEARTBEAT_LOST_DELAY);
        Interface_clearDisplay();
        Interface_showMessageOnTimer(BOAT_ONLINE_MESSAGE,LCD_HOLD_DELAY);
    }
    else if (isConnectedWithBoat && Timer_isExpired(TIMER_HEARTBEAT_CHECK)) {
        // Got boat heartbeat, restart timer
        Timer_new(TIMER_HEARTBEAT_CHECK, HEARTBEAT_LOST_DELAY);
    }
}

/**********************************************************************
 * Function: restartCompas
 * @return None
 * @remark Resets the command center.
 **********************************************************************/
 static void resetCompas() {
    Interface_clearDisplay();
    Interface_showMessage(RESET_SYSTEM_MESSAGE);
    Interface_readyLightOff();
    Interface_waitLightOn();
    delay(RESET_HOLD_DELAY);
    SoftReset();
}


/**********************************************************************
 * Function: restartAll
 * @return None
 * @remark Sends a message to reset the boat, then resets the command center.
 **********************************************************************/
 static void resetAll() {
    Mavlink_sendResetBoat();
    Interface_clearDisplay();
    Interface_showMessage(RESET_BOAT_MESSAGE);
    Interface_readyLightOff();
    Interface_waitLightOn();
    DELAY(RESET_HOLD_DELAY);
    resetCompas();
 }

/**********************************************************************
 * Function: checkReset
 * @return None
 * @remark Decide if reset is desired, and which type.
 **********************************************************************/
 static void checkReset() {
     if (!resetPressedShort) {
         // Haven't triggered any type of reset yet
         if (event.flags.resetButtonPressed && Timer_isExpired(TIMER_RESET)) {
            // Held reset button for short timer at least
            resetPressedShort = TRUE;
            Timer_new(TIMER_RESET, RESET_LONG_HOLD_DELAY);
         }
         else if (event.flags.resetButtonPressed) {
            // Just pressed reset, start timer and wait
            Timer_new(TIMER_RESET, RESET_HOLD_DELAY);
         }
     }
     else  {
         // A reset is desired, determine which type
         if (!event.flags.resetButtonPressed) {
             // want a soft reset
             resetCompas();
         }
         else if (event.flags.resetButtonPressed && Timer_isExpired(TIMER_RESET)) {
             // want a full reset
             resetAll();
         }
     }
 }


/**********************************************************************
 * Function: initializeCompas
 * @return None
 * @remark Initializes the ComPAS and all modules.
 **********************************************************************/
static void initializeCompas() {
    Board_init();

    #ifdef DEBUG
    Serial_init();
    #endif

    Timer_init();

    #ifdef USE_INTERFACE
    DBPRINT("Initializing interface.\n");
    Interface_init();
    #endif

    #ifdef USE_LCD
    DBPRINT("Initializing LCD.\n");
    LCD_init();
    #endif

    // -------------------- I2C Devices -------------------
    // I2C bus
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);
    if (I2C_hasError) {
        fatal(ERROR_I2C);
    }

    #ifdef USE_MAGNETOMETER
    DBPRINT("Initializing magnetometer.\n");
    if (Magnetometer_init() != SUCCESS) {
        fatal(ERROR_MAGNETOMETER);
    }
    #endif

    #ifdef USE_ENCODER
    DBPRINT("Initializing encoders.\n");
    if (Encoder_init() != SUCCESS) {
        fatal(ERROR_ENCODER);
    }
    #endif

    #ifdef USE_ACCELEROMETER
    DBPRINT("Initializing accelerometer.\n");
    if (Accelerometer_init() != SUCCESS) {
        fatal(ERROR_ACCELEROMETER);
    }
    #endif

    #ifdef USE_BAROMETER
    DBPRINT("Initializing barometer.\n");
    if (Barometer_init() != SUCCESS) {
        fatal(ERROR_BAROMETER);
    }
    #endif

    // ------------------- UART Devices ------------------
    #ifdef USE_GPS
    DBPRINT("Initializing GPS.\n");
    if (GPS_init() != SUCCESS) {
        fatal(ERROR_GPS);
    }
    #endif

    #ifdef USE_XBEE
    DBPRINT("Initializing XBee.\n");
    if (Xbee_init() != SUCCESS)
        fatal(ERROR_XBEE);
    #endif

    #ifdef DEBUG_BLINK
    Interface_waitLightOnTimer(BLINK_ON_DELAY);
    Interface_runSM();
    Timer_new(TIMER_TEST, BLINK_DELAY);
    #endif


    #ifdef DEBUG_STATE
    Timer_new(TIMER_TEST2, DEBUG_PRINT_DELAY);
    #endif

    // Connection related
    isConnectedWithBoat  = FALSE;
    compasHeight = 5.3f;
    haveCompasHeight = FALSE;

    // Start calibrating before use
    startCalibrateSM();
    resendMessageCount = 0;

}

// ---------------------------- Main entry pont --------------------------
#define USE_COMPAS
#ifdef USE_COMPAS
int main() {
    initializeCompas();
    DBPRINT("ComPAS initialized.\n");
    while (1) {
        doMasterSM();
    }

    return SUCCESS;
}

#endif
