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
//#define DEBUG
//#define DEBUG_VERBOSE

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <stdlib.h>

#include "Board.h"
#include "Serial.h"
#include "Ports.h"
#include "Magnetometer.h"
#include "Gps.h"
#include "Navigation.h"
#include "Drive.h"
#include "Mavlink.h"
#include "Override.h"
#include "Barometer.h"
#include "Interface.h"
#include "Error.h"
#include "TiltCompass.h"
#include "Uart.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define XBEE_UART_ID    UART1_ID

// Module selection (comment a line out to disable the module)
#define USE_OVERRIDE
#define USE_NAVIGATION
#define USE_GPS
#define USE_DRIVE
#define USE_TILTCOMPASS
//#define USE_XBEE
//#define USE_SIREN
#define USE_BAROMETER
#define DO_HEARTBEAT
#define DEBUG

#ifdef DEBUG
#ifdef USE_SD_LOGGER
#define DBPRINT(...)   do { char debug[255]; sprintf(debug,__VA_ARGS__); } while(0)
#else
#define DBPRINT(...)   printf(__VA_ARGS__)
#endif
#else
#define DBPRINT(...)   ((int)0)
#endif


#define EVENT_BYTE_SIZE     10 // provides 80 event bits

// Timer allocation
#define TIMER_STATIONKEEP   TIMER_MAIN
#define TIMER_SETORIGIN     TIMER_MAIN

#define TIMER_BAROMETER_SEND        TIMER_BACKGROUND
#define TIMER_GPS_CORRECTION_LOST   TIMER_BACKGROUND2
#define TIMER_HEARTBEAT             TIMER_BACKGROUND3

// --------------------- Timer delays -----------------------
#define STARTUP_DELAY               2500 // (ms) time to wait before starting up
#define STATION_KEEP_DELAY          3000 // (ms) to check if drifted away
#define BAROMETER_SEND_DELAY        10000 // (ms) time between sending barometer data
#define RESEND_MESSAGE_DELAY        4000 // (ms) resend a message
#define GPS_CORRECTION_LOST_DELAY   5000 // (ms) throw out gps error corrections now
#define HEARTBEAT_SEND_DELAY        3000 // (ms) between heart being sent to CC

#define RESEND_MESSAGE_LIMIT        5 // times to resend before failing

// Maximum substate errors before failing (fall into override)
#define INITIALIZE_ERROR_MAX            5

#define STATION_TOLERANCE_MIN           5.0f // (meters) to approach station
#define STATION_TOLERANCE_MAX           12.0f // (meters) distance to float away
#define RESCUE_TOLERANCE                2.0f // (meters) to approach person

// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
I2C_MODULE I2C_BUS_ID   = I2C1;


// ---- States for Master SM ----
static enum {
    STATE_SETORIGIN,    // Obtain location of command center.
    STATE_SETSTATION,    // Obtain location of station keep point
    STATE_STATIONKEEP,  // Maintaining station coordinates
    STATE_OVERRIDE,     // Remote override activated
    STATE_RESCUE,       // Rescuing a drowning person
} state;

// ---- State for Sub SM ------
static enum {
    STATE_NONE = 0x0,

    /* - Rescue SM - */
    STATE_RESCUE_WAIT,  
    STATE_RESCUE_GOTO,
    STATE_RESCUE_SEARCH,
    STATE_RESCUE_SUPPORT,

    /* - Station Keep SM - */
    STATE_STATIONKEEP_RETURN,
    STATE_STATIONKEEP_IDLE,

    /* - SetOrigin SM - */
    // N/A

    /* - SetStation SM - */
    // N/A


} subState;


static union EVENTS {
    struct {
        /* - Mavlink message flags - */
        // Acknowledgements
        unsigned int haveRequestOriginAck : 1;
        // Messages
        unsigned int haveResetMessage :1;
        unsigned int haveOverrideMessage :1;
        unsigned int haveReturnStationMessage :1;
        unsigned int haveSetStationMessage :1;
        unsigned int haveSetOriginMessage :1;
        unsigned int haveGeocentricErrorMessage :1;
        unsigned int haveStartRescueMessage :1;
        unsigned int haveBarometerMessage :1;
        unsigned int haveUnknownMessage :1;
        /*  - Navigation events - */
        unsigned int navigationDone :1;
        unsigned int navigationError :1;
        /* - Rescue events - */
        unsigned int rescueFail :1;
        /* - Station Keep events - */
        unsigned int stationKeepFail :1;
        /* Set/save station and origin */
        unsigned int setStationDone :1;
        unsigned int saveStationDone :1;
        unsigned int setOriginDone :1;
        /* - Override events - */
        unsigned int receiverDetected :1;
        unsigned int wantOverride :1;
        unsigned int haveError :1;
    } flags;
    unsigned char bytes[EVENT_BYTE_SIZE]; // allows for 80 bitfields
} event;


static bool overrideShutdown = FALSE; //  whether to force override
static bool haveStation = FALSE;
static bool haveOrigin = FALSE;
static bool wantSaveStation = FALSE;
static bool wantOverride = FALSE;
static bool haveXbee = FALSE;

static LocalCoordinate nedStation; // NED coordinate with station location
static LocalCoordinate nedRescue; // NED coordinate of drowning person

static int lastMavlinkMessageID; // ID of most recently received Mavlink message
static int lastMavlinkCommandID; // Command code of last message (for ACK)
static char lastMavlinkMessageWantsAck;
static uint8_t resendMessageCount;

static error_t lastErrorCode = ERROR_NONE;


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
static void checkEvents();
static void doSetStationSM();
static void doSetOriginSM();
static void doStationKeepSM();
static void doOverrideSM();
static void doRescueSM();
static void doMasterSM();
static void startSetStationSM();
static void startSetOriginSM();
static void startStationKeepSM();
static void startOverrideSM();
static void startRescueSM();
static void initializeAtlas();
static void resetAtlas();
static void handleAcknowledgement();
static void setError(error_t errorCode);
static void gpsCorrectionUpdate();
static void doBarometerUpdate();
static void doHeartbeatMessage();
void fatal(error_t code);


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
    for (i=0; i < EVENT_BYTE_SIZE; i++)
        event.bytes[i] = 0x0;       


    // Navigation events
    if (Navigation_isDone())
        event.flags.navigationDone = TRUE;

    if (Navigation_hasError())
        setError(Navigation_getError());

    // Override
    if (Override_isTriggered())
        event.flags.receiverDetected = TRUE;


    // XBee messages (from command center)
    if (Mavlink_hasNewMessage()) {
        lastMavlinkMessageID = Mavlink_getNewMessageID();
        lastMavlinkCommandID = MAVLINK_NO_COMMAND;
        lastMavlinkMessageWantsAck = FALSE;
        switch (lastMavlinkMessageID) {
            // -------------------------- Acknowledgements ------------------------
            case MAVLINK_MSG_ID_MAVLINK_ACK:
                // No ACKS from ComPAS to AtLAs
                break;
            
            // ------------------------------ Messages ----------------------------
            case MAVLINK_MSG_ID_CMD_OTHER:
                lastMavlinkMessageWantsAck = Mavlink_newMessage.commandOtherData.ack;
                if (Mavlink_newMessage.commandOtherData.command == MAVLINK_RESET_BOAT ) {
                    event.flags.haveResetMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_RESET_BOAT ;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_RETURN_STATION) {
                    event.flags.haveReturnStationMessage = TRUE;
                    overrideShutdown = FALSE;
                    lastMavlinkCommandID = MAVLINK_RETURN_STATION;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_SAVE_STATION) {
                    event.flags.haveSetStationMessage = TRUE;
                    wantSaveStation = TRUE;
                    lastMavlinkCommandID = MAVLINK_SAVE_STATION;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_GEOCENTRIC_ORIGIN) {
                    event.flags.haveSetOriginMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_GEOCENTRIC_ORIGIN;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_OVERRIDE) {
                    event.flags.haveOverrideMessage = TRUE;
                    overrideShutdown = TRUE;
                    lastMavlinkCommandID = MAVLINK_OVERRIDE;
                }
                break;
            case MAVLINK_MSG_ID_GPS_ECEF:
                lastMavlinkMessageWantsAck = Mavlink_newMessage.gpsGeocentricData.ack;
                if (Mavlink_newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ORIGIN) {
                    event.flags.haveSetOriginMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_GEOCENTRIC_ORIGIN;
                }
                else if (Mavlink_newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ERROR)
                    event.flags.haveGeocentricErrorMessage = TRUE;
                break;
            case MAVLINK_MSG_ID_GPS_NED:
                lastMavlinkMessageWantsAck = Mavlink_newMessage.gpsLocalData.ack;
                if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_SET_STATION) {
                    event.flags.haveSetStationMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_LOCAL_SET_STATION;
                    wantSaveStation = FALSE;
                }
                else if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_START_RESCUE) {
                    event.flags.haveStartRescueMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_LOCAL_START_RESCUE;
                }
                break;
            default:
                // Unknown message ID
                event.flags.haveUnknownMessage = TRUE;
                DBPRINT("Mavlink received unhandled message: 0x%X\n");
                break;
        }
    }
    /* If the receiver is off, boat is not stopped, and we either
        have a station or are setting one, then turn override off */
    if (!event.flags.receiverDetected && !overrideShutdown 
        && (haveStation || (state == STATE_SETSTATION))) {

        event.flags.wantOverride = FALSE;
    }
    else {
        event.flags.wantOverride = TRUE;
    }
} //  checkEvents()


/**********************************************************************
 * Function: doSetOriginSM
 * @return None
 * @remark Steps into the set origin state machine.
 **********************************************************************/
static void doSetOriginSM() {
    // Waiting for origin message from command center 
    if (event.flags.haveSetOriginMessage) {
        GeocentricCoordinate ecefOrigin;
        ecefOrigin.x = Mavlink_newMessage.gpsGeocentricData.x;
        ecefOrigin.y = Mavlink_newMessage.gpsGeocentricData.y;
        ecefOrigin.z = Mavlink_newMessage.gpsGeocentricData.z;
        Navigation_setOrigin(&ecefOrigin);

        handleAcknowledgement();
        haveOrigin = TRUE;
        event.flags.setOriginDone = TRUE;

        DBPRINT("Set new origin: X=%.1f, Y=%.1f, Z=%.1f\n",
            ecefOrigin.x, ecefOrigin.y, ecefOrigin.z);
    }
    else {
        // Resend request if timer expires
        if (Timer_isExpired(TIMER_SETORIGIN)) {
            // Resend request origin if timed out
            if (resendMessageCount >= RESEND_MESSAGE_LIMIT) {
                // Sent too many times
                setError(ERROR_NO_ORIGIN);
                return;
            }
            else {
                DBPRINTF("Resending origin request.\n");
                Mavlink_sendRequestOrigin(NO_ACK); // just want message
                Timer_new(TIMER_SETORIGIN, RESEND_MESSAGE_DELAY);
                resendMessageCount++;
            }
        } // timer expired
    }
}


/**********************************************************************

 * @return None
 * @remark Steps into the set station state machine.
 **********************************************************************/
 static void doSetStationSM() {
    /* Decide whether to save the current postion as a station, or
        a given position. */
    if (wantSaveStation) {
        // Save current position as station
        Navigation_getLocalPosition(&nedStation);

        handleAcknowledgement();
        haveStation = TRUE;
        event.flags.setStationDone = TRUE;

        DBPRINT("Saved new station: N=%.2f, E=%.2f, D=%.2f\n",
            nedStation.north, nedStation.east, nedStation.down);
     }
     else {
         // Set the given coordinate as station
        nedStation.north = Mavlink_newMessage.gpsLocalData.north;
        nedStation.east = Mavlink_newMessage.gpsLocalData.east;
        nedStation.down = Mavlink_newMessage.gpsLocalData.down;

        handleAcknowledgement();
        haveStation = TRUE;
        event.flags.setStationDone = TRUE;
        
        DBPRINT("Set new station: N=%.2f, E=%.2f, D=%.2f.\n",
            nedStation.north, nedStation.east, nedStation.down);
    }
}


/**********************************************************************
 * Function: doStationKeepSM
 * @return None
 * @remark Steps into the station keeping state machine.
 **********************************************************************/
static void doStationKeepSM() {
    switch (subState) {
        case STATE_STATIONKEEP_RETURN:
            // Driving to the station
            if (event.flags.navigationDone) {
                subState = STATE_STATIONKEEP_IDLE;
                Timer_new(TIMER_STATIONKEEP,STATION_KEEP_DELAY);
                DBPRINT("Arrived at station.\n");
            }
            // TODO obstacle detection
            break;
        case STATE_STATIONKEEP_IDLE:
            // Wait to float away from the station
            if (Timer_isExpired(TIMER_STATIONKEEP)) {
                // Check if we floated too far away from the station
                if (Navigation_getLocalDistance(&nedStation) > STATION_TOLERANCE_MAX) {
                    startStationKeepSM(); // return to station
                    return;
                }

                Timer_new(TIMER_STATIONKEEP, STATION_KEEP_DELAY);
            }
            break;
    }
    if (event.flags.navigationError) {
        setError((error_t)Navigation_getError());
    }
}


/**********************************************************************
 * Function: doOverrideSM
 * @return None
 * @remark Executes one cycle of the boat's override state machine.
 **********************************************************************/
static void doOverrideSM() {
    // Do nothing
}


/**********************************************************************
 * Function: doRescueSM
 * @return None
 * @remark Executes one cycle of the boat's rescue state machine.
 **********************************************************************/
static void doRescueSM() {
    switch (subState) {
        case STATE_RESCUE_GOTO:
            if (event.flags.navigationDone) {
                subState = STATE_RESCUE_SEARCH;
                Navigation_cancel();
            }
            break;
        case STATE_RESCUE_SEARCH:
            // Human sensor event handling
            // Falls through to success for now
            //if 
            subState = STATE_RESCUE_SUPPORT;
            DBPRINT("Rescue mission complete.\n");
            Navigation_cancel();
            Mavlink_sendStatus(MAVLINK_STATUS_RESCUE_SUCCESS);
            break;
        case STATE_RESCUE_SUPPORT:
            // Do nothing
            break;
    }
    
}

/**********************************************************************
 * Function: doMasterSM
 * @return None.
 * @remark Executes one cycle of the boat's master state machine.
 * @author David Goodman
 * @date 2013.03.28 
 **********************************************************************/
static void doMasterSM() {
    checkEvents();

    #ifdef USE_TILTCOMPASS
    TiltCompass_runSM();
    #endif

    #ifdef USE_GPS
    GPS_runSM();
    #endif

    #ifdef USE_NAVIGATION
    Navigation_runSM();
    #ifdef USE_ERROR_CORRECTION
    gpsCorrectionUpdate();
    #endif
    #endif

    #ifdef USE_DRIVE
    Drive_runSM();
    #endif

    #ifdef USE_XBEE
    Xbee_runSM();
    #endif

    #ifdef USE_BAROMETER
    Barometer_runSM();
    doBarometerUpdate(); // send barometer data
    #endif

    #ifdef DO_HEARTBEAT
    doHeartbeatMessage();
    #endif

    switch (state) {
        case STATE_SETSTATION:
            doSetStationSM();

            if (event.flags.haveStartRescueMessage) {
                startRescueSM();
            }
            else if (event.flags.setStationDone) {
                if (haveOrigin)
                    startStationKeepSM();
                else
                    startOverrideSM();   
            }
            
            break;
        case STATE_SETORIGIN:
            doSetOriginSM();
            
            if (event.flags.setOriginDone)
                startOverrideSM();   
            
            break;
        case STATE_STATIONKEEP:
            doStationKeepSM();

            if (event.flags.haveStartRescueMessage)
                startRescueSM();
            else if (!haveStation)
                setError(ERROR_NO_STATION);
            break;

        case STATE_OVERRIDE:
            if (!wantOverride) {
                    //setError(ERROR_NO_ORIGIN);
                if (!haveOrigin)
                    startSetOriginSM(); // do we ant infinite startup loop?
                else if (event.flags.haveStartRescueMessage)
                    startRescueSM();
                else if (event.flags.haveSetStationMessage )
                    startSetStationSM();
                else if (haveOrigin && haveStation)
                    startStationKeepSM();
                
                // Use autonomous controls
                if (haveOrigin && (haveStation
                    || event.flags.haveStartRescueMessage)) {
                    Override_giveMicroControl();
                    DBPRINT("Micro has control.\n");
                    #ifdef USE_SIREN
                    Siren_blueLightOff();
                    #endif
                }
            }

            break;

        case STATE_RESCUE:
            doRescueSM();

            
            if (event.flags.haveStartRescueMessage) {
                startRescueSM();
            }
            else if (event.flags.haveReturnStationMessage) {
                if (haveStation)
                    startStationKeepSM();
                else 
                    setError(ERROR_NO_STATION);
            }
            // Turn off rescue siren (red)
            if (event.flags.haveError || state != STATE_RESCUE) {
                #ifdef USE_SIREN
                Siren_redLightOff();
                #endif
            }

            break;
    }
    //  ------- Caught by most states -----------
    if (state != STATE_RESCUE) {
        if (event.flags.haveSetStationMessage)
            startSetStationSM();
    }
    if (state != STATE_OVERRIDE) {
        if (event.flags.haveError) {
            startOverrideSM();
            overrideShutdown = TRUE;
        }
        if (wantOverride)
            startOverrideSM();
    }
    if (event.flags.haveResetMessage)
        resetAtlas();
}


//------------------------ Start States -------------------------------

/**********************************************************************
 * Function: startSetOriginSM
 * @return None.
 * @remark Transitions into the set origin state machine.
 * @author David Goodman
 * @date 2013.05.04  
 **********************************************************************/
static void startSetOriginSM() {
    state = STATE_SETORIGIN;
    subState = STATE_NONE;

    // Send status message to command center to request origin coordinate
    Mavlink_sendStatus(MAVLINK_REQUEST_ORIGIN);

    resendMessageCount = 0;
    Timer_new(TIMER_SETORIGIN, RESEND_MESSAGE_DELAY);
    DBPRINT("Requesting origin.\n");
}

/**********************************************************************
 * Function: startSetStationSM
 * @return None.
 * @remark Transitions into the set station state machine.
 * @author David Goodman
 * @date 2013.05.04  
 **********************************************************************/
static void startSetStationSM() {
    state = STATE_SETSTATION;
    subState = STATE_NONE;
    Navigation_cancel();
}

/**********************************************************************
 * Function: startStationKeepSM
 * @return None.
 * @remark Transitions into the station keeping state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
static void startStationKeepSM() {
    state = STATE_STATIONKEEP;
    subState = STATE_STATIONKEEP_RETURN;

    // Send status message for debugging
    Mavlink_sendStatus(MAVLINK_STATUS_RETURN_STATION);

    Navigation_gotoLocalCoordinate(&nedStation, STATION_TOLERANCE_MIN);

    DBPRINT("Headed to station.\n");
}

/**********************************************************************
 * Function: startOverrideSM
 * @return None.
 * @remark Transitions into the override state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
static void startOverrideSM() {
    state = STATE_OVERRIDE;
    subState = STATE_NONE;

    Override_giveReceiverControl();
    DBPRINT("Reciever has control.\n");
    Mavlink_sendStatus(MAVLINK_STATUS_OVERRIDE);
    Navigation_cancel();

    #ifdef USE_SIREN
    Siren_blueLightOn();
    #endif
}

/**********************************************************************
 * Function: startRescueSM
 * @return None.
 * @remark Transitions into the rescue state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
static void startRescueSM() {
    state = STATE_RESCUE;
    subState = STATE_RESCUE_GOTO;


    // Get location data from message
    nedRescue.north = Mavlink_newMessage.gpsLocalData.north;
    nedRescue.east = Mavlink_newMessage.gpsLocalData.east;
    nedRescue.down = Mavlink_newMessage.gpsLocalData.down;

    // Send status message for debugging
    Mavlink_sendStatus(MAVLINK_STATUS_START_RESCUE);
    Navigation_gotoLocalCoordinate(&nedStation, RESCUE_TOLERANCE);

    #ifdef USE_SIREN
    Siren_redLightOn();
    #endif

    DBPRINT("Rescuing person at: N=%.2f, E=%.2f, D=%.2f.\n",
        nedRescue.north, nedRescue.east, nedRescue.down);
}




/**********************************************************************
 * Function: handleAcknowledgement
 * @return None.
 * @remark Sends an ACK for the last message that arrived if desired.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void handleAcknowledgement() {
    // Send ACK if desired.
    if (lastMavlinkMessageWantsAck)
        Mavlink_sendAck(lastMavlinkMessageID, lastMavlinkCommandID);
}

/**********************************************************************
 * Function: setError
 * @param Error code to set current error to.
 * @return None.
 * @remark Sets an error that occurred.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void setError(error_t errorCode) {
    lastErrorCode = errorCode;
    event.flags.haveError = TRUE;
    Mavlink_sendError(errorCode);
    DBPRINT("Error: %s\n", getErrorMessage(errorCode));
}

void fatal(error_t code) {

    DBPRINT("Fatal error: %s\n", getErrorMessage(code));
    if (haveXbee)
        Mavlink_sendError(code);

    while(1) {
        // Lock up
        asm("nop");
    }
}

/**********************************************************************
 * Function: doBarometerUpdate
 * @return None.
 * @remark Sends barometer data such as temperature and altitude if 
 *  the timer expired.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void doBarometerUpdate() {
    if (Timer_isExpired(TIMER_BAROMETER_SEND)) {

        Mavlink_sendBarometerData(Barometer_getTemperature(),
            Barometer_getAltitude());

        Timer_new(TIMER_BAROMETER_SEND, BAROMETER_SEND_DELAY);
    }
}

/**********************************************************************
 * Function: doHeartbeatMessage
 * @return None.
 * @remark Occasionally sends a heartbeat message to the ComPAS.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void doHeartbeatMessage() {
    if (Timer_isExpired(TIMER_HEARTBEAT)) {

        Mavlink_sendHeartbeat();

        Timer_new(TIMER_HEARTBEAT, HEARTBEAT_SEND_DELAY);
    }
}


/**********************************************************************
 * Function: doGpsCorrectionUpdate
 * @return None.
 * @remark Receives GPS correction data and applies it to the navigation
 *  module, or turns it off if no new message were received.
 * @author David Goodman
 * @date 2013.05.05
 **********************************************************************/
static void gpsCorrectionUpdate() {
    if (event.flags.haveGeocentricErrorMessage) {
        GeocentricCoordinate ecefError;
        ecefError.x = Mavlink_newMessage.gpsGeocentricData.x;
        ecefError.y = Mavlink_newMessage.gpsGeocentricData.y;
        ecefError.z = Mavlink_newMessage.gpsGeocentricData.z;
        Navigation_setGeocentricError(&ecefError);
        if (!Navigation_isUsingErrorCorrection())
            DBPRINT("Error corrections enabled.\n");

        Navigation_enableErrorCorrection();

        Timer_new(TIMER_GPS_CORRECTION_LOST, GPS_CORRECTION_LOST_DELAY);
    }
    else if (Timer_isExpired(TIMER_GPS_CORRECTION_LOST)) {
        // Disable error corrections
        Navigation_disableErrorCorrection();
        DBPRINT("Error corrections disabled due to telemetry timeout.\n");
    }
}


/**********************************************************************
 * Function: resetAtlas
 * @return None.
 * @remark Resets the boat and reinitalizes.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void resetAtlas() {
    SoftReset();
}


/**********************************************************************
 * Function: initializeAtlas
 * @return None.
 * @remark Initializes the boat's master state machine and all modules.
 * @author David Goodman
 * @date 2013.04.24
 **********************************************************************/
static void initializeAtlas() {
    Board_init();
#if defined(DEBUG) && !defined(USE_XBEE)
    Serial_init();
    DBPRINT("Initializing serial.\n");
#endif
    Timer_init();

    // ----------------- Custom Hardware ------------------
    #ifdef USE_DRIVE
    DBPRINT("Initializing drive.\n");
    Drive_init();
    #endif

    #ifdef USE_OVERRIDE
    DBPRINT("Initializing override.\n");
    Override_init();
    #endif

    // -------------------- I2C Devices -------------------
    DBPRINT("Initializing I2C.\n");
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);
    if (I2C_hasError()) {
        fatal(ERROR_I2C);
    }

    #ifdef USE_TILTCOMPASS
    DBPRINT("Initializing tilt compass.\n");
    if (TiltCompass_init() != SUCCESS) {
        fatal(ERROR_TILTCOMPASS);
    }
    #endif


    #ifdef USE_BAROMETER
    DBPRINT("Initializing barometer.\n");
    if (Barometer_init() != SUCCESS) {
        fatal(ERROR_BAROMETER);
    }
    Timer_new(TIMER_BAROMETER_SEND, BAROMETER_SEND_DELAY);
    #endif


    haveXbee = FALSE;
    #ifdef USE_XBEE
    DBPRINT("Initializing xbee\n");
    if (Xbee_init(XBEE_UART_ID) != SUCCESS) {
        fatal(ERROR_XBEE);
    }
    haveXbee = TRUE;
    #endif


    // ------------------- UART Devices ------------------
    #ifdef USE_GPS
    DBPRINT("Initializing gps.\n");
    if (GPS_init() != SUCCESS) {
        fatal(ERROR_GPS);
    }
    #endif

    #ifdef USE_NAVIGATION
    DBPRINT("Initializing navigation.\n");
    if (Navigation_init() != SUCCESS) {
        fatal(ERROR_NAVIGATION);
    }
    #endif



    Timer_new(TIMER_HEARTBEAT, HEARTBEAT_SEND_DELAY);
    Mavlink_sendStatus(MAVLINK_STATUS_ONLINE);
    
    DELAY(5);
    // Start calibrating before use
    startSetOriginSM();
}

//---------------------------------MAIN -------------------------------

/**********************************************************************
 * Function: main
 * @return SUCCESS or FAILURE/
 * @remark Entry point for AtLAs (boat).
 * @author David Goodman
 * @date 2013.03.10  
 **********************************************************************/
#define USE_MAIN
#ifdef USE_MAIN
int main(void) {
    initializeAtlas();
    while(1){
        doMasterSM();
    }
    return (SUCCESS);
}
#endif

