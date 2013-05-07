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
//#define DEBUG_VERBOSE

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
#include "Xbee.h"
#include "UART.h"
#include "Gps.h"
#include "Barometer.h"
#include "Override.h"
#include "Error.h"
#include "Interface.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
 
#define MESSAGE_LINGER_DELAY	5000 // (ms) to prevent the display from changing


// Timer allocation
#define TIMER_CALIBRATE     TIMER_MAIN
#define TIMER_RESCUE        TIMER_MAIN
#define TIMER_STOP          TIMER_MAIN
#define TIMER_SETORIGIN     TIMER_MAIN
#define TIMER_SETSTATION    TIMER_MAIN
#define TIMER_CANCEL        TIMER_MAIN2


#define TIMER_BAROMETER_LOST            TIMER_BACKGROUND
#define TIMER_HEARTBEAT_LOST            TIMER_BACKGROUND2
#define TIMER_GPS_CORRECTION            TIMER_BACKGROUND3

// Timer delays
#define BAROMETER_LOST_DELAY	    20000 // (ms) time before timeout error
#define HEARBEAT_LOST_DELAY         10000// (ms) before timeout error
#define RESEND_MESSAGE_DELAY        4000 // (ms) resend a message
#define GPS_CORRECTION_SEND_DELAY   3750
#define LCD_HOLD_DELAY              4000 // (ms) time for lcd message to linger
#define LED_HOLD_DELAY              1000 // (ms) time for led to stay lit
#define CANCEL_TIMEOUT_DELAY       3500 // (ms) for cancel msg to linger

#define RESEND_MESSAGE_LIMIT        5 // times to resend before failing

#define EVENT_BYTE_COUNT 10

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

#define I2C_CLOCK_FREQ  100000 // (Hz)

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
I2C_MODULE I2C_BUS_ID   = I2C1;

static enum {
    STATE_CALIBRATE,
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
    // N/A

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
	
} subState;

union EVENTS {
    struct {
        /* - Interface flags - */
        unsigned int rescueButtonPressed :1;
        unsigned int stopButtonPressed :1;
        unsigned int setStationButtonPressed :1;
        unsigned int okButtonPressed :1;
        unsigned int cancelButtonPressed :1;
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
        unsigned int readyToPrintMessage :1; // display hold for Ready state expired
        /* - Boat error messages - */
        unsigned int haveBoatGpsDisconnectedError :1;
        unsigned int haveBoatGpsFixError :1;
        unsigned int haveBoatNoOriginError :1;
        unsigned int haveBoatNoStationError :1;
        unsigned int haveBoatOnlineMessage  :1;
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

LocalCoordinate nedRescueTarget;
uint8_t resendMessageCount;
float compasHeight;
int lastMessageID;
error_t lastErrorCode;

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
void checkEvents();
void doCalibrateSM();
void doReadySM();
void doErrorSM();
void doRescueSM();
void doSetStationSM();
void doStopSM();
void doMasterSM();

void startCalibrateSM();
void startReadySM();
void startErrorSM();
void startRescueSM();
void startSetStationSM();
void startSetOriginSM();
void startStopSM();
void initializeCompas();

void setError(error_t errorCode);
void doBarometerUpdate();
void gpsCorrectionUpdate();
void getTargetLocation(LocalCoordinate *targetNed);


/***********************************************************************
 * PRIVATE FUNCTIONS                                                   *
 ***********************************************************************/

/**********************************************************************
 * Function: checkEvents
 * @return None
 * @remark Checks for various events that can occur, and sets the 
 *  appropriate flags for handling later.
 **********************************************************************/
void checkEvents() {
    // Clear all event flags
    int i;
    for (i=0; i < EVENT_BYTE_COUNT; i++)
        event.bytes[i] = 0x0;     

    // Calibration flags
    if (state == STATE_CALIBRATE) {
        if (Accelerometer_isLevel())
            event.flags.scopeIsLevel = TRUE;
        if (Magnetometer_isNorth())
            event.flags.scopeIsNorth = TRUE;
    }


    // Canceling out of the error state chooses Ready or Stop
    if (lastState == STATE_READY)
        event.flags.lastStateWasReady;

    // Check interface
    if (Interface_isOkPressed())
        event.flags.okButtonPressed;
    if (Interface_isCancelPressed())
        event.flags.cancelButtonPressed;
    if (Interface_isSetStationPressed())
        event.flags.setStationButtonPressed;
    if (Interface_isStopPressed())
        event.flags.stopButtonPressed;
    if (Interface_isRescuePressed())
        event.flags.rescueButtonPressed;

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
                if (Mavlink_newMessage.statusAndErrorData.status == ERROR_GPS_DISCONNECTED) {
                    event.flags.haveBoatGpsDisconnectedError = TRUE;
                    event.flags.haveBoatError = TRUE;
                }
                if (Mavlink_newMessage.statusAndErrorData.status == ERROR_GPS_NOFIX) {
                    event.flags.haveBoatGpsFixError = TRUE;
                    event.flags.haveBoatError = TRUE;
                }
                if (Mavlink_newMessage.statusAndErrorData.status == ERROR_NO_ORIGIN) {
                    event.flags.haveBoatNoOriginError = TRUE;
                    event.flags.haveBoatError = TRUE;
                }
                if (Mavlink_newMessage.statusAndErrorData.status == ERROR_NO_STATION) {
                    event.flags.haveBoatNoStationError = TRUE;
                    event.flags.haveBoatError = TRUE;
                }
                // ---------- Boat status messages -----------------
                if (Mavlink_newMessage.statusAndErrorData.status == MAVLINK_STATUS_ONLINE)
                    event.flags.haveBoatOnlineMessage = TRUE;
            /*----------------  Barometer altitude message ----------------*/
            case MAVLINK_MSG_ID_BAROMETER:
                event.flags.haveBarometerMessage = TRUE;
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
void doCalibrateSM() {
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
                    Timer_new(TIMER_CALIBRATE);
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
                    Interface_showMessageOnTimer(CALIBRATE_SUCCESS_MESSAGE,LCD_HOLD_DELAY);
                    Interface_readyLightOnTimer(LED_HOLD_DELAY);
                    Interface_yawLightsOff();
                    event.flags.calibrateDone = TRUE;
                }
                else if (!Timer_isActive(TIMER_CALIBRATE))
                    Timer_new(TIMER_CALIBRATE);
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
void doRescueSM() {
    switch (subState) {
        case STATE_RESCUE_SEND:
            // Tell boat to rescue someone
            if (event.flags.haveStartRescueAck) {
                // Transition to wait substate
                subState = STATE_RESCUE_WAIT;
                Interface_showMessage(STARTED_RESCUE_MESSAGE);
                Interface_waitLightOff();
                resendMessageCount = 0;
                Interface_readyLightOn();
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
            break;
        case STATE_RESCUE_WAIT:
            // Wait for boat to rescue someone
            if (event.flags.haveRescueSuccessMessage) {
                // Person was found! Exit to ready
                event.flags.rescueDone = TRUE;
                Interface_showMessageOnTimer(RESCUE_SUCCESS_MESSAGE, LCD_HOLD_DELAY);
            }
            else if (event.flags.cancelButtonPressed) {
                // Transition to confirmcancel substate
                subState = STATE_RESCUE_CONFIRMCANCEL;
                Timer_new(TIMER_RESCUE, CANCEL_TIMEOUT_DELAY);
                Interface_showMessage(CANCEL_RESCUE_MESSAGE);
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
                subState = STATE_RESCUE_RETURN;
                Interface_showMessage(START_RETURN_MESSAGE);
                Interface_readyLightOff();
                Interface_waitLightOn();
                resendMessageCount = 0;
                Mavlink_sendReturnStation(WANT_ACK);
                Timer_new(TIMER_RESCUE, RESEND_MESSAGE_DELAY);
            }
            else if (Timer_isExpired(TIMER_RESCUE)
                || event.flags.cancelButtonPressed) {
                // Transition to wait substate
                Timer_clear(TIMER_RESCUE);
                subState = STATE_RESCUE_WAIT;
                Interface_showMessage(STARTED_RESCUE_MESSAGE);
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
            break;
    }
} // doRescueSM

/**********************************************************************
 * Function: doStopSM
 * @return None
 * @remark Executes one cycle of the Stop state machine.
 **********************************************************************/
void doStopSM() {
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
            break;
        case STATE_STOP_IDLE:
            // Wait for something
            if (event.flags.cancelButtonPressed) {
                // Transition to confirmcancel substate
                subState = STATE_STOP_CONFIRMCANCEL;
                Timer_new(TIMER_STOP, CANCEL_TIMEOUT_DELAY);
                Interface_showMessage(CANCEL_STOP_MESSAGE);
            }
            break;
        case STATE_STOP_CONFIRMCANCEL:
            // Ask user if they want to cancel stop
            if (event.flags.okButtonPressed) {
                // Transition to return substate
                subState = STATE_STOP_RETURN;
                Interface_showMessage(START_RETURN_MESSAGE);
                Interface_readyLightOff();
                Interface_waitLightOn();
                resendMessageCount = 0;
                Mavlink_sendReturnStation(WANT_ACK);
                Timer_new(TIMER_STOP, RESEND_MESSAGE_DELAY);
            }
            else if (Timer_isExpired(TIMER_STOP)
                || event.flags.cancelButtonPressed) {
                // Transition to wait substate
                Timer_clear(TIMER_STOP);
                subState = STATE_STOP_IDLE;
                Interface_showMessage(STOPPED_BOAT_MESSAGE);
            }
            break;
        case STATE_STOP_RETURN:
            // Tell boat to return to station
            if (event.flags.haveReturnStationAck) {
                // Boat is headed to station, exits to ready
                event.flags.stopDone = TRUE;
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
            break;
    }  // switch

}

/**********************************************************************
 * Function: doSetStationSM
 * @return None
 * @remark Executes one cycle of the Set Station state.
 **********************************************************************/
void doSetStationSM() {
    if (event.flags.haveSetStationAck) {
        Interface_showMessageOnTimer(SAVED_STATION_MESSAGE, LCD_HOLD_DELAY);
        event.flags.setStationDone = TRUE;
    }
}

/**********************************************************************
 * Function: doSetOriginSM
 * @return None
 * @remark Executes one cycle of the Set Origin state.
 **********************************************************************/
void doSetOriginSM() {
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
void doMasterSM() {
    checkEvents();

    #ifdef USE_TILTCOMPASS
    TiltCompass_runSM();
    #endif

    #ifdef USE_MAGNETOMETER
    if (state = STATE_CALIBRATE)
        Magnetometer_runSM();
    #endif


    #ifdef USE_ENCODER
    Encoder_runSM();
    #endif

    #ifdef USE_ACCELEROMETER
    if (state = STATE_CALIBRATE)
        Accelerometer_runSM();
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
            if (event.flags.okButtonPressed) {
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
    // Transition out of any state as long as not calibrating..
    if (state != STATE_CALIBRATE) {
    if (event.flags.haveError) {
        startErrorSM();
    }
    if (state != STATE_SETORIGIN) {
        if (event.flags.stopButtonPressed)
            startStopSM();
        else if (event.flags.haveRequestOriginMessage)
            startSetOriginSM();
        else if (event.flags.rescueButtonPressed)
            startRescueSM();
        else if (event.flags.setStationButtonPressed)
            startSetStationSM();
        }
    }
}


/**********************************************************************
 * Function: startCalibrateSM
 * @return None
 * @remark Executes one cycle of the Stop state machine.
 **********************************************************************/
void startCalibrateSM() {
	state = STATE_CALIBRATE;
	subState = STATE_CALIBRATE_PITCH;
	resendMessageCount = 0;
	Interface_clearAll();
	Interface_showMessage(CALIBRATE_PITCH_MESSAGE);
	Interface_pitchLightsOn();
}

/**********************************************************************
 * Function: startReadySM
 * @return None
 * @remark Starts the Ready state.
 **********************************************************************/
void startReadySM() {
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
void startErrorSM() {
    state = STATE_ERROR;
    subState = STATE_NONE;
    if (lastErrorCode == ERROR_NONE)
        lastErrorCode = ERROR_UNKNOWN;

    Interface_showErrorMessage(lastErrorCode);
}

/**********************************************************************
 * Function: startSetStationSM
 * @return None
 * @return Transitions into the Set Station state.
 **********************************************************************/
void startSetStationSM() {
    state = STATE_SETSTATION;
    subState = STATE_NONE;
 
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
void startSetOriginSM() {
    state = STATE_SETORIGIN;
    subState = STATE_NONE;
 
    GPS_getPosition(&ecefPosition);
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
void startRescueSM() {
    state = STATE_RESCUE;
    subState = STATE_RESCUE_SEND;

    // Send the target location to the boat and start the resend timer
    getTargetLocation(&nedRescueTarget);
    Mavlink_sendStartRescue(WANT_ACK, &nedRescueTarget);
    Timer_new(TIMER_RESCUE, RESEND_MESSAGE_DELAY);
    resendMessageCount = 0;

    Interface_clearAll();
    Interface_waitLightOn();
    Interface_showMessage(STARTING_RESCUE_MESSAGE);
}


/**********************************************************************
 * Function: startStopSM
 * @return None
 * @remark Transitions into the Stop state machine.
 **********************************************************************/
void startStopSM() {
    state = STATE_STOP;
    subState = STATE_STOP_SEND;

    // Send a stop message and resend on timer
    Mavlink_sendOverride(WANT_ACK);
    Timer_new(TIMER_STOP, RESEND_MESSAGE_DELAY);
    resendMessageCount = 0;

    Interface_clearAll();
    Interface_waitLightOn();
    Interface_showMessage(STOPPING_BOAT_MESSAGE);
}

/**********************************************************************
 * Function: initializeCompas
 * @return None
 * @remark Initializes the ComPAS and all modules.
 **********************************************************************/
void initializeCompas() {
    Board_init();
    Serial_init();
    Timer_init();

    // I2C bus
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);

    #ifdef USE_MAGNETOMETER
    Magnetometer_init();
    #endif

    #ifdef USE_ENCODER
    Encoder_init();
    #endif

    #ifdef USE_ACCELEROMETER
    Accelerometer_init();
    #endif

    #ifdef USE_GPS
    GPS_init();
    #endif

    #ifdef USE_XBEE
    Xbee_init();
    #endif

    #ifdef USE_BAROMETER
    Barometer_init();
    #endif
        
    // Start calibrating before use
    startCalibrateSM();
	resendMessageCount = 0;
}

/**********************************************************************
 * Function: setError
 * @param Error code for error to set.
 * @return None
 * @remark Triggers an error and sets the current error to the error 
 *  code provided. Note that this function does not start the Error state.
 **********************************************************************/
void setError(error_t errorCode) {
    event.flags.haveError = TRUE;
    lastErrorCode = errorCode;
}

/**********************************************************************
 * Function: doBarometerUpdate
 * @return None.
 * @remark Handles a barometer message by calculating differential height.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
void doBarometerUpdate() {
    if (event.flags.haveBarometerMessage) {
        compasHeight = Barometer_getAltitude()
            - Mavlink_newMessage.barometerData.altitude;

        Timer_new(TIMER_BAROMETER_LOST, BAROMETER_LOST_DELAY);
    }
    else if (Timer_isExpired(TIMER_BAROMETER_LOST)) {
        setError(ERROR_NO_ALTITUDE);
    }
}

/**********************************************************************
 * Function: doGpsCorrectionUpdate
 * @return None.
 * @remark Calculates and sends the current GPS error correction data.
 * @author David Goodman
 * @date 2013.05.05
 **********************************************************************/
void gpsCorrectionUpdate() {
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
void getTargetLocation(LocalCoordinate *targetNed) {

    projectEulerToNED(targetNed, Encoder_getYaw(),
        Encoder_getPitch(), compasHeight);
}


int main() {
    initializeCompas();
    while (1) {
        doMasterSM();
    }

    return SUCCESS;
}

