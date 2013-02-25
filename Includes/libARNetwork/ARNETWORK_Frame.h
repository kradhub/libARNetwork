/**
 *  @file ARNETWORK_Frame.h
 *  @brief define the network frame protocol
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_FRAME_H_
#define _ARNETWORK_FRAME_H_

#include <inttypes.h>

/**
 *  @brief type of frame send by the NETWORK_Manager
**/
typedef enum
{
    ARNETWORK_FRAME_TYPE_UNINITIALIZED = 0, /**< not known type*/
    ARNETWORK_FRAME_TYPE_ACK, /**< acknowledgment type*/
    ARNETWORK_FRAME_TYPE_DATA, /**< data type*/
    ARNETWORK_FRAME_TYPE_DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
    ARNETWORK_FRAME_TYPE_KEEP_ALIVE /**< keep alive type*/
    
}eARNETWORK_FRAME_TYPE;

/**
 *  @brief frame send by the NETWORK_Manager
**/
typedef struct __attribute__((__packed__))
{
    uint8_t type; /**< frame type eARNETWORK_FRAME_TYPE*/
    uint8_t ID; /**< identifier of the buffer sending the frame */
    uint32_t seq; /**< sequence number of the frame */
    uint32_t size; /**< size of the frame */
    uint8_t *dataPtr; /**< pointer on the data of the frame */
}ARNETWORK_Frame_t;


#endif /** _ARNETWORK_FRAME_H_ */
