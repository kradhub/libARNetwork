/**
 *	@file sender.h
 *  @brief manage the data sending
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _SENDER_H_
#define _SENDER_H_

#include <libNetwork/inOutBuffer.h>
#include <libNetwork/buffer.h>

// static :


//Enumerations :


//Structures :
/**
 *  @brief properties of the sender
**/
typedef struct network_Sender_t network_Sender_t;
struct network_Sender_t
{

	network_buffer_t* pSendingBuffer;
	int 	sendingBufferSize;
	char* 	pSendingBuffercursor;//private
	char* 	pSendingBufferEnd;//private
	
	network_inOutBuffer_t** pptab_inputBuffer;
	int inputBufferNum;
	
	// !! sal_mutex_t mutex; //??? !!!!
	int isAlive;
	int sleepTime;
	
	int socket;
};


/**
 *  @brief Create a sender
 *	@post Call deleteSender()
 * 	@return Pointer on the new sender
**/
network_Sender_t* newSender(unsigned int sendingBufferSize, unsigned int inputBufferNum,
								network_inOutBuffer_t** ppTab_input);

/**
 *  @brief Delete the sender
 * 	@param ppSender address of the pointer on the Sender
 *	@see newSender()
**/
void deleteSender(network_Sender_t** ppSender);

/**
 *  @brief manage the communication between the drone and the application
 * 	Must be called by a specific thread 
 * 	@param data thread datas of type network_Sender_t*
**/
void* runSendingThread(void* data);


// /**
 //*  @brief start the sending
 //* 	@param pBuffsend the pointer on the Sender
 //*	@pre the thread calling runSendingThread() must be created
 //* 	@see runSendingThread()
// **/
//void startSender(network_Sender_t* pSender);

/**
 *  @brief stop the sending
 * 	@param pSender the pointer on the Sender
 *	@pre the thread calling runSendingThread() must be created
 * 	@see runSendingThread()
**/
void stopSender(network_Sender_t* pSender);

/**
 *  @brief received
 * 	@param pSender the pointer on the Sender
 * 	@param id identifier of the command CMD_TYPE_ACK received
 *	@param seqNum sequence number of the acknowledge
**/
void senderAckReceived(network_Sender_t* pSender, int id, int seqNum);

int senderConnection(network_Sender_t* pSender,const char* addr, int port);

#endif // _SENDER_H_

