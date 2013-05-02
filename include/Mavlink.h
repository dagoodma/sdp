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

#define MAVLINK_UART_ID UART2_ID

//-------------------- Message Status Codes --------------------------
// Other command
#define MAVLINK_RETURN_STATION          0x1 // return to station 
#define MAVLINK_REINITIALIZE            0x2 // reinitialize origin and station
#define MAVLINK_OVERRIDE                0x3 // force stop/override 
#define MAVLINK_SAVE_STATION            0x4 // save current position as station

// Status And Error (errors defined in Error.h)
#define MAVLINK_STATUS_START_INITIALIZE 0x1 // boat is starting up
#define MAVLINK_STATUS_START_RESCUE     0x2 // boat is going to rescue
#define MAVLINK_STATUS_RESCUE_SUCCESS   0x3 // boat has rescued 
#define MAVLINK_STATUS_RETURN_STATION   0x4 // boat is returning to station

//  Coordinate commands and data
#define MAVLINK_GEOCENTRIC_ORIGIN       0x1 // command center's origin
#define MAVLINK_GEOCENTRIC_ERROR        0x2 // periodic error corrections

#define MAVLINK_LOCAL_SET_STATION       0x1 // use this station location
#define MAVLINK_LOCAL_START_RESCUE      0x2 // start a rescue at waypoint
#define MAVLINK_LOCAL_BOAT_POSITION     0x3 // reporting boats position

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

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Mavlink_recieve();

//void Mavlink_resend_message(ACK *message);

bool Mavlink_hasNewMessage();

int Mavlink_getNewMessageID();


/*------------------------- Send Messages ----------------------------*/

void Mavlink_sendAck(uint8_t msgID, uint16_t msgStatus);

void Mavlink_sendHearbeat(uint8_t data);


/* --- Command Other --- */

void Mavlink_sendReturnStation(bool ack);

void Mavlink_sendReinitialize(bool ack);

void Mavlink_sendOverride(bool ack);


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
