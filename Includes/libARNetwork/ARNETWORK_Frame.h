/**
 *  @file ARNETWORK_Frame.h
 *  @brief define the network frame protocol
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
 */

#ifndef _ARNETWORK_FRAME_H_
#define _ARNETWORK_FRAME_H_

#include <inttypes.h>

/**
 *  @brief type of frame send by the ARNETWORK_Manager
 */
typedef enum
{
    ARNETWORK_FRAME_TYPE_UNINITIALIZED = 0, /**< Unknown type. Don't use */
    ARNETWORK_FRAME_TYPE_ACK, /**< Acknowledgment type. Internal use only */
    ARNETWORK_FRAME_TYPE_DATA, /**< Data type. Main type for data that does not require an acknowledge */
    ARNETWORK_FRAME_TYPE_DATA_LOW_LATENCY, /**< Low latency data type. Should only be used when needed */
    ARNETWORK_FRAME_TYPE_DATA_WITH_ACK, /**< Data that should have an acknowledge type. This type can have a long latency */
    ARNETWORK_FRAME_TYPE_MAX, /**< Unused, iterator maximum value */
}eARNETWORK_FRAME_TYPE;

/**
 *  @brief frame send by the ARNETWORK_Manager
 */
typedef struct __attribute__((__packed__))
{
    uint8_t type; /**< frame type eARNETWORK_FRAME_TYPE */
    uint8_t ID; /**< identifier of the buffer sending the frame */
    uint32_t seq; /**< sequence number of the frame */
    uint32_t size; /**< size of the frame */
    uint8_t *dataPtr; /**< pointer on the data of the frame */
}ARNETWORK_Frame_t;


#endif /** _ARNETWORK_FRAME_H_ */
