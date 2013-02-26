// MESSAGE STOP_RESCUE PACKING

#define MAVLINK_MSG_ID_STOP_RESCUE 242

typedef struct __mavlink_stop_rescue_t
{
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t data; ///< Holds a Message ID number
} mavlink_stop_rescue_t;

#define MAVLINK_MSG_ID_STOP_RESCUE_LEN 2
#define MAVLINK_MSG_ID_242_LEN 2



#define MAVLINK_MESSAGE_INFO_STOP_RESCUE { \
	"STOP_RESCUE", \
	2, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_stop_rescue_t, ack) }, \
         { "data", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_stop_rescue_t, data) }, \
         } \
}


/**
 * @brief Pack a stop_rescue message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_stop_rescue_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_stop_rescue_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_STOP_RESCUE;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 187);
}

/**
 * @brief Pack a stop_rescue message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_stop_rescue_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_stop_rescue_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_STOP_RESCUE;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 187);
}

/**
 * @brief Encode a stop_rescue struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param stop_rescue C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_stop_rescue_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_stop_rescue_t* stop_rescue)
{
	return mavlink_msg_stop_rescue_pack(system_id, component_id, msg, stop_rescue->ack, stop_rescue->data);
}

/**
 * @brief Send a stop_rescue message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_stop_rescue_send(mavlink_channel_t chan, uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_STOP_RESCUE, buf, 2, 187);
#else
	mavlink_stop_rescue_t packet;
	packet.ack = ack;
	packet.data = data;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_STOP_RESCUE, (const char *)&packet, 2, 187);
#endif
}

#endif

// MESSAGE STOP_RESCUE UNPACKING


/**
 * @brief Get field ack from stop_rescue message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_stop_rescue_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field data from stop_rescue message
 *
 * @return Holds a Message ID number
 */
static inline uint8_t mavlink_msg_stop_rescue_get_data(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a stop_rescue message into a struct
 *
 * @param msg The message to decode
 * @param stop_rescue C-struct to decode the message contents into
 */
static inline void mavlink_msg_stop_rescue_decode(const mavlink_message_t* msg, mavlink_stop_rescue_t* stop_rescue)
{
#if MAVLINK_NEED_BYTE_SWAP
	stop_rescue->ack = mavlink_msg_stop_rescue_get_ack(msg);
	stop_rescue->data = mavlink_msg_stop_rescue_get_data(msg);
#else
	memcpy(stop_rescue, _MAV_PAYLOAD(msg), 2);
#endif
}
