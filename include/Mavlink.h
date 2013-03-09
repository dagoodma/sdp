#ifndef OUR_MAVLINK_H
#define OUR_MAVLINK_H

#include "mavlink/autoLifeguard/mavlink.h"

enum Message_names{
    MessageName_Test_data = 0,
    MessageName_Xbee_heartbeat,
    MessageName_Mavlink_ack,
    MessageName_GPS_error,
    MessageName_Start_rescue,
    MessageName_Stop_rescue
};



void Mavlink_recieve(uint8_t uart_id);

#include "Xbee.h"
#include "Compass.h"


void Mavlink_send_ACK(uint8_t uart_id, uint8_t Message_Name);

void Mavlink_send_xbee_heartbeat(uint8_t uart_id, uint8_t data);
#ifdef XBEE_TEST
void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data);
#endif


#endif