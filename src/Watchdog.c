/* 
 * File:   Watchdog.c
 * Author: dagoodma
 *
 * Created on May 11, 2013, 5:13 PM
 */
#define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Board.h"
#include "Mavlink.h"
#include "Xbee.h"
#include "LCD.h"
//#include "Logger.h"
#include "Timer.h"
#include "Uart.h"
#include "Error.h"
#include "Serial.h"
#include "Uart.h"
#include "Interface.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define USE_XBEE

#define XBEE_UART_ID        UART2_ID

#define SHOW_HEARTBEAT

#define EVENT_BYTE_SIZE     10 // provides 80 event bits

/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static void Watchdog_init();
static void doWatchdog();


/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/

static error_t lastBoatError;
static int lastMavlinkMessageID; // ID of most recently received Mavlink message
static int lastMavlinkCommandID; // Command code of last message (for ACK)
static char lastMavlinkMessageWantsAck;
static uint8_t resendMessageCount;

static error_t lastErrorCode = ERROR_NONE;
static error_t lastBoatErrorCode = ERROR_NONE;


// ---------------- Event flags  ------------------------
static union EVENTS {
    struct {

        /*******************************************************************
         * Messages from ComPAS to AtLAs (from Atlas.c)
         *******************************************************************/
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

        /*******************************************************************
         * Messages from AtLAs to ComPAS (from Compas.c)
         *******************************************************************/
        unsigned int haveReturnStationAck :1;
        unsigned int haveStopAck :1;
        unsigned int haveSetStationAck :1;
        unsigned int haveStartRescueAck :1;
        unsigned int haveSetOriginAck :1;
        unsigned int haveRequestOriginMessage :1;
        unsigned int haveBoatPositionMessage :1;
        unsigned int haveBoatOnlineMessage :1;

    } flags;
    unsigned char bytes[EVENT_BYTE_SIZE]; // allows for 80 bitfields
} event;

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


// ---------------- Interface Messages ------------------------
// (See Interface.c and Interface.h)
// -------------------------------- Read Mavlink ---------------------------

static void doWatchdog(void) {
    // Clear all event flags
    int i;

    for (i=0; i < EVENT_BYTE_SIZE; i++)
        event.bytes[i] = 0x0;

    if (Mavlink_hasNewMessage()) {
        lastMavlinkMessageID = Mavlink_getNewMessageID();
        lastMavlinkCommandID = MAVLINK_NO_COMMAND;
        lastMavlinkMessageWantsAck = FALSE;
        switch (lastMavlinkMessageID) {
            /*----------------  Heartbeat message -------------------------*/
            case MAVLINK_MSG_ID_XBEE_HEARTBEAT:
                #ifdef SHOW_HEARTBEAT
                dbprint("A: heartbeat!\n");
                #endif
                DBPRINT("HERE!\n");
                break;
            /*--------------------  Acknowledgement messages ------------------ */
            case MAVLINK_MSG_ID_MAVLINK_ACK:
                // ----  Messages from AtLAs to ComPAS (from Compas.c) --------
		// CMD_OTHER
                if (Mavlink_newMessage.ackData.msgID == MAVLINK_MSG_ID_CMD_OTHER) {
                    // Responding to a return to station request
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_RETURN_STATION) {
                        event.flags.haveReturnStationAck = TRUE;
                        dbprint("A: ack %s\n", getMessage(RETURNING_MESSAGE));
                    }
                    // Boat is in override state
                    else if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_OVERRIDE) {
                        event.flags.haveStopAck = TRUE;
                        dbprint("A: ack %s\n", getMessage(STOPPING_BOAT_MESSAGE));
                    }
                    // Boat saved station
                    else if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_SAVE_STATION) {
                        event.flags.haveSetStationAck = TRUE;
                        dbprint("A: ack %s\n", getMessage(SAVING_STATION_MESSAGE));
                    }
                }
                // GPS_NED
                else if (Mavlink_newMessage.ackData.msgID == MAVLINK_MSG_ID_GPS_NED) {
                    // Responding to a rescue
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_LOCAL_START_RESCUE) {
                        event.flags.haveStartRescueAck = TRUE;
                        dbprint("A: ack %s\n", getMessage(STARTING_RESCUE_MESSAGE));
                    }
                    else if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_LOCAL_SET_STATION) {
                        event.flags.haveSetStationAck = TRUE;
                        dbprint("A: ack %s\n", getMessage(SET_STATION_MESSAGE));
                    }
                }
                // GPS_ECEF
                else if (Mavlink_newMessage.ackData.msgID == MAVLINK_MSG_ID_GPS_ECEF) {
                    if (Mavlink_newMessage.ackData.msgStatus == MAVLINK_GEOCENTRIC_ORIGIN) {
                        event.flags.haveSetOriginAck = TRUE;
                        dbprint("A: ack=%s\n", getMessage(SET_ORIGIN_MESSAGE));
                    }
                }
            // --- Messages from ComPAS to AtLAs (from Atlas.c) ---
            // No ACKS from ComPAS to AtLAs
                break;
            /*------------------ CMD_OTHER messages --------------- */
            case MAVLINK_MSG_ID_CMD_OTHER:
                // --- Messages from ComPAS to AtLAs (from Atlas.c) ---
                lastMavlinkMessageWantsAck = Mavlink_newMessage.commandOtherData.ack;
                if (Mavlink_newMessage.commandOtherData.command == MAVLINK_RESET_BOAT ) {
                    event.flags.haveResetMessage = TRUE;
                    dbprint("C: %s\n",getMessage(RESET_BOAT_MESSAGE));
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_RETURN_STATION) {
                    event.flags.haveReturnStationMessage = TRUE;
                    dbprint("C: %s\n",getMessage(START_RETURN_MESSAGE));
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_SAVE_STATION) {
                    event.flags.haveSetStationMessage = TRUE;
                    dbprint("C: %s\n",getMessage(SET_STATION_MESSAGE));
                }
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_OVERRIDE) {
                    event.flags.haveOverrideMessage = TRUE;
                    dbprint("C: %s\n",getMessage(STOPPING_BOAT_MESSAGE));
                }
                // ----  Messages from AtLAs to ComPAS (from Compas.c) --------
                else if (Mavlink_newMessage.commandOtherData.command == MAVLINK_REQUEST_ORIGIN) {
                    event.flags.haveRequestOriginMessage = TRUE;
                    dbprint("A: requesting origin.\n");
                }
                break;
            /*--------------------  GPS messages ------------------ */
            case MAVLINK_MSG_ID_GPS_ECEF:
                // --- Messages from ComPAS to AtLAs (from Atlas.c) ---
                lastMavlinkMessageWantsAck = Mavlink_newMessage.gpsGeocentricData.ack;
                if (Mavlink_newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ORIGIN) {
                    event.flags.haveSetOriginMessage = TRUE;
                    dbprint("C: Sending boat origin.\n");
                    dbprint("   X=%.0f, Y=%.0f, Z=%.0f\n", Mavlink_newMessage.gpsGeocentricData.x,
                     Mavlink_newMessage.gpsGeocentricData.y, Mavlink_newMessage.gpsGeocentricData.z );
                }
                else if (Mavlink_newMessage.gpsGeocentricData.status == MAVLINK_GEOCENTRIC_ERROR) {
                    event.flags.haveGeocentricErrorMessage = TRUE;
                    dbprint("C: gps err X=%.0f, Y=%.0f, Z=%.0f\n", Mavlink_newMessage.gpsGeocentricData.x,
                     Mavlink_newMessage.gpsGeocentricData.y, Mavlink_newMessage.gpsGeocentricData.z );
                }
                // ----  Messages from AtLAs to ComPAS (from Compas.c) --------
                break;
            case MAVLINK_MSG_ID_GPS_NED:
                // --- Messages from ComPAS to AtLAs (from Atlas.c) ---
                lastMavlinkMessageWantsAck = Mavlink_newMessage.gpsLocalData.ack;
                if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_SET_STATION) {
                    event.flags.haveSetStationMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_LOCAL_SET_STATION;
                    dbprint("C: %s\n", getMessage(SET_STATION_MESSAGE));
                    dbprint(" N=%.1f, E=%.1f\n", Mavlink_newMessage.gpsLocalData.north,
                     Mavlink_newMessage.gpsLocalData.east);
                }
                else if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_START_RESCUE) {
                    event.flags.haveStartRescueMessage = TRUE;
                    lastMavlinkCommandID = MAVLINK_LOCAL_START_RESCUE;
                    dbprint("C: %s\n", getMessage(STARTING_RESCUE_MESSAGE));
                    dbprint(" N=%.1f, E=%.1f\n", Mavlink_newMessage.gpsLocalData.north,
                     Mavlink_newMessage.gpsLocalData.east);
                }
                // ----  Messages from AtLAs to ComPAS (from Compas.c) -------- // Boat sent position info
                else if (Mavlink_newMessage.gpsLocalData.status == MAVLINK_LOCAL_BOAT_POSITION) {
                    event.flags.haveBoatPositionMessage = TRUE;
                    dbprint("A: pos N=%.1f, E=$.1f\n", Mavlink_newMessage.gpsLocalData.north,
                        Mavlink_newMessage.gpsLocalData.east);
                }
                break;
            /*---------------- Status and Error messages ---------- */
            case MAVLINK_MSG_ID_STATUS_AND_ERROR:
                // ---------- Boat error messages ------------------
                if (Mavlink_newMessage.statusAndErrorData.error != ERROR_NONE) {
                    lastBoatErrorCode = Mavlink_newMessage.statusAndErrorData.error;
                    dbprint("A: err=%s\n", getErrorMessage(lastBoatErrorCode));
                }
                else {
                    // ---------- Boat status messages -----------------
                    if (Mavlink_newMessage.statusAndErrorData.status == MAVLINK_STATUS_ONLINE) {
                        event.flags.haveBoatOnlineMessage = TRUE;
                        dbprint("A: %s\n", getMessage(BOAT_ONLINE_MESSAGE));
                    }
                    else if (Mavlink_newMessage.statusAndErrorData.status == MAVLINK_STATUS_START_RESCUE) {
                        dbprint("A: started a rescue\n");
                    }
                    else if (Mavlink_newMessage.statusAndErrorData.status == MAVLINK_STATUS_RESCUE_SUCCESS) {
                        dbprint("A: successfully rescued person\n");
                    }
                    else if (Mavlink_newMessage.statusAndErrorData.status == MAVLINK_STATUS_OVERRIDE) {
                        dbprint("A: in override mode\n");
                    }
                }
                break;
            /*----------------  Barometer altitude message ----------------*/
            case MAVLINK_MSG_ID_BAROMETER:
                event.flags.haveBarometerMessage = TRUE;
                dbprint("A: barom A=%.2f T=%.2f\n", Mavlink_newMessage.barometerData.altitude,
                    Mavlink_newMessage.barometerData.temperature);
                break;
            default:
                // Unknown message ID
                event.flags.haveUnknownMessage = TRUE;
                dbprint("Unknown: 0x%X\n", lastMavlinkMessageID);
                break;
        } // switch
    }
} // doWatchdog()

/**********************************************************************
 * Function: init
 * @return None.
 * @remark Initializes the boat's master state machine and all modules.
 * @author David Goodman
 * @date 2013.04.24
 **********************************************************************/
static void Watchdog_init(void) {
    Board_init();
    Board_configure(USE_SERIAL | USE_TIMER);
    //Board_configure(USE_LCD | USE_TIMER);
    DELAY(10);

    // I2C bus
    //I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);

    #ifdef USE_XBEE
    Xbee_init(XBEE_UART_ID);
    #endif

    #ifdef USE_LOGGER
    Logger_init();
    #endif
}


/*
 *  Entry point
 */
int main(void) {

    Watchdog_init();
    dbprint("Watchdog initialized.\n");

    while (1) {
        doWatchdog();

        #ifdef USE_XBEE
        Xbee_runSM();
        #endif
    }

    return (EXIT_SUCCESS);
}

