/**
 *	@file sender.h
 *  @brief manage the data sending, used by libNetwork::network and libNetwork::receiver
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _SENDER_H_
#define _SENDER_H_

#include <libNetwork/inOutBuffer.h>
#include <libNetwork/buffer.h>

/**
 *  @brief sending manager
 * 	@warning before to be used the sender must be created through newSender()
 * 	@post after its using the sender must be deleted through deleteSender()
**/
typedef struct network_Sender_t
{
	network_buffer_t* pSendingBuffer; /**< Pointer on the data buffer to send*/
	
	network_inOutBuffer_t** pptab_inputBuffer; /**< address of the table of pointers of input buffer*/
	int numOfInputBuff; 
	int socket; /**< sending Socket. Must be accessed through senderConnection()*/
	
	int isAlive; /**< Indicator of aliving used for kill the thread calling the runSendingThread function (1 = alive | 0 = dead). Must be accessed through stopSender()*/
}network_Sender_t;


/**
 *  @brief Create a new sender
 * 	@warning This function allocate memory
 *	@post deleteSender() must be called to delete the sender and free the memory allocated
 * 	@param[in] sendingBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 * 	@param[in] numOfInputBuff Number of input buffer
 * 	@param[in] ppTab_input address of the table of the pointers on the input buffers
 * 	@return Pointer on the new sender
 * 	@see deleteSender()
**/
network_Sender_t* newSender(unsigned int sendingBufferSize, unsigned int numOfInputBuff,
								network_inOutBuffer_t** ppTab_input);

/**
 *  @brief Delete the sender
 * 	@warning This function free memory
 * 	@param ppSender address of the pointer on the Sender to delete
 *	@see newSender()
**/
void deleteSender(network_Sender_t** ppSender);

/**
 *  @brief Manage the sending of the data on the sender' scoket 
 * 	@warning This function must be called by a specific thread.
 * 	@warning At the end of this function the socket of the sender is closed.
 * 	@pre The socket of the sender must be initialized through senderConnection().
 * 	@post Before join the thread calling this function, stopSender() must be called.
 * 	@details This function sends the data present in the input buffers according to their parameters.
 * 	@param data thread datas of type network_Sender_t*
 * 	@see senderConnection()
 * 	@see stopSender()
**/
void* runSendingThread(void* data);

/**
 *  @brief Stop the sending
 * 	@details Used to kill the thread calling runSendingThread().
 * 	@param pSender the pointer on the Sender
 * 	@see runSendingThread()
**/
void stopSender(network_Sender_t* pSender);

/**
 *  @brief Receive an acknowledgment fo a data.
 * 	@details Called by a libNetwork::receiver to transmit an acknowledgment.
 * 	@param pSender the pointer on the Sender
 * 	@param[in] id identifier of the command with CMD_TYPE_ACK type received by the libNetwork::receiver
 *	@param[in] seqNum sequence number of the acknowledgment
 * 	@return error equal to 0 if the data has been correctly acknowledged otherwise equal to 1.
**/
int senderAckReceived(network_Sender_t* pSender, int id, int seqNum);

/**
 *  @brief Connect the socket in UDP to a port of an address. the socket will be used to send the data.
 * 	@warning Must be called before the start of the thread running runSendingThread().
 * 	@param pSender the pointer on the Sender
 * 	@param[in] addr address of connection at which the data will be sent.
 *	@param[in] port port on which the data will be sent.
 * 	@return error equal to 0 if the connection if successful otherwise equal to 1.
**/
int senderConnection(network_Sender_t* pSender,const char* addr, int port);

#endif // _SENDER_H_

