/**
 * @file ARNETWORK_Sender.h
 * @brief manage the data sending, used by libARNetwork/manager and libARNetwork/receiver
 * @date 05/18/2012
 * @author maxime.maitre@parrot.com
 */

#ifndef _ARNETWORK_SENDER_PRIVATE_H_
#define _ARNETWORK_SENDER_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#include "ARNETWORK_IOBuffer.h"

#include <libARSAL/ARSAL_Time.h>

/**
 * Default minimum time between two ping requests
 * This avoid using too much bandwidth on fast networks
 */
#define ARNETWORK_SENDER_MINIMUM_TIME_BETWEEN_PINGS_MS (10)

/**
 * Ping timeout
 * If the ping was not acknowledged at this point, a new ping will be sent
 * And the last measure latency will be set to -1
 */
#define ARNETWORK_SENDER_PING_TIMEOUT_MS (1000)

/**
 * @brief sending manager
 * @warning before to be used the sender must be created through ARNETWORK_Sender_New()
 * @post after its using the sender must be deleted through ARNETWORK_Sender_Delete()
 */
typedef struct
{
    ARNETWORKAL_Manager_t *networkALManager;

    ARNETWORK_IOBuffer_t **inputBufferPtrArr; /**< address of the array of pointers of input buffer*/
    int numberOfInputBuff;
    ARNETWORK_IOBuffer_t **internalInputBufferPtrArr; /**< address of the array of pointers of internal input buffer*/
    int numberOfInternalInputBuff;
    ARNETWORK_IOBuffer_t **inputBufferPtrMap; /**< address of the array storing the inputBuffers by their identifier */

    ARSAL_Mutex_t nextSendMutex; /**< Mutex for the nextSendCond condition */
    ARSAL_Cond_t nextSendCond; /**< Condition to wait on to force synchronous send */

    int isAlive; /**< Indicator of aliving used for kill the thread calling the ARNETWORK_Sender_ThreadRun function (1 = alive | 0 = dead). Must be accessed through ARNETWORK_Sender_Stop()*/

    ARSAL_Mutex_t pingMutex; /**< Mutex to lock all ping-related values */
    struct timeval pingStartTime; /**< Start timestamp of the current running ping */
    int lastPingValue; /**< Latency, in ms, determined by the last ping */
    int isPingRunning; /**< Boolean-like. 1 if a ping is in progress, else 0 */
    int minTimeBetweenPings; /**< Minimum time to wait between pings. Negative value mean no ping */

    int minimumTimeBetweenSendsMs; /**< Minimum time to wait between network sends */

}ARNETWORK_Sender_t;

/**
 * @brief Create a new sender
 * @warning This function allocate memory
 * @post ARNETWORK_Sender_Delete() must be called to delete the sender and free the memory allocated
 * @param[in] ARNetworkAL Manager to push data and send data. Must not be NULL !!!
 * @param[in] numberOfInputBuffer Number of input buffer
 * @param[in] inputBufferPtrArr address of the array of the pointers on the input buffers
 * @param[in] inputBufferPtrMap address of the array storing the inputBuffers by their identifier
 * @param[in] pingDelayMs minimum time between pings. Negative value means no ping.
 * @return Pointer on the new sender
 * @see ARNETWORK_Sender_Delete()
 */
ARNETWORK_Sender_t* ARNETWORK_Sender_New (ARNETWORKAL_Manager_t *networkALManager, unsigned int numberOfInputBuffer, ARNETWORK_IOBuffer_t **inputBufferPtrArr, unsigned int numberOfInternalInputBuffer, ARNETWORK_IOBuffer_t **internalInputBufferPtrArr, ARNETWORK_IOBuffer_t **inputBufferPtrMap, int pingDelayMs);

/**
 * @brief Delete the sender
 * @warning This function free memory
 * @param senderPtrAddr address of the pointer on the Sender to delete
 * @see ARNETWORK_Sender_New()
 */
void ARNETWORK_Sender_Delete (ARNETWORK_Sender_t **senderPtrAddr);

/**
 * @brief Manage the sending of the data on the sender' socket
 * @warning This function must be called by a specific thread.
 * @warning At the end of this function the socket of the sender is closed.
 * @post Before join the thread calling this function, ARNETWORK_Sender_Stop() must be called.
 * @note This function sends the data present in the input buffers according to their parameters.
 * @param data thread datas of type ARNETWORK_Sender_t*
 * @return NULL
 * @see ARNETWORK_Sender_Stop()
 */
void* ARNETWORK_Sender_ThreadRun (void* data);

/**
 * @brief Process a buffer in the send loop
 * This function is called internally by the ARNETWORK_Sender_ThreadRun() function.
 * It should not be called anywhere else (not thread safe, not reentrant ...)
 * @param senderPtr the pointer on the Sender
 * @param buffer the buffer to process
 * @param hasWaited flag to indicate that a milisecond has passed since the last call for this buffer
 */
void ARNETWORK_Sender_ProcessBufferToSend (ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *buffer, int hasWaited);

/**
 * @brief Stop the sending
 * @details Used to kill the thread calling ARNETWORK_Sender_ThreadRun().
 * @param senderPtr the pointer on the Sender
 * @see ARNETWORK_Sender_ThreadRun()
 */
void ARNETWORK_Sender_Stop (ARNETWORK_Sender_t *senderPtr);

/**
 * @brief Signals to the sender that new data are available in a low latency buffer
 * @param senderPtr pointer on the Sender
 */
void ARNETWORK_Sender_SignalNewData (ARNETWORK_Sender_t *senderPtr);

/**
 * @brief Receive an acknowledgment fo a data.
 * @details Called by a libARNetwork/receiver to transmit an acknowledgment.
 * @param senderPtr the pointer on the Sender
 * @param[in] ID identifier of the command with ARNETWORKAL_FRAME_TYPE_ACK type received by the libARNetwork/receiver
 * @param[in] seqNumber sequence number of the acknowledgment
 * @return error equal to ARNETWORK_OK if the data has been correctly acknowledged otherwise equal to 1.
 */
eARNETWORK_ERROR ARNETWORK_Sender_AckReceived (ARNETWORK_Sender_t *senderPtr, int identifier, uint8_t seqNumber);

/**
 * @brief flush all IoBuffers of the Sender
 * @param senderPtr the pointer on the Sender
 * @return eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_Sender_Flush (ARNETWORK_Sender_t *senderPtr);

/**
 * @brief reset the Sender
 * @param senderPtr the pointer on the Sender
 */
void ARNETWORK_Sender_Reset (ARNETWORK_Sender_t *senderPtr);

/**
 * @brief Gets the estimated sender latency
 * @param senderPtr the pointer on the Sender
 * @return The estimated latency in ms, or a negative value in case of an error
 */
int ARNETWORK_Sender_GetPing (ARNETWORK_Sender_t *senderPtr);

/**
 * @brief Called by the Reader -> Signal that we got a reply to the ping
 *
 * The startTime parameter is a way to check that we are getting a
 * reply to the 'good' ping
 *
 * @param senderPtr the pointer on the Sender
 * @param startTime The start time contained into the ping packet.
 * @param endTime The timestamp of the reception of the ping reply
 */
void ARNETWORK_Sender_GotPingAck (ARNETWORK_Sender_t *senderPtr, struct timeval *startTime, struct timeval *endTime);

/**
 * @brief Send a ping reply (a pong)
 *
 * @param data The timestamp that was included in the ping request
 */
void ARNETWORK_Sender_SendPong (ARNETWORK_Sender_t *senderPtr, struct timeval *data);

#endif /** _ARNETWORK_SENDER_PRIVATE_H_ */
