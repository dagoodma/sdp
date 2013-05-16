/**
 * @file    Mavlink.h
 * @author  John Ash
 * @author  David Goodman
 *
 * @brief
 * Interface for the Mavlink message protocol.
 *
 * @details
 * This module provides an interface for sending and receiving messages
 * over Mavlink, which is a messaging protocol.
 *
 * @date March 10, 2013, 10:03 AM  -- Created
 */

#ifndef OUR_MAVLINK_H
#define OUR_MAVLINK_H

#include <stdbool.h>
#include "mavlink/autoLifeguard/mavlink.h"
#include "Xbee.h"
#include "Gps.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/
// Acknowledgement related
#define ACK_STATUS_NO_ACK 0
#define ACK_STATUS_RECIEVED 1
#define ACK_STATUS_WAIT 2
#define ACK_STATUS_DEAD 3

#define WANT_ACK    TRUE
#define NO_ACK      FALSE

#define MAVLINK_UART_ID UART1_ID

//-------------------- Message Status Codes --------------------------
// Protocol commands
#define MAVLINK_NO_COMMAND              0x0 // no status code for message
#define MAVLINK_RETURN_STATION          0x1 // cc->boat: return to station keep point *
#define MAVLINK_RESET_BOAT              0x2 // cc->boat: reset PIC32
#define MAVLINK_OVERRIDE                0x3 // cc->boat: forces boat to stop *
#define MAVLINK_SAVE_STATION            0x4 // cc->boat: save pos. as station keep point *
#define MAVLINK_REQUEST_ORIGIN          0x5 // boat->cc: please send cc location 

// Status And Error (errors defined in Error.h) messages from boat
#define MAVLINK_STATUS_ONLINE           0x1 // The boat has come online
#define MAVLINK_STATUS_START_RESCUE     0x2 // The boat is on a rescue
#define MAVLINK_STATUS_RESCUE_SUCCESS   0x3 // The boat has rescued the person
#define MAVLINK_STATUS_RETURN_STATION   0x4 // The boat is headed to the station.

// ECEF coordinate related
#define MAVLINK_GEOCENTRIC_ORIGIN       0x1 // cc->boat: sends cc position (origin) *
#define MAVLINK_GEOCENTRIC_ERROR        0x2 // cc->boat: local error coordinate correction

// Local coordinate related
#define MAVLINK_LOCAL_SET_STATION       0x1 // cc->boat: use this station keep point
#define MAVLINK_LOCAL_START_RESCUE      0x2 // cc->boat: rescue person at this point
#define MAVLINK_LOCAL_BOAT_POSITION     0x3 // boat->cc: current boat position

// * at end denotes WANT_ACK

/**********************************************************************
 * PUBLIC VARIABLES                                                   *
 **********************************************************************/

union MAVLINK_MESSAGE {
    mavlink_mavlink_ack_t       ackData;
    mavlink_cmd_other_t         commandOtherData;
    mavlink_status_and_error_t  statusAndErrorData;
    mavlink_gps_geo_t           gpsGeodeticData;
    mavlink_gps_ecef_t          gpsGeocentricData;
    mavlink_gps_ned_t           gpsLocalData;
    mavlink_barometer_t         barometerData;
} Mavlink_newMessage;

mavlink_xbee_heartbeat_t Mavlink_heartbeatData;

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Mavlink_recieve();

bool Mavlink_hasNewMessage();

int Mavlink_getNewMessageID();


bool Mavlink_hasHeartbeat();


/*------------------------- Send Messages ----------------------------*/

void Mavlink_sendAck(uint8_t msgID, uint16_t msgStatus);

void Mavlink_sendHeartbeat();


/* --- Command Other --- */

void Mavlink_sendReturnStation(bool ack);

void Mavlink_sendResetBoat();

void Mavlink_sendOverride(bool ack);

void Mavlink_sendSaveStation(bool ack);

void Mavlink_sendRequestOrigin();


/* --- Status and Error --- */

void Mavlink_sendStatus(uint16_t status);

void Mavlink_sendError(uint16_t error);


/* --- Coordinate Commands --- */

void Mavlink_sendOrigin(bool ack, GeocentricCoordinate *ecefOrigin);

void Mavlink_sendStation(bool ack, LocalCoordinate *nedPos);

void Mavlink_sendStartRescue(bool ack, LocalCoordinate *nedPos);


/* --- Coordinate and Sensor Data --- */

void Mavlink_sendGeocentricError(GeocentricCoordinate *ecefError);

void Mavlink_sendBoatPosition(LocalCoordinate *nedPos);

void Mavlink_sendBarometerData(float temperatureCelsius, float altitude);


#ifdef XBEE_TEST
void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data);
#endif

#endif
