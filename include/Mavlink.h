#ifndef OUR_MAVLINK_H
#define OUR_MAVLINK_H

#include "mavlink/autoLifeguard/mavlink.h"
#include "Xbee.h"
#include "Compas.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/
#define ACK_STATUS_NO_ACK 0
#define ACK_STATUS_RECIEVED 1
#define ACK_STATUS_WAIT 2
#define ACK_STATUS_DEAD 3


/**********************************************************************
 * PUBLIC VARIABLES                                                   *
 **********************************************************************/
enum Message_names{
    messageName_test_data = 0,
    messageName_xbee_heartbeat,
    messageName_mavlink_ack,
    messageName_GPS_error,
    messageName_start_rescue,
    messageName_stop_rescue
};

typedef struct{
    uint8_t messageName;
    uint8_t ACK_status;
    uint8_t last_buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t last_length;
    uint8_t last_uart_id;
    uint32_t ACK_time;
}ACK;

ACK start_rescue;

union message {
    mavlink_reset_t resetData;
    mavlink_gps_geo_t gpsGeodeticData;
    mavlink_gps_ecef_t gpsGeocentricData;
    mavlink_gps_ned_t gpsLocalData;
    mavlink_barometer_t barometerData;
} newMessage;

// Reset message status flags
#define MAVLINK_RESET_INITIALIZE        0x1
#define MAVLINK_RESET_RETURN_STATION    0x2
#define MAVLINK_RESET_BOAT              0x3
#define MAVLINK_RESET_OVERRIDE          0x4

// GPS ECEF message status flags
#define MAVLINK_GEOCENTRIC_ORIGIN       0x1
#define MAVLINK_GEOCENTRIC_ERROR        0x2

// GPS Local message status flags
#define MAVLINK_LOCAL_SET_STATION       0x1
#define MAVLINK_LOCAL_START_RESCUE      0x2
#define MAVLINK_LOCAL_BOAT              0x3

#define WANT_ACK    TRUE
#define NO_ACK      FALSE

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Mavlink_recieve(uint8_t uart_id);

void Mavlink_resend_message(ACK *message);

/**********************************************************************
 * SEND FUNCTIONS                                                     *
 **********************************************************************/
void Mavlink_send_ACK(uint8_t uart_id, uint8_t Message_Name);

void Mavlink_send_xbee_heartbeat(uint8_t uart_id, uint8_t data);

void Mavlink_send_start_rescue(uint8_t uart_id, uint8_t ack, uint8_t status, float latitude, float longitude);

void Mavlink_send_gps_ned_error(uint8_t uart_id, float north, float east);

void Mavlink_send_gps_geo_origin(uint8_t uart_id, float latitude, float longitude);

void Mavlink_send_barometer_data(uint8_t uart_id, int32_t temp_C, float temp_F, int32_t pressure, float altitude);

#ifdef XBEE_TEST
void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data);
#endif

/**********************************************************************
 * RECIEVE FUNCTIONS                                                  *
 **********************************************************************/
void Compas_recieve_start_rescue(mavlink_start_rescue_t* packet);

void Mavlink_recieve_GPS_geo_origin(mavlink_gps_geo_origin_t* packet);

void Mavlink_recieve_GPS_ned_error(mavlink_gps_ned_error_t* packet);

void Mavlink_recieve_ACK(mavlink_mavlink_ack_t* packet);

void Mavlink_recieve_barometer_data(mavlink_barometer_data_t* packet);
#endif
