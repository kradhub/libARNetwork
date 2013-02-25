/**
 *  @file ARNETWORK_Error.h
 *  @brief libARNetwork errors known.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_ERROR_H_
#define _ARNETWORK_ERROR_H_

/**
 *  @brief libARNetwork errors known.
**/
typedef enum
{
    ARNETWORK_OK = 0, /**< no error */
    ARNETWORK_ERROR = -1000, /**< error unknown */ 
    ARNETWORK_ERROR_ALLOC, /**< allocation error */
    ARNETWORK_ERROR_BAD_PARAMETER, /**< parameter incorrect */ 
    ARNETWORK_ERROR_ID_UNKNOWN, /**< IOBuffer identifier unknown */ 
    ARNETWORK_ERROR_BUFFER_SIZE, /**< free space of the buffer is insufficient */
    ARNETWORK_ERROR_BUFFER_EMPTY, /**< try to read a buffer empty */
    ARNETWORK_ERROR_SEMAPHORE, /**< error during the using of a semaphore */
    ARNETWORK_ERROR_MANAGER = -2000, /**< manager error unknown */ 
    ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER, /**< error during the IOBuffer creation */ 
    ARNETWORK_ERROR_MANAGER_NEW_SENDER, /**< error during the Sender creation */ 
    ARNETWORK_ERROR_MANAGER_NEW_RECEIVER, /**< error during the Receiver creation */
    ARNETWORK_ERROR_NEW_BUFFER, /**< error during the buffer creation */
    ARNETWORK_ERROR_NEW_RINGBUFFER, /**< error during the RingBuffer creation */
    ARNETWORK_ERROR_IOBUFFER = -3000, /**< IOBuffer error unknown */ 
    ARNETWORK_ERROR_IOBUFFER_BAD_ACK,  /**< the sequence number isn't same as that waiting */
    ARNETWORK_ERROR_SOCKET = -4000,  /**< socket error unknown */
    ARNETWORK_ERROR_SOCKET_PERMISSION_DENIED, /**< Permission denied */
    ARNETWORK_ERROR_RECEIVER = -5000, /**< receiver error unknown */
    ARNETWORK_ERROR_RECEIVER_BUFFER_END, /**< end of the Receiver Buffer */
    ARNETWORK_ERROR_RECEIVER_BAD_FRAME, /**< error bad frame */
    
} eARNETWORK_ERROR;

#endif /** _ARNETWORK_ERROR_H_ */

