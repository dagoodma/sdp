// MESSAGE DATA PACKING

#define MAVLINK_MSG_ID_DATA 243

typedef struct __mavlink_data_t
{
 float temperature; ///< The temperature in degrees Celsius.
 float altitude; ///< The altitude calculated from the barometer.
 uint16_t batVolt1; ///< Battery reading for electronics (NiMH) in millivolts.
 uint16_t batVolt2; ///< Battery reading for motors (LiPo) in millivolts.
 uint8_t ack; ///< TRUE or FALSE if acknowledgement required.
} mavlink_data_t;

#define MAVLINK_MSG_ID_DATA_LEN 13
#define MAVLINK_MSG_ID_243_LEN 13



#define MAVLINK_MESSAGE_INFO_DATA { \
	"DATA", \
	5, \
	{  { "temperature", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_data_t, temperature) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_data_t, altitude) }, \
         { "batVolt1", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_data_t, batVolt1) }, \
         { "batVolt2", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_data_t, batVolt2) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 12, offsetof(mavlink_data_t, ack) }, \
         } \
}


/**
 * @brief Pack a data message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param temperature The temperature in degrees Celsius.
 * @param altitude The altitude calculated from the barometer.
 * @param batVolt1 Battery reading for electronics (NiMH) in millivolts.
 * @param batVolt2 Battery reading for motors (LiPo) in millivolts.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_data_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, float temperature, float altitude, uint16_t batVolt1, uint16_t batVolt2)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[13];
	_mav_put_float(buf, 0, temperature);
	_mav_put_float(buf, 4, altitude);
	_mav_put_uint16_t(buf, 8, batVolt1);
	_mav_put_uint16_t(buf, 10, batVolt2);
	_mav_put_uint8_t(buf, 12, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 13);
#else
	mavlink_data_t packet;
	packet.temperature = temperature;
	packet.altitude = altitude;
	packet.batVolt1 = batVolt1;
	packet.batVolt2 = batVolt2;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 13);
#endif

	msg->msgid = MAVLINK_MSG_ID_DATA;
	return mavlink_finalize_message(msg, system_id, component_id, 13, 187);
}

/**
 * @brief Pack a data message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param temperature The temperature in degrees Celsius.
 * @param altitude The altitude calculated from the barometer.
 * @param batVolt1 Battery reading for electronics (NiMH) in millivolts.
 * @param batVolt2 Battery reading for motors (LiPo) in millivolts.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_data_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,float temperature,float altitude,uint16_t batVolt1,uint16_t batVolt2)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[13];
	_mav_put_float(buf, 0, temperature);
	_mav_put_float(buf, 4, altitude);
	_mav_put_uint16_t(buf, 8, batVolt1);
	_mav_put_uint16_t(buf, 10, batVolt2);
	_mav_put_uint8_t(buf, 12, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 13);
#else
	mavlink_data_t packet;
	packet.temperature = temperature;
	packet.altitude = altitude;
	packet.batVolt1 = batVolt1;
	packet.batVolt2 = batVolt2;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 13);
#endif

	msg->msgid = MAVLINK_MSG_ID_DATA;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 13, 187);
}

/**
 * @brief Encode a data struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param data C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_data_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_data_t* data)
{
	return mavlink_msg_data_pack(system_id, component_id, msg, data->ack, data->temperature, data->altitude, data->batVolt1, data->batVolt2);
}

/**
 * @brief Send a data message
 * @param chan MAVLink channel to send the message
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param temperature The temperature in degrees Celsius.
 * @param altitude The altitude calculated from the barometer.
 * @param batVolt1 Battery reading for electronics (NiMH) in millivolts.
 * @param batVolt2 Battery reading for motors (LiPo) in millivolts.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_data_send(mavlink_channel_t chan, uint8_t ack, float temperature, float altitude, uint16_t batVolt1, uint16_t batVolt2)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[13];
	_mav_put_float(buf, 0, temperature);
	_mav_put_float(buf, 4, altitude);
	_mav_put_uint16_t(buf, 8, batVolt1);
	_mav_put_uint16_t(buf, 10, batVolt2);
	_mav_put_uint8_t(buf, 12, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_DATA, buf, 13, 187);
#else
	mavlink_data_t packet;
	packet.temperature = temperature;
	packet.altitude = altitude;
	packet.batVolt1 = batVolt1;
	packet.batVolt2 = batVolt2;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_DATA, (const char *)&packet, 13, 187);
#endif
}

#endif

// MESSAGE DATA UNPACKING


/**
 * @brief Get field ack from data message
 *
 * @return TRUE or FALSE if acknowledgement required.
 */
static inline uint8_t mavlink_msg_data_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  12);
}

/**
 * @brief Get field temperature from data message
 *
 * @return The temperature in degrees Celsius.
 */
static inline float mavlink_msg_data_get_temperature(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field altitude from data message
 *
 * @return The altitude calculated from the barometer.
 */
static inline float mavlink_msg_data_get_altitude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field batVolt1 from data message
 *
 * @return Battery reading for electronics (NiMH) in millivolts.
 */
static inline uint16_t mavlink_msg_data_get_batVolt1(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  8);
}

/**
 * @brief Get field batVolt2 from data message
 *
 * @return Battery reading for motors (LiPo) in millivolts.
 */
static inline uint16_t mavlink_msg_data_get_batVolt2(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  10);
}

/**
 * @brief Decode a data message into a struct
 *
 * @param msg The message to decode
 * @param data C-struct to decode the message contents into
 */
static inline void mavlink_msg_data_decode(const mavlink_message_t* msg, mavlink_data_t* data)
{
#if MAVLINK_NEED_BYTE_SWAP
	data->temperature = mavlink_msg_data_get_temperature(msg);
	data->altitude = mavlink_msg_data_get_altitude(msg);
	data->batVolt1 = mavlink_msg_data_get_batVolt1(msg);
	data->batVolt2 = mavlink_msg_data_get_batVolt2(msg);
	data->ack = mavlink_msg_data_get_ack(msg);
#else
	memcpy(data, _MAV_PAYLOAD(msg), 13);
#endif
}
