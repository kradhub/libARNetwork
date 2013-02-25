/**
 *  @file ARNETWORK_Sender.h
 *  @brief manage the data sending, used by libARNetwork/manager and libARNetwork/receiver
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_SENDER_PRIVATE_H_
#define _ARNETWORK_SENDER_PRIVATE_H_

#include "ARNETWORK_IOBuffer.h"
#include "ARNETWORK_Buffer.h"

/**
 *  @brief sending manager
 *  @warning before to be used the sender must be created through ARNETWORK_Sender_New()
 *  @post after its using the sender must be deleted through ARNETWORK_Sender_Delete()
**/
typedef struct
{
    ARNETWORK_Buffer_t *sendingBufferPtr; /**< Pointer on the data buffer to send*/
    
    ARNETWORK_IOBuffer_t **inputBufferPtrArr; /**< address of the array of pointers of input buffer*/
    int numberOfInputBuff; 
    int socket; /**< sending Socket. Must be accessed through ARNETWORK_Sender_Connect()*/
    ARNETWORK_IOBuffer_t **inputBufferPtrMap; /**< address of the array storing the inputBuffers by their identifier */
    
    int isAlive; /**< Indicator of aliving used for kill the thread calling the ARNETWORK_Sender_ThreadRun function (1 = alive | 0 = dead). Must be accessed through ARNETWORK_Sender_Stop()*/
    int seq; /** sequence number of sending */
}ARNETWORK_Sender_t;

/**
 *  @brief Create a new sender
 *  @warning This function allocate memory
 *  @post ARNETWORK_Sender_Delete() must be called to delete the sender and free the memory allocated
 *  @param[in] sendingBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 *  @param[in] numberOfInputBuffer Number of input buffer
 *  @param[in] inputBufferPtrArr address of the array of the pointers on the input buffers
 *  @param[in] inputBufferPtrMap address of the array storing the inputBuffers by their identifier
 *  @return Pointer on the new sender
 *  @see ARNETWORK_Sender_Delete()
**/
ARNETWORK_Sender_t* ARNETWORK_Sender_New(unsigned int sendingBufferSize, unsigned int numberOfInputBuffer, ARNETWORK_IOBuffer_t **inputBufferPtrArr, ARNETWORK_IOBuffer_t **inputBufferPtrMap);

/**
 *  @brief Delete the sender
 *  @warning This function free memory
 *  @param senderPtrAddr address of the pointer on the Sender to delete
 *  @see ARNETWORK_Sender_New()
**/
void ARNETWORK_Sender_Delete(ARNETWORK_Sender_t **senderPtrAddr);

/**
 *  @brief Manage the sending of the data on the sender' socket 
 *  @warning This function must be called by a specific thread.
 *  @warning At the end of this function the socket of the sender is closed.
 *  @pre The socket of the sender must be initialized through ARNETWORK_Sender_Connect().
 *  @post Before join the thread calling this function, ARNETWORK_Sender_Stop() must be called.
 *  @note This function sends the data present in the input buffers according to their parameters.
 *  @param data thread datas of type ARNETWORK_Sender_t*
 *  @return NULL
 *  @see ARNETWORK_Sender_Connect()
 *  @see ARNETWORK_Sender_Stop()
**/
void* ARNETWORK_Sender_ThreadRun(void* data);

/**
 *  @brief Stop the sending
 *  @details Used to kill the thread calling ARNETWORK_Sender_ThreadRun().
 *  @param senderPtr the pointer on the Sender
 *  @see ARNETWORK_Sender_ThreadRun()
**/
void ARNETWORK_Sender_Stop(ARNETWORK_Sender_t *senderPtr);

/**
 *  @brief Receive an acknowledgment fo a data.
 *  @details Called by a libARNetwork/receiver to transmit an acknowledgment.
 *  @param senderPtr the pointer on the Sender
 *  @param[in] ID identifier of the command with ARNETWORK_FRAME_TYPE_ACK type received by the libARNetwork/receiver
 *  @param[in] seqNumber sequence number of the acknowledgment
 *  @return error equal to ARNETWORK_OK if the data has been correctly acknowledged otherwise equal to 1.
**/
eARNETWORK_ERROR ARNETWORK_Sender_AckReceived(ARNETWORK_Sender_t *senderPtr, int ID, int seqNumber);

/**
 *  @brief Connect the socket in UDP to a port of an address. the socket will be used to send the data.
 *  @warning Must be called before the start of the thread running ARNETWORK_Sender_ThreadRun().
 *  @param senderPtr the pointer on the Sender
 *  @param[in] addr address of connection at which the data will be sent.
 *  @param[in] port port on which the data will be sent.
 *  @return error equal to ARNETWORK_OK if the connection if successful otherwise equal to 1.
**/
eARNETWORK_ERROR ARNETWORK_Sender_Connect(ARNETWORK_Sender_t *senderPtr, const char *addr, int port);

/**
 *  @brief flush all IoBuffers of the Sender
 *  @param senderPtr the pointer on the Sender
 *  @return eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_Sender_Flush(ARNETWORK_Sender_t *senderPtr);

/**
 *  @brief reset the Sender
 *  @param senderPtr the pointer on the Sender
**/
void ARNETWORK_Sender_Reset(ARNETWORK_Sender_t *senderPtr);

#endif /** _ARNETWORK_SENDER_PRIVATE_H_ */

