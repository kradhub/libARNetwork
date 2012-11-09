/**
 *	@file receiver.h
 *  @brief manage the data received, used by libNetwork::network
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/buffer.h>

/**
 *  @brief receiver manager
 * 	@warning before to be used, the receiver must be created through newReceiver().
 * 	@post after its using, the receiver must be deleted through deleteReceiver().
**/
typedef struct network_Receiver_t
{
	network_buffer_t* pRecvBuffer; /**< Pointer on the data buffer used to store the data received*/
	network_Sender_t* pSender; /**< Pointer on the sender which waits the acknowledgments*/
	network_inOutBuffer_t** pptab_outputBuffer; /**< address of the table of pointers of output buffer*/
	int numOfOutputBuff; /**< Number of output buffer*/
	int socket; /**< receiving Socket. Must be accessed through receiverBind()*/
	
	int isAlive; /**< Indicator of aliving used for kill the thread calling the runReceivingThread function (1 = alive | 0 = dead). Must be accessed through stopReceiver()*/
	
}network_Receiver_t;


/**
 *  @brief Create a new receiver
 * 	@warning This function allocate memory
 * 	@post deleteReceiver() must be called to delete the sender and free the memory allocated
 * 	@param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 * 	@param[in] numOfOutputBuff Number of output buffer
 * 	@param[in] pptab_output address of the table of the pointers on the output buffers
 * 	@return Pointer on the new receiver
 * 	@see deleteReceiver()
**/
network_Receiver_t* newReceiver(	unsigned int recvBufferSize, unsigned int numOfOutputBuff,
									network_inOutBuffer_t** pptab_output);

/**
 *  @brief Delete the Receiver
 * 	@warning This function free memory
 * 	@param ppReceiver address of the pointer on the Receiver to delete
 *	@see newReceiver()
**/
void deleteReceiver(network_Receiver_t** ppReceiver);

/**
 *  @brief Manage the reception of the data on the Receiver' scoket.
 * 	@warning This function must be called by a specific thread.
 * 	@warning At the end of this function the socket of the receiver is closed.
 * 	@pre The socket of the receiver must be initialized through receiverBind().
 * 	@post Before join the thread calling this function, stopReceiver() must be called.
 * 	@note This function receives the data through receiverRead() and stores them in the output buffers according to their parameters.
 * 	@param data thread datas of type network_Receive_t*
 * 	@see receiverBind()
 * 	@see stopReceiver()
 * 	@see receiverRead()
**/
void* runReceivingThread(void* data);

/**
 *  @brief stop the reception
 * 	@details Used to kill the thread calling runReceivingThread().
 * 	@param pReceiver pointer on the Receiver
 * 	@see runReceivingThread()
**/
void stopReceiver(network_Receiver_t* pReceiver);

/**
 *  @brief return an acknowledgement to the sender
 * 	@param pReceiver the pointer on the Receiver
 * 	@param[in] id identifier of the command to acknowledged
 * 	@param[in] seq sequence number of the command to acknowledged
 *	@see newReceiver()
**/
void returnASK(network_Receiver_t* pReceiver, int id, int seq);

/**
 *  @brief receiving data present on the socket
 * 	@warning this function is blocking. the timeout set on the socket allows unblock this function. 
 * 	@pre The socket of the receiver must be initialized through receiverBind().
 * 	@param pReceiver the pointer on the Receiver
 * 	@return size of the data read.
 *	@see receiverBind()
**/
int receiverRead(network_Receiver_t* pReceiver);

/**
 *  @brief get the identifier of the output buffer storing the acknowledgment for an output buffer storing data acknowledged.
 * 	@param[in] id identifier of the output buffer waiting an acknowledgment.
 * 	@return identifier of the output buffer storing the acknowledgment.
**/
extern inline int idOutputToIdAck( int id)
{
	return id + 1000;
}

/**
 *  @brief get the identifier of the output buffer storing data acknowledged for an output buffer storing acknowledgments.
 * 	@param[in] id identifier of the output buffer storing the acknowledgment.
 * 	@return identifier of the output buffer waiting an acknowledgment.
**/
extern inline int idAckToIdInput( int id)
{
	return id - 1000;
}

/**
 *  @brief Bind the Receiver' socket in UDP to a port. the socket will be used to receive the data. 
 * 	@param pReceiver the pointer on the Receiver
 * 	@param[in] port port on which the data will be received.
 * 	@param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the function receiverRead().
 * 	@return error equal to 0 if the Bind if successful otherwise equal to 1.
 *	@see receiverBind()
**/
int receiverBind(network_Receiver_t* pReceiver, unsigned short port, int timeoutSec);

#endif // _RECEIVER_H_

