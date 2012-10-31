/**
 *	@file receiver.h
 *  @brief manage the data receiving
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <libNetWork/inOutBuffer.h>
#include <libNetWork/sender.h>


#define NAV_CMD_BUFFER_SIZE 256
#define NAV_CMD_CELL_SIZE 10

#define PILOTT_CMD_CELL_SIZE 10

#define SLEEP_TIME_MS 200

// static :


//Enumerations :


//Structures :

/**
 *  @brief properties of the receiver
**/
typedef struct netWork_Receiver_t
{
	netWork_buffer_t* pRecvBuffer;
	
	netWork_Sender_t* pSender;
	
	netWork_inOutBuffer_t** pptab_outputBuffer;
	int outputBufferNum;
	
	// !! sal_mutex_t mutex; //???
	int isAlive;
	int sleepTime;
	
	//int fd;//fifopipe temp
	int socket;
	
}netWork_Receiver_t;


/**
 *  @brief Create a receiver
 *	@post Call deleteReceiver()
 * 	@return Pointer on the new receiver
**/
//netWork_Receiver_t* newReceiver(unsigned int recvBufferSize, unsigned int outputBufferNum, ...);
netWork_Receiver_t* newReceiver(	unsigned int recvBufferSize, unsigned int outputBufferNum,
									netWork_inOutBuffer_t** pptab_output);

/**
 *  @brief Delete the Receiver
 * 	@param ppReceiver address of the pointer on the Receiver
 *	@see newReceiver()
**/
void deleteReceiver(netWork_Receiver_t** ppReceiver);

/**
 *  @brief manage the communication between the drone and the application
 * 	Must be called by a specific thread 
 * 	@param data thread datas of type netWork_Receive_t*
**/
void* runReceivingThread(void* data);

/**
 *  @brief stop the sending
 * 	@param pReceiver pointer on the Receiver
 *	@pre the thread calling runSendingThread() must be created
 * 	@see runReceivingThread()
**/
void stopReceiver(netWork_Receiver_t* pReceiver);

/**
 *  @brief return a acknowledged by the sender
 * 	@param pReceiver the pointer on the Receiver
 * 	@param id identifier of the command to acknowledged
 * 	@param seq sequence of the command to acknowledged
 *	@see newReceiver()
**/
void returnASK(netWork_Receiver_t* pReceiver, int id, int seq);



int receiverRead(netWork_Receiver_t* pReceiver);

int idOutputToIdAck( int id);
int idAckToIdInput( int id);

int receiverBind(netWork_Receiver_t* pReceiver, unsigned short port);

#endif // _RECEIVER_H_

