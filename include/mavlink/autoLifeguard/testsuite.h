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
		5,
	};
	mavlink_mavlink_ack_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.Message_Name = packet_in.Message_Name;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_mavlink_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_pack(system_id, component_id, &msg , packet1.Message_Name );
	mavlink_msg_mavlink_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mavlink_ack_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.Message_Name );
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
	mavlink_msg_mavlink_ack_send(MAVLINK_COMM_1 , packet1.Message_Name );
	mavlink_msg_mavlink_ack_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_gps_error(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_gps_error_t packet_in = {
		5,
	72,
	};
	mavlink_gps_error_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.ack = packet_in.ack;
        	packet1.data = packet_in.data;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_error_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_gps_error_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_error_pack(system_id, component_id, &msg , packet1.ack , packet1.data );
	mavlink_msg_gps_error_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_error_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.data );
	mavlink_msg_gps_error_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_gps_error_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_gps_error_send(MAVLINK_COMM_1 , packet1.ack , packet1.data );
	mavlink_msg_gps_error_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_start_rescue(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_start_rescue_t packet_in = {
		963497464,
	963497672,
	29,
	96,
	};
	mavlink_start_rescue_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.latitude = packet_in.latitude;
        	packet1.longitude = packet_in.longitude;
        	packet1.ack = packet_in.ack;
        	packet1.status = packet_in.status;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_start_rescue_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_start_rescue_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_start_rescue_pack(system_id, component_id, &msg , packet1.ack , packet1.status , packet1.latitude , packet1.longitude );
	mavlink_msg_start_rescue_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_start_rescue_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.status , packet1.latitude , packet1.longitude );
	mavlink_msg_start_rescue_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_start_rescue_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_start_rescue_send(MAVLINK_COMM_1 , packet1.ack , packet1.status , packet1.latitude , packet1.longitude );
	mavlink_msg_start_rescue_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_stop_rescue(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_stop_rescue_t packet_in = {
		5,
	72,
	};
	mavlink_stop_rescue_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.ack = packet_in.ack;
        	packet1.data = packet_in.data;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_stop_rescue_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_stop_rescue_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_stop_rescue_pack(system_id, component_id, &msg , packet1.ack , packet1.data );
	mavlink_msg_stop_rescue_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_stop_rescue_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.ack , packet1.data );
	mavlink_msg_stop_rescue_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_stop_rescue_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_stop_rescue_send(MAVLINK_COMM_1 , packet1.ack , packet1.data );
	mavlink_msg_stop_rescue_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_autoLifeguard(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_test_test_data(system_id, component_id, last_msg);
	mavlink_test_xbee_heartbeat(system_id, component_id, last_msg);
	mavlink_test_mavlink_ack(system_id, component_id, last_msg);
	mavlink_test_gps_error(system_id, component_id, last_msg);
	mavlink_test_start_rescue(system_id, component_id, last_msg);
	mavlink_test_stop_rescue(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // AUTOLIFEGUARD_TESTSUITE_H
