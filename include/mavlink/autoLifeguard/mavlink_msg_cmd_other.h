// MESSAGE CMD_OTHER PACKING

#define MAVLINK_MSG_ID_CMD_OTHER 238

typedef struct __mavlink_cmd_other_t
{
 uint8_t ack; ///< TRUE or FALSE if acknowledgement required.
 uint8_t command; ///< Return to station (0x1), reinitialize boat (0x2), start override (0x3), save station (0x4)
} mavlink_cmd_other_t;

#define MAVLINK_MSG_ID_CMD_OTHER_LEN 2
#define MAVLINK_MSG_ID_238_LEN 2



#define MAVLINK_MESSAGE_INFO_CMD_OTHER { \
	"CMD_OTHER", \
	2, \
	{  { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_cmd_other_t, ack) }, \
         { "command", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_cmd_other_t, command) }, \
         } \
}


/**
 * @brief Pack a cmd_other message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param command Return to station (0x1), reinitialize boat (0x2), start override (0x3), save station (0x4)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cmd_other_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, command);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_cmd_other_t packet;
	packet.ack = ack;
	packet.command = command;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_CMD_OTHER;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 167);
}

/**
 * @brief Pack a cmd_other message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param command Return to station (0x1), reinitialize boat (0x2), start override (0x3), save station (0x4)
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cmd_other_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, command);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_cmd_other_t packet;
	packet.ack = ack;
	packet.command = command;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_CMD_OTHER;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 167);
}

/**
 * @brief Encode a cmd_other struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cmd_other C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cmd_other_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cmd_other_t* cmd_other)
{
	return mavlink_msg_cmd_other_pack(system_id, component_id, msg, cmd_other->ack, cmd_other->command);
}

/**
 * @brief Send a cmd_other message
 * @param chan MAVLink channel to send the message
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param command Return to station (0x1), reinitialize boat (0x2), start override (0x3), save station (0x4)
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cmd_other_send(mavlink_channel_t chan, uint8_t ack, uint8_t command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, ack);
	_mav_put_uint8_t(buf, 1, command);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CMD_OTHER, buf, 2, 167);
#else
	mavlink_cmd_other_t packet;
	packet.ack = ack;
	packet.command = command;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CMD_OTHER, (const char *)&packet, 2, 167);
#endif
}

#endif

// MESSAGE CMD_OTHER UNPACKING


/**
 * @brief Get field ack from cmd_other message
 *
 * @return TRUE or FALSE if acknowledgement required.
 */
static inline uint8_t mavlink_msg_cmd_other_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field command from cmd_other message
 *
 * @return Return to station (0x1), reinitialize boat (0x2), start override (0x3), save station (0x4)
 */
static inline uint8_t mavlink_msg_cmd_other_get_command(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a cmd_other message into a struct
 *
 * @param msg The message to decode
 * @param cmd_other C-struct to decode the message contents into
 */
static inline void mavlink_msg_cmd_other_decode(const mavlink_message_t* msg, mavlink_cmd_other_t* cmd_other)
{
#if MAVLINK_NEED_BYTE_SWAP
	cmd_other->ack = mavlink_msg_cmd_other_get_ack(msg);
	cmd_other->command = mavlink_msg_cmd_other_get_command(msg);
#else
	memcpy(cmd_other, _MAV_PAYLOAD(msg), 2);
#endif
}
