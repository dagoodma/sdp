// MESSAGE BAROMETER_DATA PACKING

#define MAVLINK_MSG_ID_BAROMETER_DATA 244

typedef struct __mavlink_barometer_data_t
{
 int32_t temperature_celcius; ///< Contains the temperature in celcius
 float temperature_fahrenheit; ///< Contains the temperature in Fahrenheit
 int32_t pressure; ///< Contains the pressure data from the barometer
 float altitude; ///< Contains the altitude calculated from the barometer
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
} mavlink_barometer_data_t;

#define MAVLINK_MSG_ID_BAROMETER_DATA_LEN 17
#define MAVLINK_MSG_ID_244_LEN 17



#define MAVLINK_MESSAGE_INFO_BAROMETER_DATA { \
	"BAROMETER_DATA", \
	5, \
	{  { "temperature_celcius", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_barometer_data_t, temperature_celcius) }, \
         { "temperature_fahrenheit", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_barometer_data_t, temperature_fahrenheit) }, \
         { "pressure", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_barometer_data_t, pressure) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_barometer_data_t, altitude) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_barometer_data_t, ack) }, \
         } \
}


/**
 * @brief Pack a barometer_data message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param temperature_celcius Contains the temperature in celcius
 * @param temperature_fahrenheit Contains the temperature in Fahrenheit
 * @param pressure Contains the pressure data from the barometer
 * @param altitude Contains the altitude calculated from the barometer
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_barometer_data_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, int32_t temperature_celcius, float temperature_fahrenheit, int32_t pressure, float altitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[17];
	_mav_put_int32_t(buf, 0, temperature_celcius);
	_mav_put_float(buf, 4, temperature_fahrenheit);
	_mav_put_int32_t(buf, 8, pressure);
	_mav_put_float(buf, 12, altitude);
	_mav_put_uint8_t(buf, 16, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 17);
#else
	mavlink_barometer_data_t packet;
	packet.temperature_celcius = temperature_celcius;
	packet.temperature_fahrenheit = temperature_fahrenheit;
	packet.pressure = pressure;
	packet.altitude = altitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 17);
#endif

	msg->msgid = MAVLINK_MSG_ID_BAROMETER_DATA;
	return mavlink_finalize_message(msg, system_id, component_id, 17, 215);
}

/**
 * @brief Pack a barometer_data message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param temperature_celcius Contains the temperature in celcius
 * @param temperature_fahrenheit Contains the temperature in Fahrenheit
 * @param pressure Contains the pressure data from the barometer
 * @param altitude Contains the altitude calculated from the barometer
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_barometer_data_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,int32_t temperature_celcius,float temperature_fahrenheit,int32_t pressure,float altitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[17];
	_mav_put_int32_t(buf, 0, temperature_celcius);
	_mav_put_float(buf, 4, temperature_fahrenheit);
	_mav_put_int32_t(buf, 8, pressure);
	_mav_put_float(buf, 12, altitude);
	_mav_put_uint8_t(buf, 16, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 17);
#else
	mavlink_barometer_data_t packet;
	packet.temperature_celcius = temperature_celcius;
	packet.temperature_fahrenheit = temperature_fahrenheit;
	packet.pressure = pressure;
	packet.altitude = altitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 17);
#endif

	msg->msgid = MAVLINK_MSG_ID_BAROMETER_DATA;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 17, 215);
}

/**
 * @brief Encode a barometer_data struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param barometer_data C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_barometer_data_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_barometer_data_t* barometer_data)
{
	return mavlink_msg_barometer_data_pack(system_id, component_id, msg, barometer_data->ack, barometer_data->temperature_celcius, barometer_data->temperature_fahrenheit, barometer_data->pressure, barometer_data->altitude);
}

/**
 * @brief Send a barometer_data message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param temperature_celcius Contains the temperature in celcius
 * @param temperature_fahrenheit Contains the temperature in Fahrenheit
 * @param pressure Contains the pressure data from the barometer
 * @param altitude Contains the altitude calculated from the barometer
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_barometer_data_send(mavlink_channel_t chan, uint8_t ack, int32_t temperature_celcius, float temperature_fahrenheit, int32_t pressure, float altitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[17];
	_mav_put_int32_t(buf, 0, temperature_celcius);
	_mav_put_float(buf, 4, temperature_fahrenheit);
	_mav_put_int32_t(buf, 8, pressure);
	_mav_put_float(buf, 12, altitude);
	_mav_put_uint8_t(buf, 16, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_BAROMETER_DATA, buf, 17, 215);
#else
	mavlink_barometer_data_t packet;
	packet.temperature_celcius = temperature_celcius;
	packet.temperature_fahrenheit = temperature_fahrenheit;
	packet.pressure = pressure;
	packet.altitude = altitude;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_BAROMETER_DATA, (const char *)&packet, 17, 215);
#endif
}

#endif

// MESSAGE BAROMETER_DATA UNPACKING


/**
 * @brief Get field ack from barometer_data message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_barometer_data_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  16);
}

/**
 * @brief Get field temperature_celcius from barometer_data message
 *
 * @return Contains the temperature in celcius
 */
static inline int32_t mavlink_msg_barometer_data_get_temperature_celcius(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field temperature_fahrenheit from barometer_data message
 *
 * @return Contains the temperature in Fahrenheit
 */
static inline float mavlink_msg_barometer_data_get_temperature_fahrenheit(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field pressure from barometer_data message
 *
 * @return Contains the pressure data from the barometer
 */
static inline int32_t mavlink_msg_barometer_data_get_pressure(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  8);
}

/**
 * @brief Get field altitude from barometer_data message
 *
 * @return Contains the altitude calculated from the barometer
 */
static inline float mavlink_msg_barometer_data_get_altitude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Decode a barometer_data message into a struct
 *
 * @param msg The message to decode
 * @param barometer_data C-struct to decode the message contents into
 */
static inline void mavlink_msg_barometer_data_decode(const mavlink_message_t* msg, mavlink_barometer_data_t* barometer_data)
{
#if MAVLINK_NEED_BYTE_SWAP
	barometer_data->temperature_celcius = mavlink_msg_barometer_data_get_temperature_celcius(msg);
	barometer_data->temperature_fahrenheit = mavlink_msg_barometer_data_get_temperature_fahrenheit(msg);
	barometer_data->pressure = mavlink_msg_barometer_data_get_pressure(msg);
	barometer_data->altitude = mavlink_msg_barometer_data_get_altitude(msg);
	barometer_data->ack = mavlink_msg_barometer_data_get_ack(msg);
#else
	memcpy(barometer_data, _MAV_PAYLOAD(msg), 17);
#endif
}
