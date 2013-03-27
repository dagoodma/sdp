// MESSAGE GPS_NED_ERROR PACKING

#define MAVLINK_MSG_ID_GPS_NED_ERROR 240

typedef struct __mavlink_gps_ned_error_t
{
 float North; ///< North Error amount
 float East; ///< East Error amount
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
} mavlink_gps_ned_error_t;

#define MAVLINK_MSG_ID_GPS_NED_ERROR_LEN 9
#define MAVLINK_MSG_ID_240_LEN 9



#define MAVLINK_MESSAGE_INFO_GPS_NED_ERROR { \
	"GPS_NED_ERROR", \
	3, \
	{  { "North", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_ned_error_t, North) }, \
         { "East", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_ned_error_t, East) }, \
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
 * @param North North Error amount
 * @param East East Error amount
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_error_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, float North, float East)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, North);
	_mav_put_float(buf, 4, East);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_ned_error_t packet;
	packet.North = North;
	packet.East = East;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED_ERROR;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 245);
}

/**
 * @brief Pack a gps_ned_error message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param North North Error amount
 * @param East East Error amount
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_error_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,float North,float East)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, North);
	_mav_put_float(buf, 4, East);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_ned_error_t packet;
	packet.North = North;
	packet.East = East;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED_ERROR;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 245);
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
	return mavlink_msg_gps_ned_error_pack(system_id, component_id, msg, gps_ned_error->ack, gps_ned_error->North, gps_ned_error->East);
}

/**
 * @brief Send a gps_ned_error message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param North North Error amount
 * @param East East Error amount
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_ned_error_send(mavlink_channel_t chan, uint8_t ack, float North, float East)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, North);
	_mav_put_float(buf, 4, East);
	_mav_put_uint8_t(buf, 8, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED_ERROR, buf, 9, 245);
#else
	mavlink_gps_ned_error_t packet;
	packet.North = North;
	packet.East = East;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED_ERROR, (const char *)&packet, 9, 245);
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
 * @brief Get field North from gps_ned_error message
 *
 * @return North Error amount
 */
static inline float mavlink_msg_gps_ned_error_get_North(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field East from gps_ned_error message
 *
 * @return East Error amount
 */
static inline float mavlink_msg_gps_ned_error_get_East(const mavlink_message_t* msg)
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
	gps_ned_error->North = mavlink_msg_gps_ned_error_get_North(msg);
	gps_ned_error->East = mavlink_msg_gps_ned_error_get_East(msg);
	gps_ned_error->ack = mavlink_msg_gps_ned_error_get_ack(msg);
#else
	memcpy(gps_ned_error, _MAV_PAYLOAD(msg), 9);
#endif
}
