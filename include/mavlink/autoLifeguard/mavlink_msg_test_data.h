// MESSAGE TEST_DATA PACKING

#define MAVLINK_MSG_ID_TEST_DATA 235

typedef struct __mavlink_test_data_t
{
 uint8_t data; ///< Holds raw data for use in testing
} mavlink_test_data_t;

#define MAVLINK_MSG_ID_TEST_DATA_LEN 1
#define MAVLINK_MSG_ID_235_LEN 1



#define MAVLINK_MESSAGE_INFO_TEST_DATA { \
	"TEST_DATA", \
	1, \
	{  { "data", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_test_data_t, data) }, \
         } \
}


/**
 * @brief Pack a test_data message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param data Holds raw data for use in testing
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_test_data_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[1];
	_mav_put_uint8_t(buf, 0, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 1);
#else
	mavlink_test_data_t packet;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 1);
#endif

	msg->msgid = MAVLINK_MSG_ID_TEST_DATA;
	return mavlink_finalize_message(msg, system_id, component_id, 1, 205);
}

/**
 * @brief Pack a test_data message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param data Holds raw data for use in testing
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_test_data_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[1];
	_mav_put_uint8_t(buf, 0, data);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 1);
#else
	mavlink_test_data_t packet;
	packet.data = data;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 1);
#endif

	msg->msgid = MAVLINK_MSG_ID_TEST_DATA;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 1, 205);
}

/**
 * @brief Encode a test_data struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param test_data C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_test_data_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_test_data_t* test_data)
{
	return mavlink_msg_test_data_pack(system_id, component_id, msg, test_data->data);
}

/**
 * @brief Send a test_data message
 * @param chan MAVLink channel to send the message
 *
 * @param data Holds raw data for use in testing
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_test_data_send(mavlink_channel_t chan, uint8_t data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[1];
	_mav_put_uint8_t(buf, 0, data);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_DATA, buf, 1, 205);
#else
	mavlink_test_data_t packet;
	packet.data = data;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_DATA, (const char *)&packet, 1, 205);
#endif
}

#endif

// MESSAGE TEST_DATA UNPACKING


/**
 * @brief Get field data from test_data message
 *
 * @return Holds raw data for use in testing
 */
static inline uint8_t mavlink_msg_test_data_get_data(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Decode a test_data message into a struct
 *
 * @param msg The message to decode
 * @param test_data C-struct to decode the message contents into
 */
static inline void mavlink_msg_test_data_decode(const mavlink_message_t* msg, mavlink_test_data_t* test_data)
{
#if MAVLINK_NEED_BYTE_SWAP
	test_data->data = mavlink_msg_test_data_get_data(msg);
#else
	memcpy(test_data, _MAV_PAYLOAD(msg), 1);
#endif
}
