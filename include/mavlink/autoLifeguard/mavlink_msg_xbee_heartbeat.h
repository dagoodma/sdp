// MESSAGE XBEE_HEARTBEAT PACKING

#define MAVLINK_MSG_ID_XBEE_HEARTBEAT 236

typedef struct __mavlink_xbee_heartbeat_t
{
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t data; ///< Holds raw data for use in testing
} mavlink_xbee_heartbeat_t;

#define MAVLINK_MSG_ID_XBEE_HEARTBEAT_LEN 2
#define MAVLINK_MSG_ID_236_LEN 2



#define MAVLINK_MESSAGE_INFO_XBEE_HEARTBEAT { \
	"XBEE_HEARTBEAT", \
	2, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_xbee_heartbeat_t, ack) }, \
         { "data", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_xbee_heartbeat_t, data) }, \
         } \
}


/**
 * @brief Pack a xbee_heartbeat message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds raw data for use in testing
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_xbee_heartbeat_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_xbee_heartbeat_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_XBEE_HEARTBEAT;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 58);
}

/**
 * @brief Pack a xbee_heartbeat message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds raw data for use in testing
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_xbee_heartbeat_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_xbee_heartbeat_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_XBEE_HEARTBEAT;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 58);
}

/**
 * @brief Encode a xbee_heartbeat struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param xbee_heartbeat C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_xbee_heartbeat_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_xbee_heartbeat_t* xbee_heartbeat)
{
	return mavlink_msg_xbee_heartbeat_pack(system_id, component_id, msg, xbee_heartbeat->ack, xbee_heartbeat->data);
}

/**
 * @brief Send a xbee_heartbeat message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds raw data for use in testing
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_xbee_heartbeat_send(mavlink_channel_t chan, uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_XBEE_HEARTBEAT, buf, 2, 58);
#else
	mavlink_xbee_heartbeat_t packet;
	packet.ack = ack;
	packet.data = data;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_XBEE_HEARTBEAT, (const char *)&packet, 2, 58);
#endif
}

#endif

// MESSAGE XBEE_HEARTBEAT UNPACKING


/**
 * @brief Get field ack from xbee_heartbeat message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_xbee_heartbeat_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field data from xbee_heartbeat message
 *
 * @return Holds raw data for use in testing
 */
static inline uint8_t mavlink_msg_xbee_heartbeat_get_data(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a xbee_heartbeat message into a struct
 *
 * @param msg The message to decode
 * @param xbee_heartbeat C-struct to decode the message contents into
 */
static inline void mavlink_msg_xbee_heartbeat_decode(const mavlink_message_t* msg, mavlink_xbee_heartbeat_t* xbee_heartbeat)
{
#if MAVLINK_NEED_BYTE_SWAP
	xbee_heartbeat->ack = mavlink_msg_xbee_heartbeat_get_ack(msg);
	xbee_heartbeat->data = mavlink_msg_xbee_heartbeat_get_data(msg);
#else
	memcpy(xbee_heartbeat, _MAV_PAYLOAD(msg), 2);
#endif
}
