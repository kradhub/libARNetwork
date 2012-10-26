/**
 *	@file common.h
 *  @brief simul common file
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <inttypes.h>

// static :

//Enumerations :


/**
 *  @brief sending mode
**/
typedef enum eSEND_MODE eSEND_MODE;
enum eSEND_MODE
{
	SEND_MODE_HIGHT_PRIRITY, /**< ... */
	SEND_MODE_LOW_LATENCY, /**< ... */
	SEND_MODE_LOSS_LESS, /**< ... */
	DEFAULT /**< ... */
};

/**
 *  @brief Acknowledged command
**/
typedef enum eAR_CMD_TYPE
{
    CMD_TYPE_ACK,
    CMD_TYPE_DATA,
    CMD_TYPE_DATA_WITH_ACK,
    CMD_TYPE_DEFAULT
}eAR_CMD_TYPE;


/**
 *  @brief  command
**/
//typedef struct AR_CMD_HEADER
//{
//    eAR_CMD_TYPE type;
//    int id;
//    int size;
//}AR_CMD_HEADER;


/**
 *  @brief 
**/
typedef struct AR_CMD 
{
	eAR_CMD_TYPE type;
    int id;
    int seq;
    int size;
    void* data;  // !!! ???
}AR_CMD;

/**
 *  @brief 
**/
typedef union UNION_CMD 
{
	AR_CMD*		pCmd;
    uint8_t* 	pTabUint8;
}UNION_CMD;

#endif // _COMMON_H_

