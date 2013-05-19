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

static BOOL hasHeartbeat = FALSE;

#define MAV_NUMBER 15 // defines the MAV number, arbitrary
#define COMP_ID 15


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
    while(UART_isReceiveEmpty(Xbee_getUartId()) == FALSE) {
        uint8_t c = UART_getChar(Xbee_getUartId());
        //if a message can be deciphered
        if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            switch(msg.msgid) {
                case MAVLINK_MSG_ID_XBEE_HEARTBEAT:
                {
                    mavlink_msg_xbee_heartbeat_decode(&msg, &Mavlink_heartbeatData);
                    hasHeartbeat = TRUE;

                    break;
                }
                #ifdef XBEE_TEST
                case MAVLINK_MSG_ID_TEST_DATA:
                {
                    mavlink_test_data_t data;
                    mavlink_msg_test_data_decode(&msg, &data);
                    //call outside function to handle data
                    Xbee_message_data_test(&data);
                }
                    break;
                #endif
                case MAVLINK_MSG_ID_MAVLINK_ACK:
                    mavlink_msg_mavlink_ack_decode(&msg, &Mavlink_newMessage.ackData);
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;  
                case MAVLINK_MSG_ID_CMD_OTHER:
                    //mavlink_reset_t newMessage.resetData;
                    mavlink_msg_cmd_other_decode(&msg, &(Mavlink_newMessage.commandOtherData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
                case MAVLINK_MSG_ID_STATUS_AND_ERROR:
                    //mavlink_reset_t newMessage.resetData;
                    mavlink_msg_status_and_error_decode(&msg, &(Mavlink_newMessage.statusAndErrorData));
                    hasNewMsg = TRUE;
                    newMsgID = msg.msgid;
                    break;
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
                    /*if (Mavlink_newMessage.gpsLocalData.ack == TRUE) {
                        Mavlink_sendAck(Xbee_getUartId(), MAVLINK_MSG_ID_GPS_NED);
                    }*/
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

bool Mavlink_hasHeartbeat() {
    BOOL result = hasHeartbeat;
    hasHeartbeat = FALSE;
    return result;
}


/*------------------------- Send Messages ----------------------------*/

void Mavlink_sendAck(uint8_t msgID, uint16_t msgStatus){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_mavlink_ack_pack(MAV_NUMBER, COMP_ID, &msg, msgID, msgStatus);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
}
void Mavlink_sendHeartbeat(){
    uint8_t data = 0x1;
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_xbee_heartbeat_pack(MAV_NUMBER, COMP_ID, &msg, NO_ACK, data);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
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
    sendCmdOther(ack, MAVLINK_RETURN_STATION);
}

void Mavlink_sendResetBoat(){
    sendCmdOther(NO_ACK, MAVLINK_RESET_BOAT);
}

void Mavlink_sendOverride(bool ack){
    sendCmdOther(ack, MAVLINK_OVERRIDE);
}

void Mavlink_sendSaveStation(bool ack) {
    sendCmdOther(ack, MAVLINK_SAVE_STATION);
}

void Mavlink_sendRequestOrigin() {
    sendCmdOther(NO_ACK, MAVLINK_REQUEST_ORIGIN);
}


/* --- Status and Error --- */

void Mavlink_sendStatus(uint16_t status){
    sendStatusAndError(status, ERROR_NONE);
}

void Mavlink_sendError(uint16_t error){
    sendStatusAndError(MAVLINK_STATUS_NONE, error);
}


/* --- Coordinate Commands --- */

void Mavlink_sendOrigin(bool ack, GeocentricCoordinate *ecefOrigin){
    sendGpsEcef(ack, MAVLINK_GEOCENTRIC_ORIGIN, ecefOrigin);
}

void Mavlink_sendStation(bool ack, LocalCoordinate *nedPos){
    sendGpsNed(ack, MAVLINK_LOCAL_SET_STATION, nedPos);
}

void Mavlink_sendStartRescue(bool ack, LocalCoordinate *nedPos){
    sendGpsNed(ack, MAVLINK_LOCAL_START_RESCUE, nedPos);
}


/* --- Coordinate and Sensor Data --- */

void Mavlink_sendGeocentricError(GeocentricCoordinate *ecefError){
    sendGpsEcef(NO_ACK, MAVLINK_GEOCENTRIC_ERROR, ecefError);
}

void Mavlink_sendBoatPosition(LocalCoordinate *nedPos){
    sendGpsNed(NO_ACK, MAVLINK_LOCAL_BOAT_POSITION, nedPos);
}

void Mavlink_sendBarometerData(float temperatureCelsius, float altitude){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_barometer_pack(MAV_NUMBER, COMP_ID, &msg, NO_ACK, temperatureCelsius, altitude);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
}

/************************************************************************
 * PRIVATE FUNCTIONS                                                    *
 ************************************************************************/

static void sendGpsNed(bool ack, uint8_t status, LocalCoordinate *nedPos){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_gps_ned_pack(MAV_NUMBER, COMP_ID, &msg, ack,status,
            nedPos->north, nedPos->east, nedPos->down);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
}

static void sendStatusAndError(uint16_t status, uint16_t error){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_status_and_error_pack(MAV_NUMBER, COMP_ID, &msg, NO_ACK, status, error);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
}

static void sendCmdOther(bool ack, uint8_t command){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_cmd_other_pack(MAV_NUMBER, COMP_ID, &msg, ack, command);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
}

static void sendGpsEcef(bool ack, uint8_t status, GeocentricCoordinate *ecef){
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_gps_ecef_pack(MAV_NUMBER, COMP_ID, &msg, ack, status,ecef->x,ecef->y,ecef->z);
    uint16_t length = mavlink_msg_to_send_buffer(buf, &msg);
    UART_putString(Xbee_getUartId(), buf, length);
}

