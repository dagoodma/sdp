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
#endif