#include <xc.h>
#include "Mavlink.h"
#include "Uart.h"
#include "Board.h"
#include "Xbee.h"
static int packet_drops = 0;
static mavlink_message_t msg;
static mavlink_status_t status;

#define MAV_NUMBER 15 // defines the MAV number, arbitrary
#define COMP_ID 15


void Mavlink_recieve(uint8_t uart_id){
    while(UART_isReceiveEmpty(uart_id) == FALSE){
        uint8_t c = UART_getChar(uart_id);
        //if a message can be deciphered
        if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            switch(msg.msgid){
                case MAVLINK_MSG_ID_TEST_DATA:
                {
                    mavlink_test_data_t data;
                    mavlink_msg_test_data_decode(&msg, &data);
                    //call outside function to handle data
                    Xbee_message_data_test(&data);
                }break;
            }
        }
    }
    packet_drops += status.packet_rx_drop_count;
}


#ifdef XBEE_TEST
void Mavlink_send_Test_data(uint8_t uart_id, uint8_t data){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_test_data_pack(MAV_NUMBER, COMP_ID, &msg, data);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
}
#endif