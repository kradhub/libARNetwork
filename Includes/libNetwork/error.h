/**
 *	@file error.h
 *  @brief network errors known.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_ERROR_H_
#define _NETWORK_ERROR_H_

/**
 *  @brief libNetwork errors known.
**/
typedef enum
{
	NETWORK_OK = 0, /**< no error */
    NETWORK_ERROR = -1000, /**< error unknown */ 
    NETWORK_ERROR_ALLOC, /**< allocation error */
    NETWORK_ERROR_BAD_PARAMETER, /**< parameter incorrect */ 
    NETWORK_ERROR_ID_UNKNOWN, /**< IoBuffer identifier unknown */ 
    NETWORK_ERROR_BUFFER_SIZE, /**< free space of the buffer is insuffisante */
    NETWORK_ERROR_BUFFER_EMPTY, /**< try to read a buffer empty */
    NETWORK_MANAGER_ERROR = -2000, /**< manager error unknown */ 
    NETWORK_MANAGER_ERROR_NEW_IOBUFFER, /**< error during the IoBuffer creation */ 
    NETWORK_MANAGER_ERROR_NEW_SENDER, /**< error during the Sender creation */ 
    NETWORK_MANAGER_ERROR_NEW_RECEIVER, /**< error during the Receiver creation */
    NETWORK_ERROR_NEW_BUFFER, /**< error during the buffer creation */
    NETWORK_ERROR_NEW_RINGBUFFER, /**< error during the RingBuffer creation */
    NETWORK_IOBUFFER_ERROR = -3000, /**< IoBuffer error unknown */ 
    NETWORK_IOBUFFER_ERROR_BAD_ACK,  /**< the sequence number isn't same as that waiting */
	
} eNETWORK_Error;

#endif // _NETWORK_ERROR_H_

