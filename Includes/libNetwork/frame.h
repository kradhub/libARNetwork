/**
 *  @file frame.h
 *  @brief define the network frame protocol
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_FRAME_H_
#define _NETWORK_FRAME_H_

#include <inttypes.h>

/**
 *  @brief type of frame send by the NETWORK_Manager
**/
typedef enum eNETWORK_Frame_Type
{
    NETWORK_FRAME_TYPE_UNINITIALIZED = 0, /**< not known type*/
    NETWORK_FRAME_TYPE_ACK, /**< acknowledgment type*/
    NETWORK_FRAME_TYPE_DATA, /**< data type*/
    NETWORK_FRAME_TYPE_DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
    NETWORK_FRAME_TYPE_KEEP_ALIVE /**< keep alive type*/
    
}eNETWORK_Frame_Type;

/**
 *  @brief frame send by the NETWORK_Manager
**/
typedef struct __attribute__((__packed__)) network_frame_t
{
    uint8_t type; /**< frame type eNETWORK_Frame_Type*/
    uint8_t id; /**< identifier of the buffer sending the frame */
    uint32_t seq; /**< sequence number of the frame */
    uint32_t size; /**< size of the frame */
    uint8_t*  pData;  /**< pointer on the data of the frame */
}network_frame_t;


#endif /** _NETWORK_FRAME_H_ */
