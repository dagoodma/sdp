// MESSAGE STATUS_AND_ERROR PACKING

#define MAVLINK_MSG_ID_STATUS_AND_ERROR 239

typedef struct __mavlink_status_and_error_t
{
 uint16_t status; ///< Boat is initializing (0x1), boat is rescuing (0x2),  boat rescue success (0x3), boat is returning to station (0x4), boat in override (0x5).
 uint16_t error; ///< Error codes defined in Error.h
 uint8_t ack; ///< TRUE or FALSE if acknowledgement required.
} mavlink_status_and_error_t;

#define MAVLINK_MSG_ID_STATUS_AND_ERROR_LEN 5
#define MAVLINK_MSG_ID_239_LEN 5



#define MAVLINK_MESSAGE_INFO_STATUS_AND_ERROR { \
	"STATUS_AND_ERROR", \
	3, \
	{  { "status", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_status_and_error_t, status) }, \
         { "error", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_status_and_error_t, error) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_status_and_error_t, ack) }, \
         } \
}


/**
 * @brief Pack a status_and_error message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Boat is initializing (0x1), boat is rescuing (0x2),  boat rescue success (0x3), boat is returning to station (0x4), boat in override (0x5).
 * @param error Error codes defined in Error.h
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_status_and_error_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint16_t status, uint16_t error)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[5];
	_mav_put_uint16_t(buf, 0, status);
	_mav_put_uint16_t(buf, 2, error);
	_mav_put_uint8_t(buf, 4, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 5);
#else
	mavlink_status_and_error_t packet;
	packet.status = status;
	packet.error = error;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 5);
#endif

	msg->msgid = MAVLINK_MSG_ID_STATUS_AND_ERROR;
	return mavlink_finalize_message(msg, system_id, component_id, 5, 220);
}

/**
 * @brief Pack a status_and_error message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Boat is initializing (0x1), boat is rescuing (0x2),  boat rescue success (0x3), boat is returning to station (0x4), boat in override (0x5).
 * @param error Error codes defined in Error.h
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_status_and_error_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint16_t status,uint16_t error)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[5];
	_mav_put_uint16_t(buf, 0, status);
	_mav_put_uint16_t(buf, 2, error);
	_mav_put_uint8_t(buf, 4, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 5);
#else
	mavlink_status_and_error_t packet;
	packet.status = status;
	packet.error = error;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 5);
#endif

	msg->msgid = MAVLINK_MSG_ID_STATUS_AND_ERROR;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 5, 220);
}

/**
 * @brief Encode a status_and_error struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param status_and_error C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_status_and_error_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_status_and_error_t* status_and_error)
{
	return mavlink_msg_status_and_error_pack(system_id, component_id, msg, status_and_error->ack, status_and_error->status, status_and_error->error);
}

/**
 * @brief Send a status_and_error message
 * @param chan MAVLink channel to send the message
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Boat is initializing (0x1), boat is rescuing (0x2),  boat rescue success (0x3), boat is returning to station (0x4), boat in override (0x5).
 * @param error Error codes defined in Error.h
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_status_and_error_send(mavlink_channel_t chan, uint8_t ack, uint16_t status, uint16_t error)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[5];
	_mav_put_uint16_t(buf, 0, status);
	_mav_put_uint16_t(buf, 2, error);
	_mav_put_uint8_t(buf, 4, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_STATUS_AND_ERROR, buf, 5, 220);
#else
	mavlink_status_and_error_t packet;
	packet.status = status;
	packet.error = error;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_STATUS_AND_ERROR, (const char *)&packet, 5, 220);
#endif
}

#endif

// MESSAGE STATUS_AND_ERROR UNPACKING


/**
 * @brief Get field ack from status_and_error message
 *
 * @return TRUE or FALSE if acknowledgement required.
 */
static inline uint8_t mavlink_msg_status_and_error_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field status from status_and_error message
 *
 * @return Boat is initializing (0x1), boat is rescuing (0x2),  boat rescue success (0x3), boat is returning to station (0x4), boat in override (0x5).
 */
static inline uint16_t mavlink_msg_status_and_error_get_status(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Get field error from status_and_error message
 *
 * @return Error codes defined in Error.h
 */
static inline uint16_t mavlink_msg_status_and_error_get_error(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  2);
}

/**
 * @brief Decode a status_and_error message into a struct
 *
 * @param msg The message to decode
 * @param status_and_error C-struct to decode the message contents into
 */
static inline void mavlink_msg_status_and_error_decode(const mavlink_message_t* msg, mavlink_status_and_error_t* status_and_error)
{
#if MAVLINK_NEED_BYTE_SWAP
	status_and_error->status = mavlink_msg_status_and_error_get_status(msg);
	status_and_error->error = mavlink_msg_status_and_error_get_error(msg);
	status_and_error->ack = mavlink_msg_status_and_error_get_ack(msg);
#else
	memcpy(status_and_error, _MAV_PAYLOAD(msg), 5);
#endif
}
