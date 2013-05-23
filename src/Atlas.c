/**********************************************************************
 Module
 Atlas.c

 Author: David Goodman

 History
 When                   Who         What/Why
 --------------         ---         --------
3/27/2013   11:10PM     dagoodma    Created project.
***********************************************************************/


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
#define IS_ATLAS
//#define DEBUG
//#define DEBUG_VERBOSE

#define XBEE_UART_ID    UART1_ID
#define GPS_UART_ID     UART2_ID

// Module selection (comment a line out to disable the module)
#define USE_OVERRIDE
#define USE_NAVIGATION
#define USE_GPS
#define USE_DRIVE
#define USE_TILTCOMPASS
#define USE_XBEE
//#define USE_SIREN // NOT IMPLEMENTED
#define USE_BAROMETER
#define DO_HEARTBEAT

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
#define TIMER_SETORIGIN_RETRY   TIMER_MAIN2

#define TIMER_BAROMETER_SEND        TIMER_BACKGROUND
#define TIMER_GPS_CORRECTION_LOST   TIMER_BACKGROUND2
#define TIMER_HEARTBEAT             TIMER_BACKGROUND3

// --------------------- Timer delays -----------------------
#define STARTUP_DELAY               2500 // (ms) time to wait before starting up
#define STATION_KEEP_DELAY          10000 // (ms) to check if drifted away
#define BAROMETER_SEND_DELAY        10000 // (ms) time between sending barometer data
#define RESEND_MESSAGE_DELAY        4000 // (ms) resend a message
#define GPS_CORRECTION_LOST_DELAY   5000 // (ms) throw out gps error corrections now
#define HEARTBEAT_SEND_DELAY        3000 // (ms) between heart being sent to CC
#define DEBUG_PRINT_DELAY           1200
#define RETRY_ORIGIN_DELAY          3000

#define RESEND_MESSAGE_LIMIT        5 // times to resend before failing

// Maximum substate errors before failing (fall into override)
#define INITIALIZE_ERROR_MAX            5

#define STATION_TOLERANCE_MIN           5.0f // (meters) to approach station
#define STATION_TOLERANCE_MAX           8.0f // (meters) distance to float away
#define RESCUE_TOLERANCE                2.0f // (meters) to approach person

// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  75000 // (Hz)

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
I2C_MODULE I2C_BUS_ID   = I2C1;


// ---- States for Master SM ----
static enum {
    STATE_SETORIGIN = 0x1,    // Obtain location of command center.
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
        unsigned int rescueSuccess :1;
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
static bool receiverShutdown = FALSE; // whether receiver made us enter override or not
static bool haveStation = FALSE;
static bool haveOrigin = FALSE;
static bool wantSaveStation = FALSE;
//static bool wantOverride = FALSE;
static bool haveXbee = FALSE;
static bool haveError = FALSE;
static bool rescueShutdown = FALSE;

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
static void clearError();
static void gpsCorrectionUpdate();
static void doBarometerUpdate();
static void doHeartbeatMessage();
static void checkOverride();
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
                lastMavlinkMessageWantsAck = Mavlink_newMessage.commandOtherData.ack == WANT_ACK;
                lastMavlinkCommandID = Mavlink_newMessage.commandOtherData.command;
                if (Mavlink_newMessage.commandOtherData.command == MAVLINK_RESET_BOAT ) {
                    event.flags.haveResetMessage = TRUE;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_RETURN_STATION) {
                    event.flags.haveReturnStationMessage = TRUE;
                    overrideShutdown = FALSE;
                    rescueShutdown = FALSE;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_SAVE_STATION) {
                    event.flags.haveSetStationMessage = TRUE;
                    wantSaveStation = TRUE;
                    overrideShutdown = FALSE;
                    rescueShutdown = FALSE;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_GEOCENTRIC_ORIGIN) {
                    event.flags.haveSetOriginMessage = TRUE;
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_OVERRIDE) {
                    event.flags.haveOverrideMessage = TRUE;
                    overrideShutdown = TRUE;
                }
                break;
            case MAVLINK_MSG_ID_GPS_ECEF:
                lastMavlinkMessageWantsAck = (Mavlink_newMessage.gpsGeocentricData.ack == WANT_ACK);
                lastMavlinkCommandID = Mavlink_newMessage.commandOtherData.command;
                if (Mavlink_newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ORIGIN) {
                    event.flags.haveSetOriginMessage = TRUE;
                }
                else if (Mavlink_newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ERROR)
                    event.flags.haveGeocentricErrorMessage = TRUE;
                break;
            case MAVLINK_MSG_ID_GPS_NED:
                lastMavlinkMessageWantsAck = Mavlink_newMessage.gpsLocalData.ack == WANT_ACK;
                lastMavlinkCommandID = Mavlink_newMessage.gpsLocalData.status;
                if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_SET_STATION) {
                    event.flags.haveSetStationMessage = TRUE;
                    wantSaveStation = FALSE;
                    overrideShutdown = FALSE;
                    rescueShutdown = FALSE;
                }
                else if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_START_RESCUE) {
                    event.flags.haveStartRescueMessage = TRUE;
                    overrideShutdown = FALSE;
                    rescueShutdown = FALSE;
                }
                break;
            default:
                // Unknown message ID
                event.flags.haveUnknownMessage = TRUE;
                DBPRINT("Mavlink received unhandled message: 0x%X\n");
                break;
        }
    }
    if (event.flags.haveOverrideMessage || overrideShutdown || event.flags.receiverDetected
            || rescueShutdown)
        event.flags.wantOverride = TRUE;
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

        //handleAcknowledgement();
        Mavlink_sendAck(MAVLINK_MSG_ID_GPS_ECEF, MAVLINK_GEOCENTRIC_ORIGIN);
        haveOrigin = TRUE;
        event.flags.setOriginDone = TRUE;

        DBPRINT("Set new origin: X=%.1f, Y=%.1f, Z=%.1f\n",ecefOrigin.x, ecefOrigin.y, ecefOrigin.z);
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
                DBPRINT("Resending origin request.\n");
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
        #ifdef USE_NAVIGATION
        Navigation_getLocalPosition(&nedStation);

        if (Navigation_hasError()) {
            // Don't have GPS lock
            setError(Navigation_getError());
            return;
        }
        #endif

        event.flags.setStationDone = TRUE;
        haveStation = TRUE;
        wantSaveStation = FALSE;

        DBPRINT("Saved new station: N=%.2f, E=%.2f, D=%.2f\n", nedStation.north, nedStation.east, nedStation.down);
     }
     else {
         // Set the given coordinate as station
        nedStation.north = Mavlink_newMessage.gpsLocalData.north;
        nedStation.east = Mavlink_newMessage.gpsLocalData.east;
        nedStation.down = Mavlink_newMessage.gpsLocalData.down;

        haveStation = TRUE;
        event.flags.setStationDone = TRUE;
        
        DBPRINT("Set new station: N=%.2f, E=%.2f, D=%.2f.\n",nedStation.north, nedStation.east, nedStation.down);
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
            #ifdef USE_NAVIGATION
            if (event.flags.navigationDone) {
                subState = STATE_STATIONKEEP_IDLE;
                Timer_new(TIMER_STATIONKEEP,STATION_KEEP_DELAY); // check position on timer
                Mavlink_sendStatus(MAVLINK_STATUS_ARRIVED_STATION);
                DBPRINT("Arrived at station.\n");
            }
            #else
                subState = STATE_STATIONKEEP_IDLE;
                Timer_new(TIMER_STATIONKEEP,STATION_KEEP_DELAY); // check position on timer
                Mavlink_sendStatus(MAVLINK_STATUS_ARRIVED_STATION);
                DBPRINT("Arrived at station.\n");
            #endif
            // TODO obstacle detection
            break;
        case STATE_STATIONKEEP_IDLE:
            // Wait to float away from the station
            if (Timer_isExpired(TIMER_STATIONKEEP)) {
                // Check if we floated too far away from the station
                #ifdef USE_NAVIGATION
                if (Navigation_getLocalDistance(&nedStation) > STATION_TOLERANCE_MAX) {
                    startStationKeepSM(); // return to station
                    return;
                }
                else {
                    Timer_new(TIMER_STATIONKEEP, STATION_KEEP_DELAY);
                }
                #else
                    startStationKeepSM(); // return to station
                    return;
                #endif
            }
            break;
    }

    #ifdef USE_NAVIGATION
    if (event.flags.navigationError) {
        setError((error_t)Navigation_getError());
    }
    #endif
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
            #ifdef USE_NAVIGATION
            if (event.flags.navigationDone) {
                subState = STATE_RESCUE_SEARCH;
                Navigation_cancel();
            }
            #else
                subState = STATE_RESCUE_SEARCH;
                DBPRINT("Arrived near person, searching.\n");
            #endif
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
            event.flags.rescueSuccess = TRUE;

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

    //checkOverride();

    #ifdef DEBUG_VERBOSE
    if (Timer_isExpired(TIMER_TEST2)) {
        DBPRINT("State=%X,%X, Receiver=%X, WantOver=%X, ForceOver=%X, ReceiverShut=%X, HaveError=%X\n",
            state, subState, event.flags.receiverDetected, event.flags.wantOverride, overrideShutdown, receiverShutdown, haveError);
        Timer_new(TIMER_TEST2, DEBUG_PRINT_DELAY);
    }
    #endif

    switch (state) {
        case STATE_SETSTATION:
            doSetStationSM();

            if (event.flags.haveStartRescueMessage) {
                startRescueSM();
            }
            else if (event.flags.setStationDone) {
                startStationKeepSM();
            }
            
            break;
        case STATE_SETORIGIN:
            doSetOriginSM();
            
            if (event.flags.setOriginDone)
                startOverrideSM();    // wait for set station
            
            break;
        case STATE_STATIONKEEP:
            doStationKeepSM();

            if (event.flags.haveStartRescueMessage)
                startRescueSM();
            else if (!haveStation)
                setError(ERROR_NO_STATION);
            break;

        case STATE_OVERRIDE:
            if (!receiverShutdown && event.flags.receiverDetected)
                startOverrideSM(); // receiver came online
            else if (event.flags.haveOverrideMessage) {
                // Already stopped
                handleAcknowledgement();
                Mavlink_sendStatus(STOPPED_BOAT_MESSAGE);
            }
            /*
            else if (event.flags.haveSetStationMessage) {
                if (haveOrigin)
                    startSetStationSM();
                else {
                    handleAcknowledgement();
                    Mavlink_sendError(ERROR_NO_ORIGIN);
                }
            }
             * */
            else if (!event.flags.wantOverride) {
                if (event.flags.haveStartRescueMessage) {
                    if (haveOrigin)
                        startRescueSM();
                    else {
                        handleAcknowledgement();
                        Mavlink_sendError(ERROR_NO_ORIGIN);
                        Timer_new(TIMER_SETORIGIN_RETRY, RETRY_ORIGIN_DELAY);
                    }
                }
                else if (event.flags.haveSetStationMessage) {
                    startSetStationSM();
                }
                else if (event.flags.haveReturnStationMessage || !haveError) {
                    // Fall Through state
                    if (haveOrigin && haveStation)
                        startStationKeepSM();
                    else if (event.flags.haveReturnStationMessage) {
                        handleAcknowledgement();
                        if (!haveOrigin) {
                            Mavlink_sendError(ERROR_NO_ORIGIN);
                            Timer_new(TIMER_SETORIGIN_RETRY, RETRY_ORIGIN_DELAY);
                        }
                        else if (!haveStation)
                            Mavlink_sendError(ERROR_NO_STATION);
                    }
                    else {
                        // Do nothing, waiting for setStation msg
                    }
                }
                else if (!haveOrigin)
                    startSetOriginSM(); // other fall through
                
                if (Timer_isExpired(TIMER_SETORIGIN_RETRY)) {
                    startSetOriginSM(); // retry origin
                }
            }
            else {
                if (event.flags.haveStartRescueMessage) {
                    handleAcknowledgement();
                    Mavlink_sendError(ERROR_OVERRIDE);
                }
                else if (event.flags.haveReturnStationMessage) {
                    handleAcknowledgement();
                    Mavlink_sendError(ERROR_OVERRIDE);
                }
                else if (event.flags.haveSetStationMessage) {
                    handleAcknowledgement();
                    Mavlink_sendError(ERROR_OVERRIDE);
                }
            }

            if (receiverShutdown && !event.flags.receiverDetected)
                receiverShutdown = FALSE;
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
            if (event.flags.rescueSuccess) {
                // Just stop in place
                rescueShutdown = TRUE;
                startOverrideSM();
            }

            break;
    }
    //  ------- Caught by most states -----------
    if (state != STATE_RESCUE && state != STATE_SETSTATION) {
        if (event.flags.haveSetStationMessage && !event.flags.wantOverride && !haveError)
            startSetStationSM();
    }
    if (state != STATE_OVERRIDE) {
        /*
        if (event.flags.haveError) {
            DBPRINT("Error: %s", getErrorMessage(lastErrorCode));
            //Mavlink_sendError(lastErrorCode); //sent in setError
            startOverrideSM();
        }*/
        if (event.flags.wantOverride)
            startOverrideSM();
        else if (!haveOrigin && state != STATE_SETORIGIN) {
            Mavlink_sendError(ERROR_NO_ORIGIN);
            startOverrideSM();
        }
        else if (!haveStation && state != STATE_SETSTATION && state != STATE_SETORIGIN
                && state != STATE_RESCUE) {
            Mavlink_sendError(ERROR_NO_STATION);
            startOverrideSM();
        }
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
    clearError();

    // Send status message to command center to request origin coordinate
    Mavlink_sendRequestOrigin();

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
    clearError();

    // Handle acknowledgement works (I think) but saw issues with handleAck...
    //if (event.flags.haveSetStationMessage)
    //    handleAcknowledgement();
    Mavlink_sendAck(MAVLINK_MSG_ID_CMD_OTHER, MAVLINK_SAVE_STATION);

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
    clearError();
    
    if (event.flags.haveReturnStationMessage)
        handleAcknowledgement();

    // Send status message for debugging
    Mavlink_sendStatus(MAVLINK_STATUS_RETURN_STATION);

    #ifdef USE_NAVIGATION
    Navigation_gotoLocalCoordinate(&nedStation, STATION_TOLERANCE_MIN); // to station
    #endif

    Override_giveMicroControl();
    DBPRINT("Micro has control.\n");

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

    if (event.flags.receiverDetected) {
        Mavlink_sendStatus(MAVLINK_STATUS_OVERRIDE);
        receiverShutdown = TRUE;
    }
    else
        receiverShutdown = FALSE;

    if (event.flags.haveOverrideMessage)
        handleAcknowledgement();
    

    //if (event.flags.haveOverrideMessage)
    //    Mavlink_sendAck(MAVLINK_MSG_ID_CMD_OTHER, MAVLINK_OVERRIDE);

    Override_giveReceiverControl();
    DBPRINT("Reciever has control.\n");
    Navigation_cancel() ;

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
    clearError();

    if (event.flags.haveStartRescueMessage)
        handleAcknowledgement();

    // Get location data from message
    nedRescue.north = Mavlink_newMessage.gpsLocalData.north;
    nedRescue.east = Mavlink_newMessage.gpsLocalData.east;
    nedRescue.down = Mavlink_newMessage.gpsLocalData.down;

    // Send status message for debugging
    #ifdef USE_NAVIGATION
    Navigation_gotoLocalCoordinate(&nedStation, RESCUE_TOLERANCE); // to rescue
    #endif

    if (!Navigation_hasError())
        Mavlink_sendStatus(MAVLINK_STATUS_START_RESCUE);
    else {
        setError(Navigation_getError());
        return;
    }

    #ifdef USE_SIREN
    Siren_redLightOn();
    #endif

    Override_giveMicroControl();
    DBPRINT("Micro has control.\n");

    DBPRINT("Rescuing person at: N=%.2f, E=%.2f, D=%.2f.\n",nedRescue.north, nedRescue.east, nedRescue.down);
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

    lastMavlinkMessageWantsAck = FALSE;
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
    /* TODO uncomment this
     if (errorCode ==  ERROR_NONE)
     *      return;
     */
    lastErrorCode = errorCode;
    event.flags.haveError = TRUE;
    haveError = TRUE;
    Mavlink_sendError(errorCode);
    DBPRINT("Error: %s\n", getErrorMessage(errorCode));
    //DELAY(20);
    startOverrideSM();
}
/**********************************************************************
 * Function: clearError
 * @return None.
 * @remark Clears any set error codes.
 * @author David Goodman
 * @date 2013.05.19
 **********************************************************************/
static void clearError() {
    lastErrorCode = ERROR_NONE;
    event.flags.haveError = FALSE;
    haveError = FALSE;
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

        #ifdef USE_XBEE
        Mavlink_sendBarometerData(Barometer_getTemperature(),
            Barometer_getAltitude());
        #endif

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
    if (Timer_isExpired(TIMER_HEARTBEAT) || !Timer_isActive(TIMER_HEARTBEAT)) {

        #ifdef USE_XBEE
        Mavlink_sendHeartbeat();
        #endif

        Timer_new(TIMER_HEARTBEAT, HEARTBEAT_SEND_DELAY);
    }
}

/**********************************************************************
 * Function: checkOverride
 * @return None.
 * @remark Check if an override is desired.
 * @author David Goodman
 * @date 2013.05.04
 **********************************************************************/
static void checkOverride() {
    // TODO remove, this was moved into override state logic and into
    /* If the receiver is off, boat is not stopped, and we either
        have a station or are setting one, then turn override off */
    /*
    if (!event.flags.receiverDetected && !overrideShutdown
        && ((haveStation && haveOrigin) || (state == STATE_SETSTATION)
            || state == STATE_SETORIGIN || (haveOrigin && state == STATE_RESCUE))) {
        wantOverride = FALSE;
    }*//*
    if (event.flags.receiverDetected || overrideShutdown
            || (!haveOrigin && state != STATE_SETORIGIN)
            || (!haveStation && (state != STATE_SETORIGIN && state != STATE_SETSTATION)) ) {
        wantOverride = TRUE;
    }
    else {
        wantOverride = FALSE;
    }*/
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
#if defined(DEBUG) && defined(USE_SERIAL) //&& !defined(USE_GPS)
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


    // ------------------- UART Devices ------------------

    haveXbee = FALSE;
    #ifdef USE_XBEE
    DBPRINT("Initializing xbee\n");
    if (Xbee_init(XBEE_UART_ID) != SUCCESS) {
        fatal(ERROR_XBEE);
    }
    haveXbee = TRUE;
    #endif


    #ifdef USE_GPS
    DBPRINT("Initializing gps.\n");
    if (GPS_init(GPS_UART_ID) != SUCCESS) {
        fatal(ERROR_GPS);
    }
    #else
    DBPRINT("Skipping gps.\n");
    #endif

    #ifdef USE_NAVIGATION
    DBPRINT("Initializing navigation.\n");
    if (Navigation_init() != SUCCESS) {
        fatal(ERROR_NAVIGATION);
    }
    #endif

    #ifdef DEBUG_VERBOSE
    Timer_new(TIMER_TEST2, DEBUG_PRINT_DELAY);
    #endif


    Timer_new(TIMER_HEARTBEAT, HEARTBEAT_SEND_DELAY);
    Mavlink_sendStatus(MAVLINK_STATUS_ONLINE);
    
    startSetOriginSM();

/*
    while (1) {
        asm("nop");
    }*/
}

//---------------------------------MAIN -------------------------------

/**********************************************************************
 * Function: main
 * @return SUCCESS or FAILURE
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



//#define ATLAS_SYSTEM_TEST
#ifdef ATLAS_SYSTEM_TEST

#define PRINT_DELAY     1500

// Be sure to disable Xbee and enable Debug

static void printGps();
static void printBarometer();
static void printTiltCompass();


int main() {
    initializeAtlas();

    DBPRINT("AtLAs system test initialized.\n");
    // Print GPS and sensor data while sending XBee messages
    Timer_new(TIMER_TEST,PRINT_DELAY);
    while (1) {
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

        //checkOverride();

        #ifdef DEBUG_VERBOSE
        if (Timer_isExpired(TIMER_TEST2)) {
            DBPRINT("State=%X,%X, Receiver=%X, WantOver=%X, ForceOver=%X, ReceiverShut=%X, HaveError=%X\n",
                state, subState, event.flags.receiverDetected, event.flags.wantOverride, overrideShutdown, receiverShutdown, haveError);
            Timer_new(TIMER_TEST2, DEBUG_PRINT_DELAY);
        }
        #endif

        if (Timer_isExpired(TIMER_TEST)) {
            printGps();
            printBarometer();
            printTiltCompass();

            if (event.flags.haveError)
                printf("Found an error: %s\n", getErrorMessage(lastErrorCode));

            Timer_new(TIMER_TEST,PRINT_DELAY);
        }
    }


}

static void printBarometer() {
    printf("Barometer: T=%.2f, Alt=%.2f\n", Barometer_getTemperature(), Barometer_getAltitude());
}


static void printTiltCompass() {
    printf("Compass: %.2f\n", TiltCompass_getHeading());
}

static void printGps() {
    if (!GPS_isConnected()) {
        printf("GPS not connected.");
    }
    else if (GPS_hasFix() && GPS_hasPosition()) {
        GeocentricCoordinate ecefPos;
        GPS_getPosition(&ecefPos);
        printf("Position: x=%.2f, y=%.2f, z=%.2f (m)\n", ecefPos.x,
            ecefPos.y, ecefPos.z);

        printf("Velocity: %.2f (m/s), Heading: %.2f (deg)",
            GPS_getVelocity(), GPS_getHeading());
    }
    else {
        printf("No fix!");
    }

    if (Navigation_isReady())
        printf(" Nav ready.\n");
    else
        printf( "Nav not ready!\n");
}

#endif
