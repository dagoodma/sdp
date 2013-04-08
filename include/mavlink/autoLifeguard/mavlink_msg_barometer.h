// MESSAGE BAROMETER PACKING

#define MAVLINK_MSG_ID_BAROMETER 242

typedef struct __mavlink_barometer_t
{
 int32_t temperature; ///< Contains the temperature in degrees celcius
 float altitude; ///< Contains the altitude calculated from the barometer
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
} mavlink_barometer_t;

#define MAVLINK_MSG_ID_BAROMETER_LEN 9
#define MAVLINK_MSG_ID_242_LEN 9



#define MAVLINK_MESSAGE_INFO_BAROMETER { \
	"BAROMETER", \
	3, \
	{  { "temperature", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_barometer_t, temperature) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_barometer_t, altitude) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_barometer_t, ack) }, \
         } \
}


/**
 * @brief Pack a barometer message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param temperature Contains the temperature in degrees celcius
 * @param altitude Contains the altitude calculated from the barometer
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_barometer_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, int32_t temperature, float altitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_int32_t(buf, 0, temperature);
	_mav_put_float(buf, 4, altitude);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_barometer_t packet;
	packet.temperature = temperature;
	packet.altitude = altitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_BAROMETER;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 136);
}

/**
 * @brief Pack a barometer message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param temperature Contains the temperature in degrees celcius
 * @param altitude Contains the altitude calculated from the barometer
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_barometer_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,int32_t temperature,float altitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_int32_t(buf, 0, temperature);
	_mav_put_float(buf, 4, altitude);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_barometer_t packet;
	packet.temperature = temperature;
	packet.altitude = altitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_BAROMETER;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 136);
}

/**
 * @brief Encode a barometer struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param barometer C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_barometer_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_barometer_t* barometer)
{
	return mavlink_msg_barometer_pack(system_id, component_id, msg, barometer->ack, barometer->temperature, barometer->altitude);
}

/**
 * @brief Send a barometer message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param temperature Contains the temperature in degrees celcius
 * @param altitude Contains the altitude calculated from the barometer
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_barometer_send(mavlink_channel_t chan, uint8_t ack, int32_t temperature, float altitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_int32_t(buf, 0, temperature);
	_mav_put_float(buf, 4, altitude);
	_mav_put_uint8_t(buf, 8, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_BAROMETER, buf, 9, 136);
#else
	mavlink_barometer_t packet;
	packet.temperature = temperature;
	packet.altitude = altitude;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_BAROMETER, (const char *)&packet, 9, 136);
#endif
}

#endif

// MESSAGE BAROMETER UNPACKING


/**
 * @brief Get field ack from barometer message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_barometer_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field temperature from barometer message
 *
 * @return Contains the temperature in degrees celcius
 */
static inline int32_t mavlink_msg_barometer_get_temperature(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field altitude from barometer message
 *
 * @return Contains the altitude calculated from the barometer
 */
static inline float mavlink_msg_barometer_get_altitude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Decode a barometer message into a struct
 *
 * @param msg The message to decode
 * @param barometer C-struct to decode the message contents into
 */
static inline void mavlink_msg_barometer_decode(const mavlink_message_t* msg, mavlink_barometer_t* barometer)
{
#if MAVLINK_NEED_BYTE_SWAP
	barometer->temperature = mavlink_msg_barometer_get_temperature(msg);
	barometer->altitude = mavlink_msg_barometer_get_altitude(msg);
	barometer->ack = mavlink_msg_barometer_get_ack(msg);
#else
	memcpy(barometer, _MAV_PAYLOAD(msg), 9);
#endif
}
