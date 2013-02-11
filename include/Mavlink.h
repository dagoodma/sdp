#ifndef OUR_MAVLINK_H
#define OUR_MAVLINK_H

#include "mavlink/test/autoLifeguard/mavlink.h"

void Mavlink_recieve(uint8_t uart_id);

void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data);

#endif