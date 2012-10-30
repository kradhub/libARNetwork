/**
 *	@file sender.h
 *  @brief manage the data sending
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _SENDER_H_
#define _SENDER_H_

#include <libNetWork/inOutBuffer.h>
#include <libNetWork/singleBuffer.h>

// static :


//Enumerations :


//Structures :
/**
 *  @brief properties of the sender
**/
typedef struct netWork_Sender_t netWork_Sender_t;
struct netWork_Sender_t
{

	netWork_buffer_t* pSendingBuffer;
	int 	sendingBufferSize;
	char* 	pSendingBuffercursor;//private
	char* 	pSendingBufferEnd;//private
	
	netWork_inOutBuffer_t** pptab_inputBuffer;
	int inputBufferNum;
	
	// !! sal_mutex_t mutex; //??? !!!!
	int isAlive;
	int sleepTime;
	
	int fd;//fifopipe temp
};


/**
 *  @brief Create a sender
 *	@post Call deleteSender()
 * 	@return Pointer on the new sender
**/
//netWork_Sender_t* newSender(unsigned int sendingBufferSize, unsigned int inputBufferNum, ...);
netWork_Sender_t* newSender(unsigned int sendingBufferSize, unsigned int inputBufferNum,
								netWork_inOutBuffer_t** ppTab_input);

/**
 *  @brief Delete the sender
 * 	@param ppSender address of the pointer on the Sender
 *	@see newSender()
**/
void deleteSender(netWork_Sender_t** ppSender);

/**
 *  @brief manage the communication between the drone and the application
 * 	Must be called by a specific thread 
 * 	@param data thread datas of type netWork_Sender_t*
**/
void* runSendingThread(void* data);


// /**
 //*  @brief start the sending
 //* 	@param pBuffsend the pointer on the Sender
 //*	@pre the thread calling runSendingThread() must be created
 //* 	@see runSendingThread()
// **/
//void startSender(netWork_Sender_t* pSender);

/**
 *  @brief stop the sending
 * 	@param pSender the pointer on the Sender
 *	@pre the thread calling runSendingThread() must be created
 * 	@see runSendingThread()
**/
void stopSender(netWork_Sender_t* pSender);

/**
 *  @brief received
 * 	@param pSender the pointer on the Sender
 * 	@param id identifier of the command CMD_TYPE_ACK received
 *	@param seqNum sequence number of the acknowledge
**/
void senderAckReceived(netWork_Sender_t* pSender, int id, int seqNum);



#endif // _SENDER_H_

