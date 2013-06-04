/**
 *  @file ARNETWORK_Receiver.h
 *  @brief manage the data received
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
 */

#ifndef _ARNETWORK_RECEIVER_PRIVATE_H_
#define _ARNETWORK_RECEIVER_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>
#include "ARNETWORK_IOBuffer.h"
#include "ARNETWORK_Sender.h"
#include "ARNETWORK_Buffer.h"

/**
 *  @brief receiver manager
 *  @warning before to be used, the receiver must be created through ARNETWORK_Receiver_New().
 *  @post after its using, the receiver must be deleted through ARNETWORK_Receiver_Delete().
 */
typedef struct
{
	ARNETWORKAL_Manager_t *networkALManager;
    ARNETWORK_Sender_t *senderPtr; /**< Pointer on the sender which waits the acknowledgments*/
    ARNETWORK_IOBuffer_t **outputBufferPtrArr; /**< address of the array of pointers of output buffer*/
    int numberOfOutputBuff; /**< Number of output buffer*/
    ARNETWORK_IOBuffer_t **internalOutputBufferPtrArr; /**< address of the array of pointers of internal output buffer*/
    int numberOfInternalOutputBuff; /**< Number of internal output buffer*/
    ARNETWORK_IOBuffer_t** outputBufferPtrMap; /**< address of the array storing the outputBuffers by their identifier */

    uint8_t* readingPointer; /** head of reading on the RecvBuffer */

    int isAlive; /**< Indicator of aliving used for kill the thread calling the ARNETWORK_Receiver_ThreadRun function (1 = alive | 0 = dead). Must be accessed through ARNETWORK_Receiver_Stop()*/

}ARNETWORK_Receiver_t;

/**
 *  @brief Create a new receiver
 *  @warning This function allocate memory
 *  @post ARNETWORK_Receiver_Delete() must be called to delete the sender and free the memory allocated
 *  @param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 *  @param[in] numberOfOutputBuff Number of output buffer
 *  @param[in] outputBufferPtrArr address of the array of the pointers on the output buffers
 *  @param[in] outputBufferPtrMap address of the array storing the outputBuffers by their identifier
 *  @return Pointer on the new receiver
 *  @see ARNETWORK_Receiver_Delete()
 */
ARNETWORK_Receiver_t* ARNETWORK_Receiver_New(ARNETWORKAL_Manager_t *networkALManager, unsigned int numberOfOutputBuff, ARNETWORK_IOBuffer_t **outputBufferPtrArr, ARNETWORK_IOBuffer_t **outputBufferPtrMap);

/**
 *  @brief Delete the Receiver
 *  @warning This function free memory
 *  @param receiverPtrAddr address of the pointer on the Receiver to delete
 *  @see ARNETWORK_Receiver_New()
 */
void ARNETWORK_Receiver_Delete(ARNETWORK_Receiver_t **receiverPtrAddr);

/**
 *  @brief Manage the reception of the data on the Receiver' socket.
 *  @warning This function must be called by a specific thread.
 *  @warning At the end of this function the socket of the receiver is closed.
 *  @post Before join the thread calling this function, ARNETWORK_Receiver_Stop() must be called.
 *  @note This function receives the data through ARNETWORKAL_Manager_Receiving_Callback_t and stores them in the output buffers according to their parameters.
 *  @param data thread datas of type ARNETWORK_Receive_t*
 *  @return NULL
 *  @see ARNETWORK_Receiver_Stop()
 */
void* ARNETWORK_Receiver_ThreadRun(void *data);

/**
 *  @brief stop the reception
 *  @details Used to kill the thread calling ARNETWORK_Receiver_ThreadRun().
 *  @param receiverPtr pointer on the Receiver
 *  @see ARNETWORK_Receiver_ThreadRun()
 */
void ARNETWORK_Receiver_Stop(ARNETWORK_Receiver_t *receiverPtr);

/**
 *  @brief return an acknowledgement to the sender
 *  @param receiverPtr the pointer on the Receiver
 *  @param[in] ID identifier of the command to acknowledged
 *  @param[in] seq sequence number of the command to acknowledged
 *  @return eARNETWORK_ERROR
 *  @see ARNETWORK_Receiver_New()
 */
eARNETWORK_ERROR ARNETWORK_Receiver_ReturnACK(ARNETWORK_Receiver_t *receiverPtr, int ID, uint32_t seq);

#endif /** _ARNETWORK_RECEIVER_PRIVATE_H_ */
