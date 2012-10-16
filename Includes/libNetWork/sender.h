/**
 *	@file sender.h
 *  @brief manage the data sending
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _SENDER_H_
#define _SENDER_H_

// static :


//Enumerations :


//Structures :
/**
 *  @brief properties of the sender
**/
typedef struct netWork_Sender_t netWork_Sender_t;
struct netWork_Sender_t
{
	netWork_buffSend_t* pNavCmdBuff;
	netWork_buffPilotCmd_t* pPiliotCmsBuff;
	// !! sal_mutex_t mutex; //???
	int isAlive;
	int sleepTime;
};



/**
 *  @brief Create a sender
 *	@post Call deleteSender()
 * 	@return Pointer on the new sender
**/
netWork_Sender_t* newSender(); 

/**
 *  @brief Delete the sender
 * 	@param ppBuffsend address of the pointer on the Sender
 *	@see newBuffCmdAck()
**/
void deleteSender(netWork_Sender_t** ppSender);

/**
 *  @brief manage the communication between the drone and the application
 * 	Must be called by a specific thread 
 * 	@param data thread datas of type netWork_Sender_t*
**/
void* runSendingThread(void* data);


#endif // _SENDER_H_

