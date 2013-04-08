// MESSAGE RESET PACKING

#define MAVLINK_MSG_ID_RESET 238

typedef struct __mavlink_reset_t
{
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t status; ///< Boat is initializing (0x1), return to station (0x2), tells the boat to reinitialize (0x3), shutdown into override (0x4)
} mavlink_reset_t;

#define MAVLINK_MSG_ID_RESET_LEN 2
#define MAVLINK_MSG_ID_238_LEN 2



#define MAVLINK_MESSAGE_INFO_RESET { \
	"RESET", \
	2, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_reset_t, ack) }, \
         { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_reset_t, status) }, \
         } \
}


/**
 * @brief Pack a reset message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Boat is initializing (0x1), return to station (0x2), tells the boat to reinitialize (0x3), shutdown into override (0x4)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_reset_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_reset_t packet;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_RESET;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 167);
}

/**
 * @brief Pack a reset message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Boat is initializing (0x1), return to station (0x2), tells the boat to reinitialize (0x3), shutdown into override (0x4)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_reset_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_reset_t packet;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_RESET;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 167);
}

/**
 * @brief Encode a reset struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param reset C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_reset_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_reset_t* reset)
{
	return mavlink_msg_reset_pack(system_id, component_id, msg, reset->ack, reset->status);
}

/**
 * @brief Send a reset message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Boat is initializing (0x1), return to station (0x2), tells the boat to reinitialize (0x3), shutdown into override (0x4)
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_reset_send(mavlink_channel_t chan, uint8_t ack, uint8_t status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, status);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_RESET, buf, 2, 167);
#else
	mavlink_reset_t packet;
	packet.ack = ack;
	packet.status = status;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_RESET, (const char *)&packet, 2, 167);
#endif
}

#endif

// MESSAGE RESET UNPACKING


/**
 * @brief Get field ack from reset message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_reset_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field status from reset message
 *
 * @return Boat is initializing (0x1), return to station (0x2), tells the boat to reinitialize (0x3), shutdown into override (0x4)
 */
static inline uint8_t mavlink_msg_reset_get_status(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a reset message into a struct
 *
 * @param msg The message to decode
 * @param reset C-struct to decode the message contents into
 */
static inline void mavlink_msg_reset_decode(const mavlink_message_t* msg, mavlink_reset_t* reset)
{
#if MAVLINK_NEED_BYTE_SWAP
	reset->ack = mavlink_msg_reset_get_ack(msg);
	reset->status = mavlink_msg_reset_get_status(msg);
#else
	memcpy(reset, _MAV_PAYLOAD(msg), 2);
#endif
}
