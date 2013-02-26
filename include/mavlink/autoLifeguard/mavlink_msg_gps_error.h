// MESSAGE GPS_ERROR PACKING

#define MAVLINK_MSG_ID_GPS_ERROR 240

typedef struct __mavlink_gps_error_t
{
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t data; ///< Holds a Message ID number
} mavlink_gps_error_t;

#define MAVLINK_MSG_ID_GPS_ERROR_LEN 2
#define MAVLINK_MSG_ID_240_LEN 2



#define MAVLINK_MESSAGE_INFO_GPS_ERROR { \
	"GPS_ERROR", \
	2, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_gps_error_t, ack) }, \
         { "data", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_gps_error_t, data) }, \
         } \
}


/**
 * @brief Pack a gps_error message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_error_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_gps_error_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_ERROR;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 110);
}

/**
 * @brief Pack a gps_error message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_error_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_gps_error_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_ERROR;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 110);
}

/**
 * @brief Encode a gps_error struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param gps_error C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_gps_error_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_gps_error_t* gps_error)
{
	return mavlink_msg_gps_error_pack(system_id, component_id, msg, gps_error->ack, gps_error->data);
}

/**
 * @brief Send a gps_error message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_error_send(mavlink_channel_t chan, uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_ERROR, buf, 2, 110);
#else
	mavlink_gps_error_t packet;
	packet.ack = ack;
	packet.data = data;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_ERROR, (const char *)&packet, 2, 110);
#endif
}

#endif

// MESSAGE GPS_ERROR UNPACKING


/**
 * @brief Get field ack from gps_error message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_gps_error_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field data from gps_error message
 *
 * @return Holds a Message ID number
 */
static inline uint8_t mavlink_msg_gps_error_get_data(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a gps_error message into a struct
 *
 * @param msg The message to decode
 * @param gps_error C-struct to decode the message contents into
 */
static inline void mavlink_msg_gps_error_decode(const mavlink_message_t* msg, mavlink_gps_error_t* gps_error)
{
#if MAVLINK_NEED_BYTE_SWAP
	gps_error->ack = mavlink_msg_gps_error_get_ack(msg);
	gps_error->data = mavlink_msg_gps_error_get_data(msg);
#else
	memcpy(gps_error, _MAV_PAYLOAD(msg), 2);
#endif
}
