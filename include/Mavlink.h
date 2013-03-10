#ifndef OUR_MAVLINK_H
#define OUR_MAVLINK_H

#include "mavlink/autoLifeguard/mavlink.h"

enum Message_names{
    messageName_test_data = 0,
    messageName_xbee_heartbeat,
    messageName_mavlink_ack,
    messageName_GPS_error,
    messageName_start_rescue,
    messageName_stop_rescue
};

#define ACK_STATUS_NO_ACK 0
#define ACK_STATUS_RECIEVED 1
#define ACK_STATUS_WAIT 2
#define ACK_STATUS_DEAD 3

void Mavlink_recieve(uint8_t uart_id);

#include "Xbee.h"
#include "Compas.h"


void Mavlink_send_ACK(uint8_t uart_id, uint8_t Message_Name);

void Mavlink_send_xbee_heartbeat(uint8_t uart_id, uint8_t data);
#ifdef XBEE_TEST
void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data);
#endif
void Mavlink_recieve_ACK(mavlink_mavlink_ack_t* packet);

void Compass_message_recieve_start_rescue(mavlink_start_rescue_t* packet);

uint8_t Mavlink_returnACKStatus(uint8_t message_name);

void Mavlink_editACKStatus(uint8_t message_name, uint8_t new_status);

#endif