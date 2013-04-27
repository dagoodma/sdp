/** @file
 *	@brief MAVLink comm protocol testsuite generated from autoLifeguard.xml
 *	@see http://qgroundcontrol.org/mavlink/
 */
#ifndef AUTOLIFEGUARD_TESTSUITE_H
#define AUTOLIFEGUARD_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL

static void mavlink_test_autoLifeguard(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{

	mavlink_test_autoLifeguard(system_id, component_id, last_msg);
}
#endif




static void mavlink_test_test_data(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_test_data_t packet_in = {
		5,
	};
	mavlink_test_data_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.data = packet_in.data;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_test_data_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_test_data_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_test_data_pack(system_id, component_id, &msg , packet1.data );
	mavlink_msg_test_data_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_test_data_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.data );
	mavlink_msg_test_data_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_test_data_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_test_data_send(MAVLINK_COMM_1 , packet1.data );
	mavlink_msg_test_data_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_xbee_heartbeat(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_xbee_heartbeat_t packet_in = {
		5,
	72,
	};
	mavlink_xbee_heartbeat_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.ack = packet_in.ack;
        	packet1.data = packet_in.data;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_xbee_heartbeat_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_xbee_heartbeat_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_xbee_heartbeat_pack(system_id, component_id, &msg , packet1.ack , packet1.data );
	mavlink_msg_xbee_heartbeat_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_xbee_heartbeat_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.data );
	mavlink_msg_xbee_heartbeat_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_xbee_heartbeat_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_xbee_heartbeat_send(MAVLINK_COMM_1 , packet1.ack , packet1.data );
	mavlink_msg_xbee_heartbeat_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_mavlink_ack(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_mavlink_ack_t packet_in = {
		17235,
	139,
	};
	mavlink_mavlink_ack_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.msgStatus = packet_in.msgStatus;
        	packet1.msgID = packet_in.msgID;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_mavlink_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_pack(system_id, component_id, &msg , packet1.msgID , packet1.msgStatus );
	mavlink_msg_mavlink_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.msgID , packet1.msgStatus );
	mavlink_msg_mavlink_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_mavlink_ack_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_send(MAVLINK_COMM_1 , packet1.msgID , packet1.msgStatus );
	mavlink_msg_mavlink_ack_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_cmd_other(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_cmd_other_t packet_in = {
		5,
	72,
	};
	mavlink_cmd_other_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.ack = packet_in.ack;
        	packet1.command = packet_in.command;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_cmd_other_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_cmd_other_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_cmd_other_pack(system_id, component_id, &msg , packet1.ack , packet1.command );
	mavlink_msg_cmd_other_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_cmd_other_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.command );
	mavlink_msg_cmd_other_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_cmd_other_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_cmd_other_send(MAVLINK_COMM_1 , packet1.ack , packet1.command );
	mavlink_msg_cmd_other_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_status_and_error(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_status_and_error_t packet_in = {
		17235,
	17339,
	17,
	};
	mavlink_status_and_error_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.status = packet_in.status;
        	packet1.error = packet_in.error;
        	packet1.ack = packet_in.ack;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_status_and_error_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_status_and_error_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_status_and_error_pack(system_id, component_id, &msg , packet1.ack , packet1.status , packet1.error );
	mavlink_msg_status_and_error_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_status_and_error_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.status , packet1.error );
	mavlink_msg_status_and_error_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_status_and_error_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_status_and_error_send(MAVLINK_COMM_1 , packet1.ack , packet1.status , packet1.error );
	mavlink_msg_status_and_error_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_gps_geo(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_gps_geo_t packet_in = {
		17.0,
	45.0,
	29,
	};
	mavlink_gps_geo_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.latitiude = packet_in.latitiude;
        	packet1.longitude = packet_in.longitude;
        	packet1.ack = packet_in.ack;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_geo_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_gps_geo_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_geo_pack(system_id, component_id, &msg , packet1.ack , packet1.latitiude , packet1.longitude );
	mavlink_msg_gps_geo_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_geo_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.latitiude , packet1.longitude );
	mavlink_msg_gps_geo_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_gps_geo_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_geo_send(MAVLINK_COMM_1 , packet1.ack , packet1.latitiude , packet1.longitude );
	mavlink_msg_gps_geo_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_gps_ecef(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_gps_ecef_t packet_in = {
		17.0,
	45.0,
	73.0,
	41,
	108,
	};
	mavlink_gps_ecef_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.x = packet_in.x;
        	packet1.y = packet_in.y;
        	packet1.z = packet_in.z;
        	packet1.ack = packet_in.ack;
        	packet1.status = packet_in.status;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ecef_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_gps_ecef_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ecef_pack(system_id, component_id, &msg , packet1.ack , packet1.status , packet1.x , packet1.y , packet1.z );
	mavlink_msg_gps_ecef_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ecef_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.status , packet1.x , packet1.y , packet1.z );
	mavlink_msg_gps_ecef_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_gps_ecef_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ecef_send(MAVLINK_COMM_1 , packet1.ack , packet1.status , packet1.x , packet1.y , packet1.z );
	mavlink_msg_gps_ecef_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_gps_ned(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_gps_ned_t packet_in = {
		17.0,
	45.0,
	73.0,
	41,
	108,
	};
	mavlink_gps_ned_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.north = packet_in.north;
        	packet1.east = packet_in.east;
        	packet1.down = packet_in.down;
        	packet1.ack = packet_in.ack;
        	packet1.status = packet_in.status;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ned_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_gps_ned_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ned_pack(system_id, component_id, &msg , packet1.ack , packet1.status , packet1.north , packet1.east , packet1.down );
	mavlink_msg_gps_ned_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ned_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.status , packet1.north , packet1.east , packet1.down );
	mavlink_msg_gps_ned_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_gps_ned_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_ned_send(MAVLINK_COMM_1 , packet1.ack , packet1.status , packet1.north , packet1.east , packet1.down );
	mavlink_msg_gps_ned_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_barometer(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_barometer_t packet_in = {
		17.0,
	45.0,
	29,
	};
	mavlink_barometer_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.temperature = packet_in.temperature;
        	packet1.altitude = packet_in.altitude;
        	packet1.ack = packet_in.ack;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_barometer_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_barometer_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_barometer_pack(system_id, component_id, &msg , packet1.ack , packet1.temperature , packet1.altitude );
	mavlink_msg_barometer_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_barometer_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.temperature , packet1.altitude );
	mavlink_msg_barometer_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_barometer_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_barometer_send(MAVLINK_COMM_1 , packet1.ack , packet1.temperature , packet1.altitude );
	mavlink_msg_barometer_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_autoLifeguard(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_test_test_data(system_id, component_id, last_msg);
	mavlink_test_xbee_heartbeat(system_id, component_id, last_msg);
	mavlink_test_mavlink_ack(system_id, component_id, last_msg);
	mavlink_test_cmd_other(system_id, component_id, last_msg);
	mavlink_test_status_and_error(system_id, component_id, last_msg);
	mavlink_test_gps_geo(system_id, component_id, last_msg);
	mavlink_test_gps_ecef(system_id, component_id, last_msg);
	mavlink_test_gps_ned(system_id, component_id, last_msg);
	mavlink_test_barometer(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // AUTOLIFEGUARD_TESTSUITE_H
