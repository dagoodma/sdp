// MESSAGE GPS_GEO PACKING

#define MAVLINK_MSG_ID_GPS_GEO 240

typedef struct __mavlink_gps_geo_t
{
 float latitiude; ///< Geodetic latitude position in degrees
 float longitude; ///< Geodetic longitude position in degrees
 uint8_t ack; ///< TRUE or FALSE if acknowledgement required.
} mavlink_gps_geo_t;

#define MAVLINK_MSG_ID_GPS_GEO_LEN 9
#define MAVLINK_MSG_ID_240_LEN 9



#define MAVLINK_MESSAGE_INFO_GPS_GEO { \
	"GPS_GEO", \
	3, \
	{  { "latitiude", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_geo_t, latitiude) }, \
         { "longitude", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_geo_t, longitude) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_gps_geo_t, ack) }, \
         } \
}


/**
 * @brief Pack a gps_geo message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param latitiude Geodetic latitude position in degrees
 * @param longitude Geodetic longitude position in degrees
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_geo_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, float latitiude, float longitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, latitiude);
	_mav_put_float(buf, 4, longitude);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_geo_t packet;
	packet.latitiude = latitiude;
	packet.longitude = longitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_GEO;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 251);
}

/**
 * @brief Pack a gps_geo message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param latitiude Geodetic latitude position in degrees
 * @param longitude Geodetic longitude position in degrees
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_geo_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,float latitiude,float longitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, latitiude);
	_mav_put_float(buf, 4, longitude);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_geo_t packet;
	packet.latitiude = latitiude;
	packet.longitude = longitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_GEO;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 251);
}

/**
 * @brief Encode a gps_geo struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param gps_geo C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_gps_geo_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_gps_geo_t* gps_geo)
{
	return mavlink_msg_gps_geo_pack(system_id, component_id, msg, gps_geo->ack, gps_geo->latitiude, gps_geo->longitude);
}

/**
 * @brief Send a gps_geo message
 * @param chan MAVLink channel to send the message
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param latitiude Geodetic latitude position in degrees
 * @param longitude Geodetic longitude position in degrees
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_geo_send(mavlink_channel_t chan, uint8_t ack, float latitiude, float longitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, latitiude);
	_mav_put_float(buf, 4, longitude);
	_mav_put_uint8_t(buf, 8, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_GEO, buf, 9, 251);
#else
	mavlink_gps_geo_t packet;
	packet.latitiude = latitiude;
	packet.longitude = longitude;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_GEO, (const char *)&packet, 9, 251);
#endif
}

#endif

// MESSAGE GPS_GEO UNPACKING


/**
 * @brief Get field ack from gps_geo message
 *
 * @return TRUE or FALSE if acknowledgement required.
 */
static inline uint8_t mavlink_msg_gps_geo_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field latitiude from gps_geo message
 *
 * @return Geodetic latitude position in degrees
 */
static inline float mavlink_msg_gps_geo_get_latitiude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field longitude from gps_geo message
 *
 * @return Geodetic longitude position in degrees
 */
static inline float mavlink_msg_gps_geo_get_longitude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Decode a gps_geo message into a struct
 *
 * @param msg The message to decode
 * @param gps_geo C-struct to decode the message contents into
 */
static inline void mavlink_msg_gps_geo_decode(const mavlink_message_t* msg, mavlink_gps_geo_t* gps_geo)
{
#if MAVLINK_NEED_BYTE_SWAP
	gps_geo->latitiude = mavlink_msg_gps_geo_get_latitiude(msg);
	gps_geo->longitude = mavlink_msg_gps_geo_get_longitude(msg);
	gps_geo->ack = mavlink_msg_gps_geo_get_ack(msg);
#else
	memcpy(gps_geo, _MAV_PAYLOAD(msg), 9);
#endif
}
