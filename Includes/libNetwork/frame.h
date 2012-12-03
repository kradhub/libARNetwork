/**
 *	@file frame.h
 *  @brief define the network frame protocol
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_FRAME_H_
#define _NETWORK_FRAME_H_

#include <inttypes.h>

/**
 *  @brief command type know by the network
**/
typedef enum eNETWOK_Frame_Type
{
    network_frame_t_TYPE_UNINITIALIZED = 0, /**< not known type*/
    network_frame_t_TYPE_ACK, /**< acknowledgment type*/
    network_frame_t_TYPE_DATA, /**< data type*/
    network_frame_t_TYPE_DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
    network_frame_t_TYPE_KEEP_ALIVE /**< keep alive type*/
    
}eNETWOK_Frame_Type;

/**
 *  @brief frame send by the NETWORK_Manager
**/
typedef struct network_frame_t
{
	eNETWOK_Frame_Type type; /**< frame type */
    uint32_t id; /**< identifier of the buffer sending the frame */
    uint32_t seq; /**< sequence number of the frame */
    uint32_t size; /**< size of the frame */
    uint8_t  data;  /**< data of the frame */
}network_frame_t;


#endif // _NETWORK_FRAME_H_