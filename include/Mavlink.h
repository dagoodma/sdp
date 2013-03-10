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
    uint8_t* last_buf;
    uint16_t last_length;
    uint8_t last_uart_id;
    uint32_t ACK_time;
}ACK;

ACK start_rescue;
/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Mavlink_recieve(uint8_t uart_id);

void Mavlink_send_ACK(uint8_t uart_id, uint8_t Message_Name);

void Mavlink_send_xbee_heartbeat(uint8_t uart_id, uint8_t data);

void Mavlink_recieve_ACK(mavlink_mavlink_ack_t* packet);

void Mavlink_resend_message(ACK message);
/*
uint8_t Mavlink_returnACKStatus(uint8_t message_name);

void Mavlink_editACKStatus(uint8_t message_name, uint8_t new_status);
*/

#ifdef XBEE_TEST
void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data);
#endif
#endif