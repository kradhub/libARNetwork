/**
 *	@file sender.h
 *  @brief manage the data sending, used by libNetwork/manager and libNetwork/receiver
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_SENDER_H_
#define _NETWORK_SENDER_H_

#include "ioBuffer.h"
#include "buffer.h"

/**
 *  @brief sending manager
 * 	@warning before to be used the sender must be created through NETWORK_NewSender()
 * 	@post after its using the sender must be deleted through NETWORK_DeleteSender()
**/
typedef struct network_sender_t
{
	network_buffer_t* pSendingBuffer; /**< Pointer on the data buffer to send*/
	
	network_ioBuffer_t** pptab_inputBuffer; /**< address of the table of pointers of input buffer*/
	int numOfInputBuff; 
	int socket; /**< sending Socket. Must be accessed through NETWORK_SenderConnection()*/
	
	int isAlive; /**< Indicator of aliving used for kill the thread calling the NETWORK_RunSendingThread function (1 = alive | 0 = dead). Must be accessed through NETWORK_StopSender()*/
    int seq; /** sequence number of sending */
}network_sender_t;

/**
 *  @brief Create a new sender
 * 	@warning This function allocate memory
 *	@post NETWORK_DeleteSender() must be called to delete the sender and free the memory allocated
 * 	@param[in] sendingBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 * 	@param[in] numOfInputBuff Number of input buffer
 * 	@param[in] ppTab_input address of the table of the pointers on the input buffers
 * 	@return Pointer on the new sender
 * 	@see NETWORK_DeleteSender()
**/
network_sender_t* NETWORK_NewSender(unsigned int sendingBufferSize, unsigned int numOfInputBuff,
								network_ioBuffer_t** ppTab_input);

/**
 *  @brief Delete the sender
 * 	@warning This function free memory
 * 	@param ppSender address of the pointer on the Sender to delete
 *	@see NETWORK_NewSender()
**/
void NETWORK_DeleteSender(network_sender_t** ppSender);

/**
 *  @brief Manage the sending of the data on the sender' socket 
 * 	@warning This function must be called by a specific thread.
 * 	@warning At the end of this function the socket of the sender is closed.
 * 	@pre The socket of the sender must be initialized through NETWORK_SenderConnection().
 * 	@post Before join the thread calling this function, NETWORK_StopSender() must be called.
 * 	@note This function sends the data present in the input buffers according to their parameters.
 * 	@param data thread datas of type network_sender_t*
 *  @return NULL
 * 	@see NETWORK_SenderConnection()
 * 	@see NETWORK_StopSender()
**/
void* NETWORK_RunSendingThread(void* data);

/**
 *  @brief Stop the sending
 * 	@details Used to kill the thread calling NETWORK_RunSendingThread().
 * 	@param pSender the pointer on the Sender
 * 	@see NETWORK_RunSendingThread()
**/
void NETWORK_StopSender(network_sender_t* pSender);

/**
 *  @brief Receive an acknowledgment fo a data.
 * 	@details Called by a libNetwork/receiver to transmit an acknowledgment.
 * 	@param pSender the pointer on the Sender
 * 	@param[in] id identifier of the command with NETWORK_FRAME_TYPE_ACK type received by the libNetwork/receiver
 *	@param[in] seqNum sequence number of the acknowledgment
 * 	@return error equal to NETWORK_OK if the data has been correctly acknowledged otherwise equal to 1.
**/
eNETWORK_Error NETWORK_SenderAckReceived(network_sender_t* pSender, int id, int seqNum);

/**
 *  @brief Connect the socket in UDP to a port of an address. the socket will be used to send the data.
 * 	@warning Must be called before the start of the thread running NETWORK_RunSendingThread().
 * 	@param pSender the pointer on the Sender
 * 	@param[in] addr address of connection at which the data will be sent.
 *	@param[in] port port on which the data will be sent.
 * 	@return error equal to NETWORK_OK if the connection if successful otherwise equal to 1.
**/
int NETWORK_SenderConnection(network_sender_t* pSender,const char* addr, int port);

/**
 *  @brief flush all IoBuffers of the Sender
 * 	@param pSender the pointer on the Sender
**/
void NETWORK_SenderFlush(network_sender_t* pSender);

/**
 *  @brief reset the Sender
 * 	@param pSender the pointer on the Sender
**/
void NETWORK_SenderReset(network_sender_t* pSender);

#endif // _NETWORK_SENDER_H_

