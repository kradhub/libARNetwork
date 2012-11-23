/**
 *	@file NETWORK_Common.h
 *  @brief define the network protocol
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_COMMON_H_
#define _NETWORK_COMMON_H_

#include <inttypes.h>

/**
 *  @brief command type know by the network
**/
typedef enum eAR_CMD_TYPE
{
    CMD_TYPE_ACK, /**< acknowledgment type*/
    CMD_TYPE_DATA, /**< data type*/
    CMD_TYPE_DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
    CMD_TYPE_KEEP_ALIVE, /**< keep alive type*/
    CMD_TYPE_DEFAULT /**< not known type*/
}eAR_CMD_TYPE;

/**
 *  @brief indexs of the data contents in a command
**/
typedef enum eAR_CMD_INDEX
{
	AR_CMD_INDEX_TYPE = 0, /**< index of the command type*/
	AR_CMD_INDEX_ID = 4, /**< index of the identifier of the buffer where was stored the command*/
	AR_CMD_INDEX_SEQ = 8, /**< index of the sequence number of the command*/
	AR_CMD_INDEX_SIZE = 12, /**< index of the size of the command*/
	AR_CMD_INDEX_DATA = 16, /**< index of the command data*/
	AR_CMD_HEADER_SIZE = 16 /**< size of the command header*/
}eAR_CMD_INDEX;

/**
 *  @brief 
**/
typedef struct AR_CMD
{
	eAR_CMD_TYPE type; /**< command type */
    uint32_t id; /**< identifier of the buffer sending the command */
    uint32_t seq; /**< sequence number of the command */
    uint32_t size; /**< size of the command */
    //void* data;  // !!! ???
}AR_CMD;

/**
 *  @brief 
**/
typedef union UNION_CMD 
{
	AR_CMD*		pCmd;
    uint8_t* 	pTabUint8;
}UNION_CMD;

#endif // _NETWORK_COMMON_H_
