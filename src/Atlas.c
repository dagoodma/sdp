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
#define DEBUG
//#define DEBUG_VERBOSE

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>

#include "Board.h"
#include "Serial.h"
#include "Ports.h"
#include "Magnetometer.h"
#include "Gps.h"
#include "Navigation.h"
#include "Drive.h"
#include "Mavlink.h"
#include 

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// Module selection (comment a line out to disable the module)
#define USE_OVERRIDE
#define USE_NAVIGATION
#define USE_GPS
#define USE_DRIVE
#define USE_TILTCOMPASS

#define EVENT_BYTE_SIZE     10 // provides 80 event bits

// Timer delays
#define INITIALIZE_TIMEOUT_DELAY        5000 // (ms) time before restarting

// Maximum substate errors before failing (fall into override)
#define INITIALIZE_ERROR_MAX            5

#define STATION_TOLERANCE_MIN           5.0f // (meters) to approach station
#define STATION_TOLERANCE_MAX           25.0f // (meters) distance to float away
#define RESCUE_TOLERANCE                2.0f // (meters) to approach person

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// ---- States for Master SM ----
static enum {
    STATE_INITIALIZE,   // Initializing and obtaining origin and station
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

    /* - Station Keep SM - */
    STATE_STATIONKEEP_RETURN,
    STATE_STATIONKEEP_IDLE,

    /* - Initialize SM - */
    STATE_INITIALIZE_ORIGIN, 
    STATE_INITIALIZE_STATION

} subState;


union EVENTS
    struct {
        /* - Mavlink message flags - */
        unsigned int haveReturnStationMessage :1;
        unsigned int haveReinitializeMessage :1;
        unsigned int haveOverrideMessage :1;
        unsigned int haveOriginMessage :1;
        unsigned int haveGeocentricErrorMessage :1;
        unsigned int haveSetStationMessage :1;
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
        /*  - Initialization events - */
        unsigned int initializeDone :1;
        unsigned int initializeFail :1; // timed out too many times
    } flags;
    unsigned char bytes[EVENT_BYTE_SIZE]; // allows for 80 bitfields
} event;


bool overrideShutdown = FALSE; //  whether to force override
bool isInitialized = FALSE;

LocalCoordinate nedStation; // NED coordinate with station location
LocalCoordinate nedRescue; // NED coordinate of drowning person

int lastMavlinkMessageID; // ID of most recently received Mavlink message

int errorCount = 0; // Number of errors that have occurred in current state


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
void checkEvents();
void doInitializeSM();
void doStationKeepSM();
void doOverrideSM();
void doRescueSM();
void doMasterSM();
void startInitializeSM();
void startStationKeepSM();
void startOverrideSM();
void startRescueSM();
void initializeAtlas();

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
    for (i=0; i < EVENT_BYTE_SIZE; i++)
        event.bytes[i] = 0x0;       


    // Navigation events
    if (Navigation_isDone())
        event.flags.navigationDone = TRUE;

    if (Navigation_hasError())
        event.flags.navigationError = TRUE;

    // Override
    if (Override_isTriggered())
        event.flags.overrideTriggered = TRUE;


    // XBee messages (from command center)
    if (Mavlink_hasNewMessage()) {
        lastMessageID = Mavlink_getNewMessageID();
        switch (lastMessageID) {
            case MAVLINK_MSG_ID_RESET:
                if (newMessage.commandOther->status == MAVLINK_RETURN_STATION) {
                    event.flags.haveReturnStationMessage = TRUE;
                    overrideShutdown = FALSE;
                }
                else if (newMessage.commandOther->status == MAVLINK_REINITIALIZE) {
                    event.flags.haveReinitializeMessage = TRUE;
                }
                else if (newMessage.commandOther->status == MAVLINK_OVERRIDE) {
                    event.flags.haveOverrideMessage = TRUE;
                    overrideShutdown = TRUE;
                }
                break;
            case MAVLINK_MSG_ID_GPS_ECEF:
                if (newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ORIGIN)
                    event.flags.haveOriginMessage = TRUE;
                else if (newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ERROR)
                    event.flags.haveGeocentricErrorMessage = TRUE;
                break;
            case MAVLINK_MSG_ID_GPS_NED:
                if (newMessage.gpsLocalData.status == MAVLINK_LOCAL_SET_STATION)
                    event.flags.haveSetStationMessage = TRUE;
                else if (newMessage.gpsLocalData.status == MAVLINK_LOCAL_START_RESCUE)
                    event.flags.haveStartRescueMessage = TRUE;
                break;
            default:
                // Unknown message ID
                event.flags.haveUnknownMessage = TRUE;
                DBPRINTF("Mavlink received unhandled message: 0x%X\n");
                break;
        }
    }
} //  checkEvents()


/**********************************************************************
 * Function: doInitializeSM
 * @return None
 * @remark Steps into the initialize state machine.
 **********************************************************************/
void doInitializeSM() {
    switch (subState) {
        case STATE_INITIALIZE_ORIGIN:
            // Waiting for origin message from command center 
            if (event.flags.haveOriginMessage) {
                LocalCoordinate nedOrigin;
                nedOrigin.north = Mavlink_newMessage.gpsLocalData->north;
                nedOrigin.east = Mavlink_newMessage.gpsLocalData->north;
                nedOrigin.down = Mavlink_newMessage.gpsLocalData->north;
                Navigation_setOrigin(&nedOrigin);

                if (Mavlink_newMessage.gpsLocalData->ack == WANT_ACK)
                    Mavlink_sendAck(lastMessageID,
                        Mavlink_newMessage.gpsLocalData->status);

                DBPRINTF("Received origin message.\n");

                Timer_new(TIMER_MAIN, INITIALIZE_TIMEOUT_DELAY);
                subState = STATE_INITIALIZE_STATION;
            }
            break;
        case STATE_INITIALIZE_STATION:
            // Wait
            if (event.flags.haveStationMessage) {
                nedStation.north = Mavlink_newMessage.gpsLocalData->north;
                nedStation.east = Mavlink_newMessage.gpsLocalData->east;
                nedStation.down = Mavlink_newMessage.gpsLocalData->down;

                if (Mavlink_newMessage.gpsLocalData->ack == WANT_ACK)
                    Mavlink_sendAck(lastMessageID,
                        Mavlink_newMessage.gpsLocalData->status);

                DBPRINTF("Received station message.\n");

                // Exit the state machine
                event.flags.initializeDone = TRUE;
                return;
            }
            break;

    }
    // Check if we timed out waiting for an initialization message
    if (Timer_isExpired(TIMER_MAIN)) {
        errorCount++;
        Mavlink_sendInitializeTimeoutError();
        DBPRINTF("Error: %s\n", ERROR_MESSAGE[ERROR_INITIALIZE_TIMEDOUT]);
        if (errorCount < INITIALIZE_ERROR_MAX)
            startInitializeSM();
        else
            event.flags.intializeFail = TRUE; // exit into override state
    }
}


/**********************************************************************
 * Function: doStationKeepSM
 * @return None
 * @remark Steps into the station keeping state machine.
 **********************************************************************/
void doStationKeepSM() {
    switch (subState) {
        case STATE_STATIONKEEP_RETURN:
            // Driving to the station
            if (event.flags.navigationDone) {
                subState = STATE_STATIONKEEP_IDLE;
                Timer_new(TIMER_MAIN,STATION_KEEP_DELAY);
            }
            // TODO obstacle detection
            break;
        case STATE_STATIONKEEP_IDLE:
            // Wait to float away from the station
            if (Timer_isExpired(TIMER_MAIN)) {
                // Check if we floated too far away from the station
                if (Navigation_getLocalDistance(&nedStation) > STATION_TOLERANCE_MAX) {
                    startStationKeepSM(); // return to station
                    return;
                }

                Timer_new(TIMER_MAIN, STATION_KEEP_DELAY);
            }
            break;
    }
    if (event.flags.navigationFail) {
        error_t errorCode = Navigation_getError();
        Mavlink_sendError(errorCode);
        DBPRINTF("Error: %s\n", ERROR_MESSAGE[errorCode]);
        event.flags.stationKeepFail = TRUE;
    }
}


/**********************************************************************
 * Function: doOverrideSM
 * @return None
 * @remark Executes one cycle of the boat's override state machine.
 **********************************************************************/
void doOverrideSM() {
    // Do nothing
}


/**********************************************************************
 * Function: doRescueSM
 * @return None
 * @remark Executes one cycle of the boat's rescue state machine.
 **********************************************************************/
void doRescueSM() {
    switch (subState) {
        case STATE_RESCUE_GOTO:
            if (event.flags.navigationFail) {
                error_t errorCode = Navigation_getError();
                Mavlink_sendError(errorCode);
                DBPRINTF("Error: %s\n", ERROR_MESSAGE[errorCode]);
                event.flags.rescueFail = TRUE;
            }
            if (event.flags.navigationDone) {
                subState = STATE_RESCUE_SEARCH;
                Navigation_cancel();
            }
            break;
        case STATE_RESCUE_SEARCH:
            // Human sensor event handling
            // Falls through to success for now
            subState = STATE_RESCUE_SUPPORT;
            DBPRINTF("Rescue mission complete.\n");
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
void doMasterSM() {
    checkEvents();

    // I2C bus
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);

    #ifdef USE_TILTCOMPASS
    TiltCompass_runSM();
    #endif

    #ifdef USE_GPS
    GPS_runSM();
    #endif

    #ifdef USE_NAVIGATION
    Navigation_runSM();
    #endif

    #ifdef USE_DRIVE
    Drive_runSM();
    #endif

    #ifdef USE_XBEE
    Xbee_runSM();
    #endif

    switch (state) {
        case STATE_INITIALIZE:
            doInitializeSM();
            
            // Transition out when finished or failure occurs
            if (event.flags.initializeDone) {
                isInitialized = TRUE;
                startStationKeepSM();
            }
            if (event.flags.initializeFail) {
                forceOverride = TRUE;
                startOverrideSM();
            }

            break;
        case STATE_STATIONKEEP:
            doStationKeepSM();

            if (event.flags.haveRescueMessage)
                startRescueSM();
            if (event.flags.stationKeepFail)
                forceOverride = TRUE;
                startOverrideSM();
                start


            break;

        case STATE_OVERRIDE:
            doOverrideSM();
    
            if (!event.flags.overrideTriggered && !event.flags.haveOverrideMessage) {
                Override_giveMicroControl();
                DBPRINTF("Micro has control.\n");
                startStationKeepSM();
            }

            break;

        case STATE_RESCUE:
            doRescueSM();

            if (event.flags.haveReturnStationMessage) {
                if (isInitialized)
                    startStationKeepSM();
                else 
                    startInitializeSM();
            }
            if (event.flags.rescueFail) {
                forceOverride = TRUE;
                startOverrideSM();
            }

            break;
    }
    //  ------- Caught by all states ----------
    if (event.flags.overrideTriggered || event.flags.haveOverrideMessage)
        startOverrideSM();
    if (event.flags.haveReinitializeMessage)
        startInitializeSM();
}


//------------------------ Start States -------------------------------


/**********************************************************************
 * Function: startInitializeSM
 * @return None.
 * @remark Transitions into the intialize state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
void startInitializeSM() {
    state = STATE_INITIALIZE;
    subState = STATE_INITIALIZE_ORIGIN;
    isInitialized = FALSE;

    // Send status message to command center to initiate initialization
    Mavlink_sendStatus(MAVLINK_STATUS_START_INITIALIZE);

    errorCount = 0;
    Timer_new(TIMER_MAIN, INITIALIZE_TIMEOUT_DELAY);
}

/**********************************************************************
 * Function: startStationKeepSM
 * @return None.
 * @remark Transitions into the station keeping state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
void startStationKeepSM() {
    state = STATE_STATIONKEEP;
    subState = STATE_STATIONKEEP_RETURN;

    // Send status message for debugging
    Mavlink_sendStatus(MAVLINK_STATUS_RETURN_STATION);

    Navigation_gotoLocalCoordinate(&nedStation, STATION_TOLERANCE_MIN);
}

/**********************************************************************
 * Function: startOverrideSM
 * @return None.
 * @remark Transitions into the override state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
void startOverrideSM() {
    state = STATE_OVERRIDE;
    subState = STATE_NONE;

    Override_giveReceiverControl();
    DBPRINT("Reciever has control.\n");
    Navigation_cancel();
}

/**********************************************************************
 * Function: startRescueSM
 * @return None.
 * @remark Transitions into the rescue state machine.
 * @author David Goodman
 * @date 2013.04.26  
 **********************************************************************/
void startRescueSM() {
    state = STATE_RESCUE;
    subState = STATE_RESCUE_GOTO;

    DBPRINTF("Start a rescue.\n");

    // Get location data from message
    nedRescue.north = Mavlink_newMessage.gpsLocalData->north;
    nedRescue.east = Mavlink_newMessage.gpsLocalData->east;
    nedRescue.south = Mavlink_newMessage.gpsLocalData->south;

    // Send status message for debugging
    Mavlink_sendStatus(MAVLINK_STATUS_START_RESCUE);

    Navigation_gotoLocalCoordinate(&nedStation, RESCUE_TOLERANCE);
}




/**********************************************************************
 * Function: initializeAtlas
 * @return None.
 * @remark Initializes the boat's master state machine and all modules.
 * @author David Goodman
 * @date 2013.04.24  
 **********************************************************************/
void initializeAtlas() {
    Board_init();
    Serial_init();
    Timer_init();

    #ifdef USE_DRIVE
    Drive_init();
    #endif

    // I2C bus
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);

    #ifdef USE_TILTCOMPASS
    TiltCompass_init();
    #endif

    #ifdef USE_GPS
    GPS_init();
    #endif

    #ifdef USE_NAVIGATION
    Navigation_init();
    #endif

    #ifdef USE_OVERRIDE
    Override_init();
    #endif
        
    // Start calibrating before use
    startInitializeState();
}


//---------------------------------MAIN -------------------------------

/**********************************************************************
 * Function: main
 * @return SUCCESS or FAILURE/
 * @remark Entry point for AtLAs (boat).
 * @author David Goodman
 * @date 2013.03.10  
 **********************************************************************/
#ifdef USE_MAIN
int main(void) {
    initMasterSM();
    printf("%.2f",0.00);
    printf("Atlas ready for use. \n\n");
    while(1){
        doMasterSM();
    }
    return (SUCCESS);
}
#endif


