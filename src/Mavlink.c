#include <xc.h>
#include "Mavlink.h"
#include "Uart.h"
#include "Board.h"
#include "Xbee.h"
#include "Compas.h"

static int packet_drops = 0;
static mavlink_message_t msg;
static mavlink_status_t status;


static uint8_t newMsgID = 0;
static BOOL hasNewMsg = FALSE;

#define MAV_NUMBER 15 // defines the MAV number, arbitrary
#define COMP_ID 15

void Mavlink_recieve(uint8_t uart_id){
    while(UART_isReceiveEmpty(uart_id) == FALSE){
        uint8_t c = UART_getChar(uart_id);
        //if a message can be deciphered
        if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            switch(msg.msgid){
                case MAVLINK_MSG_ID_XBEE_HEARTBEAT:
                {
                    mavlink_xbee_heartbeat_t data;
                    mavlink_msg_xbee_heartbeat_decode(&msg, &data);
                    //call outside function to handle data
                    Xbee_recieved_message_heartbeat(&data);
                }break;
#ifdef XBEE_TEST
                case MAVLINK_MSG_ID_TEST_DATA:
                {
                    mavlink_test_data_t data;
                    mavlink_msg_test_data_decode(&msg, &data);
                    //call outside function to handle data
                    Xbee_message_data_test(&data);
                }break;
#endif
                case MAVLINK_MSG_ID_START_RESCUE:
                {
                    mavlink_start_rescue_t newMessage.startRescueData;
                    mavlink_msg_start_rescue_decode(&msg, &(newMessage.startRescueData);
                    if(newMessage.startRescueData.ack == TRUE){
                        Mavlink_send_ACK(XBEE_UART_ID, messageName_start_rescue);
                    }
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                }break;
                case MAVLINK_MSG_ID_STOP_RESCUE:
                {
                    mavlink_stop_rescue_t newMessage.stopRescueData;
                    mavlink_msg_start_rescue_decode(&msg, &(newMessage.stopRescueData);
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                }break;
                case MAVLINK_MSG_ID_MAVLINK_ACK:
                {
                    mavlink_mavlink_ack_t data;
                    mavlink_msg_mavlink_ack_decode(&msg, &data);
                    Mavlink_recieve_ACK(&data);
                }break;
                case MAVLINK_MSG_ID_GPS_ECEF_ERROR:
                {
                    mavlink_gps_ecef_error_t newMessage.gpsErrorData;
                    mavlink_msg_gps_ecef_error_decode(&msg,&(newMessage.gpsErrorData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                }break;
                case MAVLINK_MSG_ID_GPS_GEO:
                {
                    mavlink_gps_geo_t newMessage.gpsGeoData;
                    mavlink_msg_gps_geo_decode(&msg,&(newMessage.gpsGeoData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                }break;
                case MAVLINK_MSG_ID_GPS_ECEF:
                {
                    mavlink_gps_ecef_t newMessage.gpsECEFData;
                    mavlink_msg_gps_ecef_decode(&msg,&(newMessage.gpsECEFData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                }break;
                case MAVLINK_MSG_ID_BAROMETER:
                {
                    mavlink_barometer_t newMessage.barometerData;
                    mavlink_msg_barometer_decode(&msg, &(newMessage.barometerData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                }break;
            }
        }
    }
    packet_drops += status.packet_rx_drop_count;
}
/*************************************************************************
 * SEND FUNCTIONS                                                        *
 *************************************************************************/

void Mavlink_send_ACK(uint8_t uart_id, uint8_t Message_Name){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_mavlink_ack_pack(MAV_NUMBER, COMP_ID, &msg, Message_Name);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
}
void Mavlink_send_xbee_heartbeat(uint8_t uart_id, uint8_t data){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_xbee_heartbeat_pack(MAV_NUMBER, COMP_ID, &msg, TRUE, data);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
}

void Mavlink_send_start_rescue(uint8_t uart_id, uint8_t ack, uint8_t status, float latitude, float longitude){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_start_rescue_pack(MAV_NUMBER, COMP_ID, &msg, ack, status, latitude, longitude);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
    if(ack == TRUE){
        start_rescue.ACK_status = ACK_STATUS_WAIT;
        int x;
        for(x = 0; x <=MAVLINK_MAX_PACKET_LEN;x++){
            start_rescue.last_buf[x] = buf[x];
        }
        start_rescue.last_length = length;
        start_rescue.last_uart_id = uart_id;
    }
}

void Mavlink_send_gps_geo_origin(uint8_t uart_id, float latitude, float longitude){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_gps_geo_origin_pack(MAV_NUMBER, COMP_ID, &msg, FALSE, latitude, longitude);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
}

void Mavlink_send_gps_ned_error(uint8_t uart_id, float north, float east){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_gps_ned_error_pack(MAV_NUMBER, COMP_ID, &msg, FALSE, north, east);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
}

void Mavlink_send_barometer_data(uint8_t uart_id, int32_t temp_C, float temp_F, int32_t pressure, float altitude){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_barometer_data_pack(MAV_NUMBER, COMP_ID, &msg, FALSE, temp_C, temp_F, pressure, altitude);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(uart_id, buf, length);
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


/*************************************************************************
 * RECIEVE FUNCTIONS                                                     *
 *************************************************************************/

void Mavlink_recieve_ACK(mavlink_mavlink_ack_t* packet){
    switch(packet->Message_Name){
        case messageName_start_rescue:
        {
            start_rescue.ACK_status = ACK_STATUS_RECIEVED;
        }break;
    }
}

void Compas_recieve_start_rescue(mavlink_start_rescue_t* packet){
    printf("North: %f East: %f\n",packet->north,packet->east);
}

void Mavlink_recieve_GPS_geo_origin(mavlink_gps_geo_origin_t* packet){
    //What would you like this function to do?
    //most likely store these values to a global variable and set a flag to read them. etc....
    float lat, longi;
    lat = packet->latitiude;
    longi = packet->longitude;
}

void Mavlink_recieve_GPS_ned_error(mavlink_gps_ned_error_t* packet){
    //What would you like this function to do?
    //most likely store these values to a global variable and set a flag to read them. etc....
    float North, East;
    North = packet->north;
    East = packet->east;
}

void Mavlink_recieve_barometer_data(mavlink_barometer_data_t* packet){
    their_barometer.altitude = packet->altitude;
    their_barometer.pressure = packet->pressure;
    their_barometer.temp_C = packet->temperature_celcius;
    their_barometer.temp_F = packet->temperature_fahrenheit;
}

BOOL Mavlink_hasNewMessage() {
    uint8_t result = hasNewMsg;
    hasNewMsg = FALSE;
    return result;
}

int Mavlink_getNewMessageID() {
    return newMsgID;
}


/*************************************************************************
 * PUBLIC FUNCTIONS                                                      *
 *************************************************************************/

void Mavlink_resend_message(ACK *message){
    UART_putString(message->last_uart_id, message->last_buf, message->last_length);
    message->ACK_status = ACK_STATUS_WAIT;
}