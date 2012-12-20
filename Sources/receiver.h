/**
 *  @file receiver.h
 *  @brief manage the data received, used by libNetwork/network
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_RECEIVER_H_
#define _NETWORK_RECEIVER_H_

#include "ioBuffer.h"
#include "sender.h"
#include "buffer.h"

#define NETWORK_ID_ACK_OFFSET 1024

/**
 *  @brief receiver manager
 *  @warning before to be used, the receiver must be created through NETWORK_NewReceiver().
 *  @post after its using, the receiver must be deleted through NETWORK_DeleteReceiver().
**/
typedef struct
{
    network_buffer_t* pRecvBuffer; /**< Pointer on the data buffer used to store the data received*/
    network_sender_t* pSender; /**< Pointer on the sender which waits the acknowledgments*/
    network_ioBuffer_t** pptab_outputBuffer; /**< address of the table of pointers of output buffer*/
    int numOfOutputBuff; /**< Number of output buffer*/
    int socket; /**< receiving Socket. Must be accessed through NETWORK_ReceiverBind()*/
    
    int isAlive; /**< Indicator of aliving used for kill the thread calling the NETWORK_RunReceivingThread function (1 = alive | 0 = dead). Must be accessed through NETWORK_StopReceiver()*/
    
}network_receiver_t;

/**
 *  @brief Create a new receiver
 *  @warning This function allocate memory
 *  @post NETWORK_DeleteReceiver() must be called to delete the sender and free the memory allocated
 *  @param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 *  @param[in] numOfOutputBuff Number of output buffer
 *  @param[in] pptab_output address of the table of the pointers on the output buffers
 *  @return Pointer on the new receiver
 *  @see NETWORK_DeleteReceiver()
**/
network_receiver_t* NETWORK_NewReceiver( unsigned int recvBufferSize, unsigned int numOfOutputBuff,
                                           network_ioBuffer_t** pptab_output);

/**
 *  @brief Delete the Receiver
 *  @warning This function free memory
 *  @param ppReceiver address of the pointer on the Receiver to delete
 *  @see NETWORK_NewReceiver()
**/
void NETWORK_DeleteReceiver(network_receiver_t** ppReceiver);

/**
 *  @brief Manage the reception of the data on the Receiver' socket.
 *  @warning This function must be called by a specific thread.
 *  @warning At the end of this function the socket of the receiver is closed.
 *  @pre The socket of the receiver must be initialized through NETWORK_ReceiverBind().
 *  @post Before join the thread calling this function, NETWORK_StopReceiver() must be called.
 *  @note This function receives the data through NETWORK_ReceiverRead() and stores them in the output buffers according to their parameters.
 *  @param data thread datas of type network_Receive_t*
 *  @return NULL
 *  @see NETWORK_ReceiverBind()
 *  @see NETWORK_StopReceiver()
 *  @see NETWORK_ReceiverRead()
**/
void* NETWORK_RunReceivingThread(void* data);

/**
 *  @brief stop the reception
 *     @details Used to kill the thread calling NETWORK_RunReceivingThread().
 *     @param pReceiver pointer on the Receiver
 *     @see NETWORK_RunReceivingThread()
**/
void NETWORK_StopReceiver(network_receiver_t* pReceiver);

/**
 *  @brief return an acknowledgement to the sender
 *  @param pReceiver the pointer on the Receiver
 *  @param[in] id identifier of the command to acknowledged
 *  @param[in] seq sequence number of the command to acknowledged
 *  @see NETWORK_NewReceiver()
**/
void NETWORK_ReturnACK(network_receiver_t* pReceiver, int id, int seq);

/**
 *  @brief receiving data present on the socket
 *  @warning this function is blocking. the timeout set on the socket allows unblock this function. 
 *  @pre The socket of the receiver must be initialized through NETWORK_ReceiverBind().
 *  @param pReceiver the pointer on the Receiver
 *  @return size of the data read.
 *  @see NETWORK_ReceiverBind()
**/
int NETWORK_ReceiverRead(network_receiver_t* pReceiver);

/**
 *  @brief get the identifier of the output buffer storing the acknowledgment for an output buffer storing data acknowledged.
 *  @param[in] id identifier of the output buffer waiting an acknowledgment.
 *  @return identifier of the output buffer storing the acknowledgment.
**/
static inline int idOutputToIdAck( int id)
{
    return id + NETWORK_ID_ACK_OFFSET;
}

/**
 *  @brief get the identifier of the output buffer storing data acknowledged for an output buffer storing acknowledgments.
 *  @param[in] id identifier of the output buffer storing the acknowledgment.
 *  @return identifier of the output buffer waiting an acknowledgment.
**/
static inline int idAckToIdInput( int id)
{
    return id - NETWORK_ID_ACK_OFFSET;
}

/**
 *  @brief Bind the Receiver' socket in UDP to a port. the socket will be used to receive the data. 
 *  @param pReceiver the pointer on the Receiver
 *  @param[in] port port on which the data will be received.
 *  @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the function NETWORK_ReceiverRead().
 *  @return error equal to NETWORK_OK if the Bind if successful otherwise equal to 1.
 *  @see NETWORK_ReceiverBind()
**/
int NETWORK_ReceiverBind(network_receiver_t* pReceiver, unsigned short port, int timeoutSec);

#endif // _NETWORK_RECEIVER_H_

