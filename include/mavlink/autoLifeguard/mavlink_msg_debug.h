// MESSAGE DEBUG PACKING

#define MAVLINK_MSG_ID_DEBUG 245

typedef struct __mavlink_debug_t
{
 uint8_t ack; ///< Always FALSE.
 char sender; ///< Sent by AtLAs (0x1) or ComPAS (0x2).
 char message[100]; ///< String containing a debug message."
} mavlink_debug_t;

#define MAVLINK_MSG_ID_DEBUG_LEN 102
#define MAVLINK_MSG_ID_245_LEN 102

#define MAVLINK_MSG_DEBUG_FIELD_MESSAGE_LEN 100

#define MAVLINK_MESSAGE_INFO_DEBUG { \
	"DEBUG", \
	3, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_debug_t, ack) }, \
         { "sender", NULL, MAVLINK_TYPE_CHAR, 0, 1, offsetof(mavlink_debug_t, sender) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 100, 2, offsetof(mavlink_debug_t, message) }, \
         } \
}


/**
 * @brief Pack a debug message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack Always FALSE.
 * @param sender Sent by AtLAs (0x1) or ComPAS (0x2).
 * @param message String containing a debug message."
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_debug_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, char sender, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[102];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_char(buf, 1, sender);
	_mav_put_char_array(buf, 2, message, 100);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 102);
#else
	mavlink_debug_t packet;
	packet.ack = ack;
	packet.sender = sender;
	mav_array_memcpy(packet.message, message, sizeof(char)*100);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 102);
#endif

	msg->msgid = MAVLINK_MSG_ID_DEBUG;
	return mavlink_finalize_message(msg, system_id, component_id, 102, 216);
}

/**
 * @brief Pack a debug message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack Always FALSE.
 * @param sender Sent by AtLAs (0x1) or ComPAS (0x2).
 * @param message String containing a debug message."
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_debug_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,char sender,const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[102];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_char(buf, 1, sender);
	_mav_put_char_array(buf, 2, message, 100);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 102);
#else
	mavlink_debug_t packet;
	packet.ack = ack;
	packet.sender = sender;
	mav_array_memcpy(packet.message, message, sizeof(char)*100);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 102);
#endif

	msg->msgid = MAVLINK_MSG_ID_DEBUG;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 102, 216);
}

/**
 * @brief Encode a debug struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param debug C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_debug_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_debug_t* debug)
{
	return mavlink_msg_debug_pack(system_id, component_id, msg, debug->ack, debug->sender, debug->message);
}

/**
 * @brief Send a debug message
 * @param chan MAVLink channel to send the message
 *
 * @param ack Always FALSE.
 * @param sender Sent by AtLAs (0x1) or ComPAS (0x2).
 * @param message String containing a debug message."
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_debug_send(mavlink_channel_t chan, uint8_t ack, char sender, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[102];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_char(buf, 1, sender);
	_mav_put_char_array(buf, 2, message, 100);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_DEBUG, buf, 102, 216);
#else
	mavlink_debug_t packet;
	packet.ack = ack;
	packet.sender = sender;
	mav_array_memcpy(packet.message, message, sizeof(char)*100);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_DEBUG, (const char *)&packet, 102, 216);
#endif
}

#endif

// MESSAGE DEBUG UNPACKING


/**
 * @brief Get field ack from debug message
 *
 * @return Always FALSE.
 */
static inline uint8_t mavlink_msg_debug_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field sender from debug message
 *
 * @return Sent by AtLAs (0x1) or ComPAS (0x2).
 */
static inline char mavlink_msg_debug_get_sender(const mavlink_message_t* msg)
{
	return _MAV_RETURN_char(msg,  1);
}

/**
 * @brief Get field message from debug message
 *
 * @return String containing a debug message."
 */
static inline uint16_t mavlink_msg_debug_get_message(const mavlink_message_t* msg, char *message)
{
	return _MAV_RETURN_char_array(msg, message, 100,  2);
}

/**
 * @brief Decode a debug message into a struct
 *
 * @param msg The message to decode
 * @param debug C-struct to decode the message contents into
 */
static inline void mavlink_msg_debug_decode(const mavlink_message_t* msg, mavlink_debug_t* debug)
{
#if MAVLINK_NEED_BYTE_SWAP
	debug->ack = mavlink_msg_debug_get_ack(msg);
	debug->sender = mavlink_msg_debug_get_sender(msg);
	mavlink_msg_debug_get_message(msg, debug->message);
#else
	memcpy(debug, _MAV_PAYLOAD(msg), 102);
#endif
}
