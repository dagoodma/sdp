// MESSAGE GPS_NED_ERROR PACKING

#define MAVLINK_MSG_ID_GPS_NED_ERROR 240

typedef struct __mavlink_gps_ned_error_t
{
 float north; ///< North Error amount
 float east; ///< East Error amount
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
} mavlink_gps_ned_error_t;

#define MAVLINK_MSG_ID_GPS_NED_ERROR_LEN 9
#define MAVLINK_MSG_ID_240_LEN 9



#define MAVLINK_MESSAGE_INFO_GPS_NED_ERROR { \
	"GPS_NED_ERROR", \
	3, \
	{  { "north", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_ned_error_t, north) }, \
         { "east", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_ned_error_t, east) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_gps_ned_error_t, ack) }, \
         } \
}


/**
 * @brief Pack a gps_ned_error message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param north North Error amount
 * @param east East Error amount
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_error_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, float north, float east)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_ned_error_t packet;
	packet.north = north;
	packet.east = east;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED_ERROR;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 54);
}

/**
 * @brief Pack a gps_ned_error message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param north North Error amount
 * @param east East Error amount
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_error_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,float north,float east)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_ned_error_t packet;
	packet.north = north;
	packet.east = east;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED_ERROR;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 54);
}

/**
 * @brief Encode a gps_ned_error struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param gps_ned_error C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_gps_ned_error_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_gps_ned_error_t* gps_ned_error)
{
	return mavlink_msg_gps_ned_error_pack(system_id, component_id, msg, gps_ned_error->ack, gps_ned_error->north, gps_ned_error->east);
}

/**
 * @brief Send a gps_ned_error message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param north North Error amount
 * @param east East Error amount
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_ned_error_send(mavlink_channel_t chan, uint8_t ack, float north, float east)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_uint8_t(buf, 8, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED_ERROR, buf, 9, 54);
#else
	mavlink_gps_ned_error_t packet;
	packet.north = north;
	packet.east = east;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED_ERROR, (const char *)&packet, 9, 54);
#endif
}

#endif

// MESSAGE GPS_NED_ERROR UNPACKING


/**
 * @brief Get field ack from gps_ned_error message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_gps_ned_error_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field north from gps_ned_error message
 *
 * @return North Error amount
 */
static inline float mavlink_msg_gps_ned_error_get_north(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field east from gps_ned_error message
 *
 * @return East Error amount
 */
static inline float mavlink_msg_gps_ned_error_get_east(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Decode a gps_ned_error message into a struct
 *
 * @param msg The message to decode
 * @param gps_ned_error C-struct to decode the message contents into
 */
static inline void mavlink_msg_gps_ned_error_decode(const mavlink_message_t* msg, mavlink_gps_ned_error_t* gps_ned_error)
{
#if MAVLINK_NEED_BYTE_SWAP
	gps_ned_error->north = mavlink_msg_gps_ned_error_get_north(msg);
	gps_ned_error->east = mavlink_msg_gps_ned_error_get_east(msg);
	gps_ned_error->ack = mavlink_msg_gps_ned_error_get_ack(msg);
#else
	memcpy(gps_ned_error, _MAV_PAYLOAD(msg), 9);
#endif
}
