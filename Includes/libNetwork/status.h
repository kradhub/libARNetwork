/**
 *  @file status.h
 *  @brief network status known.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_STATUS_H_
#define _NETWORK_STATUS_H_

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
    NETWORK_ERROR_BUFFER_SIZE, /**< free space of the buffer is insufficient */
    NETWORK_ERROR_BUFFER_EMPTY, /**< try to read a buffer empty */
    NETWORK_ERROR_SEMAPHORE, /**< error during the using of a semaphore */
    NETWORK_MANAGER_ERROR = -2000, /**< manager error unknown */ 
    NETWORK_MANAGER_ERROR_NEW_IOBUFFER, /**< error during the IoBuffer creation */ 
    NETWORK_MANAGER_ERROR_NEW_SENDER, /**< error during the Sender creation */ 
    NETWORK_MANAGER_ERROR_NEW_RECEIVER, /**< error during the Receiver creation */
    NETWORK_ERROR_NEW_BUFFER, /**< error during the buffer creation */
    NETWORK_ERROR_NEW_RINGBUFFER, /**< error during the RingBuffer creation */
    NETWORK_IOBUFFER_ERROR = -3000, /**< IoBuffer error unknown */ 
    NETWORK_IOBUFFER_ERROR_BAD_ACK,  /**< the sequence number isn't same as that waiting */
    NETWORK_SCOCKET_ERROR = -4000,  /**< socket error unknown */
    NETWORK_SCOCKET_ERROR_PERMISSION_DENIED, /**< Permission denied */
    
} eNETWORK_Error;

/**
 *  @brief status return by the callback.
**/
typedef enum
{
    NETWORK_CALLBACK_RETURN_DEFECT = 0, /**<   */
    NETWORK_CALLBACK_RETURN_RETRY, /**< reset the number of retry  */
    NETWORK_CALLBACK_RETURN_DATA_POP, /**< pop the data */
    NETWORK_CALLBACK_RETURN_FLUSH /**< FLUSH all input buffers*/
    
} eNETWORK_CALLBACK_RETURN;

/**
 *  @brief status sent by the callback.
**/
typedef enum
{
    NETWORK_CALLBACK_STATUS_SENT = 0, /**< data sent  */
    NETWORK_CALLBACK_STATUS_SENT_WITH_ACK, /**< data acknowledged sent  */
    NETWORK_CALLBACK_STATUS_TIMEOUT, /**< timeout occurred, data not received */
    NETWORK_CALLBACK_STATUS_FREE /**< free the data not sent*/
    
} eNETWORK_CALLBACK_STATUS;

#endif // _NETWORK_STATUS_H_

