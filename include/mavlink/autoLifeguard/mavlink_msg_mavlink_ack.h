// MESSAGE MAVLINK_ACK PACKING

#define MAVLINK_MSG_ID_MAVLINK_ACK 237

typedef struct __mavlink_mavlink_ack_t
{
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t data; ///< Holds a Message ID number
} mavlink_mavlink_ack_t;

#define MAVLINK_MSG_ID_MAVLINK_ACK_LEN 2
#define MAVLINK_MSG_ID_237_LEN 2



#define MAVLINK_MESSAGE_INFO_MAVLINK_ACK { \
	"MAVLINK_ACK", \
	2, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_mavlink_ack_t, ack) }, \
         { "data", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_mavlink_ack_t, data) }, \
         } \
}


/**
 * @brief Pack a mavlink_ack message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mavlink_ack_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_mavlink_ack_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_MAVLINK_ACK;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 161);
}

/**
 * @brief Pack a mavlink_ack message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mavlink_ack_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_mavlink_ack_t packet;
	packet.ack = ack;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_MAVLINK_ACK;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 161);
}

/**
 * @brief Encode a mavlink_ack struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param mavlink_ack C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_mavlink_ack_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_mavlink_ack_t* mavlink_ack)
{
	return mavlink_msg_mavlink_ack_pack(system_id, component_id, msg, mavlink_ack->ack, mavlink_ack->data);
}

/**
 * @brief Send a mavlink_ack message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param data Holds a Message ID number
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_mavlink_ack_send(mavlink_channel_t chan, uint8_t ack, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, data);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MAVLINK_ACK, buf, 2, 161);
#else
	mavlink_mavlink_ack_t packet;
	packet.ack = ack;
	packet.data = data;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MAVLINK_ACK, (const char *)&packet, 2, 161);
#endif
}

#endif

// MESSAGE MAVLINK_ACK UNPACKING


/**
 * @brief Get field ack from mavlink_ack message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_mavlink_ack_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field data from mavlink_ack message
 *
 * @return Holds a Message ID number
 */
static inline uint8_t mavlink_msg_mavlink_ack_get_data(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a mavlink_ack message into a struct
 *
 * @param msg The message to decode
 * @param mavlink_ack C-struct to decode the message contents into
 */
static inline void mavlink_msg_mavlink_ack_decode(const mavlink_message_t* msg, mavlink_mavlink_ack_t* mavlink_ack)
{
#if MAVLINK_NEED_BYTE_SWAP
	mavlink_ack->ack = mavlink_msg_mavlink_ack_get_ack(msg);
	mavlink_ack->data = mavlink_msg_mavlink_ack_get_data(msg);
#else
	memcpy(mavlink_ack, _MAV_PAYLOAD(msg), 2);
#endif
}
