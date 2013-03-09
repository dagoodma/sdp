// MESSAGE MAVLINK_ACK PACKING

#define MAVLINK_MSG_ID_MAVLINK_ACK 237

typedef struct __mavlink_mavlink_ack_t
{
 uint8_t Message_Name; ///<  Returns the name of message recieved
} mavlink_mavlink_ack_t;

#define MAVLINK_MSG_ID_MAVLINK_ACK_LEN 1
#define MAVLINK_MSG_ID_237_LEN 1



#define MAVLINK_MESSAGE_INFO_MAVLINK_ACK { \
	"MAVLINK_ACK", \
	1, \
	{  { "Message_Name", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_mavlink_ack_t, Message_Name) }, \
         } \
}


/**
 * @brief Pack a mavlink_ack message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param Message_Name  Returns the name of message recieved
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mavlink_ack_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t Message_Name)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[1];
	_mav_put_uint8_t(buf, 0, Message_Name);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 1);
#else
	mavlink_mavlink_ack_t packet;
	packet.Message_Name = Message_Name;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 1);
#endif

	msg->msgid = MAVLINK_MSG_ID_MAVLINK_ACK;
	return mavlink_finalize_message(msg, system_id, component_id, 1, 203);
}

/**
 * @brief Pack a mavlink_ack message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param Message_Name  Returns the name of message recieved
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mavlink_ack_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t Message_Name)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[1];
	_mav_put_uint8_t(buf, 0, Message_Name);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 1);
#else
	mavlink_mavlink_ack_t packet;
	packet.Message_Name = Message_Name;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 1);
#endif

	msg->msgid = MAVLINK_MSG_ID_MAVLINK_ACK;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 1, 203);
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
	return mavlink_msg_mavlink_ack_pack(system_id, component_id, msg, mavlink_ack->Message_Name);
}

/**
 * @brief Send a mavlink_ack message
 * @param chan MAVLink channel to send the message
 *
 * @param Message_Name  Returns the name of message recieved
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_mavlink_ack_send(mavlink_channel_t chan, uint8_t Message_Name)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[1];
	_mav_put_uint8_t(buf, 0, Message_Name);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MAVLINK_ACK, buf, 1, 203);
#else
	mavlink_mavlink_ack_t packet;
	packet.Message_Name = Message_Name;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MAVLINK_ACK, (const char *)&packet, 1, 203);
#endif
}

#endif

// MESSAGE MAVLINK_ACK UNPACKING


/**
 * @brief Get field Message_Name from mavlink_ack message
 *
 * @return  Returns the name of message recieved
 */
static inline uint8_t mavlink_msg_mavlink_ack_get_Message_Name(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
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
	mavlink_ack->Message_Name = mavlink_msg_mavlink_ack_get_Message_Name(msg);
#else
	memcpy(mavlink_ack, _MAV_PAYLOAD(msg), 1);
#endif
}
