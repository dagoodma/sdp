/**********************************************************************
 Module
 Mavlink.c

 Author: John Ash, David Goodman

***********************************************************************/
#include <xc.h>
#include "Mavlink.h"
#include "Uart.h"
#include "Board.h"
#include "Xbee.h"

static int packet_drops = 0;
static mavlink_message_t msg;
static mavlink_status_t status;


static uint8_t newMsgID = 0;
static BOOL hasNewMsg = FALSE;

#define MAV_NUMBER 15 // defines the MAV number, arbitrary
#define COMP_ID 15
#define MAVLINK_UART_ID UART1_ID

/********************************************************************
 * PRIVATE PROTOTYPES                                               *
 ********************************************************************/
static void sendGpsNed(bool ack, uint8_t status, LocalCoordinate *nedPos);
static void sendStatusAndError(uint16_t status, uint16_t error);
static void sendCmdOther(bool ack, uint8_t command);
static void sendGpsEcef(bool ack, uint8_t status, GeocentricCoordinate *ecef);
static void sendBarometer(float temperatureCelsius, float altitude);

/********************************************************************
 * Public Functions
 ********************************************************************/

void Mavlink_recieve(){
    while(UART_isReceiveEmpty(MAVLINK_UART_ID) == FALSE) {
        uint8_t c = UART_getChar(MAVLINK_UART_ID);
        //if a message can be deciphered
        if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            switch(msg.msgid) {
                case MAVLINK_MSG_ID_XBEE_HEARTBEAT:
                {//REQUIRED DO NOT REMOVE -- JOHN
                    mavlink_xbee_heartbeat_t data;
                    mavlink_msg_xbee_heartbeat_decode(&msg, &data);
                    //call outside function to handle data
                    Xbee_recieved_message_heartbeat(&data);
                    break;
                }//REQUIRED DO NOT REMOVE -- JOHN
                case MAVLINK_MSG_ID_MAVLINK_ACK:
                {//REQUIRED DO NOT REMOVE -- JOHN
                    mavlink_mavlink_ack_t data;
                    mavlink_msg_mavlink_ack_decode(&msg, &data);

                }//REQUIRED DO NOT REMOVE -- JOHN
                    break;
#ifdef XBEE_TEST
                case MAVLINK_MSG_ID_TEST_DATA:
                    mavlink_test_data_t data;
                    mavlink_msg_test_data_decode(&msg, &data);
                    //call outside function to handle data
                    Xbee_message_data_test(&data);
                    break;
#endif
                    /*
                case MAVLINK_MSG_ID_RESET:
                    //mavlink_reset_t newMessage.resetData;
                    mavlink_msg_reset_decode(&msg, &(newMessage.resetData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
                     */
                case MAVLINK_MSG_ID_GPS_GEO:
                    //mavlink_gps_geo_t newMessage.gpsGeodeticData;
                    mavlink_msg_gps_geo_decode(&msg, &(Mavlink_newMessage.gpsGeodeticData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
                case MAVLINK_MSG_ID_GPS_ECEF:
                    //mavlink_gps_ecef_t newMessage.gpsGeocentricData;
                    mavlink_msg_gps_ecef_decode(&msg, &(Mavlink_newMessage.gpsGeocentricData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
                case MAVLINK_MSG_ID_GPS_NED:
                    //mavlink_gps_ned_t newMessage.gpsLocalData;
                    mavlink_msg_gps_ned_decode(&msg,&(Mavlink_newMessage.gpsLocalData));
                    if (Mavlink_newMessage.gpsLocalData.ack == TRUE) {
                        Mavlink_sendAck(XBEE_UART_ID, MAVLINK_MSG_ID_GPS_NED);
                    }
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
                case MAVLINK_MSG_ID_BAROMETER:
                    //mavlink_barometer_t newMessage.barometerData;
                    mavlink_msg_barometer_decode(&msg, &(Mavlink_newMessage.barometerData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
            }
        }
    }
    packet_drops += status.packet_rx_drop_count;
}

/*------------------------- Receive Messages ----------------------------*/

bool Mavlink_hasNewMessage() {
    uint8_t result = hasNewMsg;
    hasNewMsg = FALSE;
    return result;
}

int Mavlink_getNewMessageID() {
    return newMsgID;
}


/*------------------------- Send Messages ----------------------------*/

void Mavlink_sendAck(uint8_t msgID, uint16_t msgStatus){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_mavlink_ack_pack(MAV_NUMBER, COMP_ID, &msg, msgID, msgStatus);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
}
void Mavlink_sendHearbeat(uint8_t data){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_xbee_heartbeat_pack(MAV_NUMBER, COMP_ID, &msg, TRUE, data);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
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



//DAVIDS NEW FUNCTIONS:

/* --- Command Other --- */

void Mavlink_sendReturnStation(bool ack){
    sendCmdOther(ack, 0x01);
}

void Mavlink_sendReinitialize(bool ack){
    sendCmdOther(ack, 0x02);
}

void Mavlink_sendOverride(bool ack){
    sendCmdOther(ack, 0x03);
}

/* --- Status and Error --- */

void Mavlink_sendStatus(uint16_t status){
    sendStatusAndError(status, 0);
}

void Mavlink_sendError(uint16_t error){
    sendStatusAndError(0, error);
}


/* --- Coordinate Commands --- */

void Mavlink_sendOrigin(bool ack, GeocentricCoordinate *ecefOrigin){
    sendGpsEcef(ack, 0x01, ecefOrigin);
}

void Mavlink_sendStation(bool ack, LocalCoordinate *nedPos){
    sendGpsNed(ack, 0x1, nedPos);
}

void Mavlink_sendStartRescue(bool ack, LocalCoordinate *nedPos){
    sendGpsNed(ack, 0x2, nedPos);
}


/* --- Coordinate and Sensor Data --- */

void Mavlink_sendGeocentricError(GeocentricCoordinate *ecefError){
    sendGpsEcef(NO_ACK, 0x02, ecefError);
}

void Mavlink_sendBoatPosition(LocalCoordinate *nedPos){
    sendGpsNed(NO_ACK, 0x3, nedPos);
}


/************************************************************************
 * PRIVATE FUNCTIONS                                                    *
 ************************************************************************/

static void sendGpsNed(bool ack, uint8_t status, LocalCoordinate *nedPos){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_gps_ned_pack(MAV_NUMBER, COMP_ID, &msg, ack,status, nedPos->n, nedPos->e, nedPos->d);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
}

static void sendStatusAndError(uint16_t status, uint16_t error){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_status_and_error_pack(MAV_NUMBER, COMP_ID, &msg, NO_ACK, status, error);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
}

static void sendCmdOther(bool ack, uint8_t command){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_cmd_other_pack(MAV_NUMBER, COMP_ID, &msg, ack, command);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
}

static void sendGpsEcef(bool ack, uint8_t status, GeocentricCoordinate *ecef){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_gps_ecef_pack(MAV_NUMBER, COMP_ID, &msg, ack, status,ecef->x,ecef->y,ecef->z);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
}

static void Mavlink_sendBarometer(float temperatureCelsius, float altitude){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_barometer_pack(MAV_NUMBER, COMP_ID, &msg, NO_ACK, temperatureCelsius, altitude);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(MAVLINK_UART_ID, buf, length);
}
