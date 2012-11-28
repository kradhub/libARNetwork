/**
 *	@file sender.h
 *  @brief manage the data sending, used by libNetwork/manager and libNetwork/receiver
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _PRIVATE_NETWORK_SENDER_H_
#define _PRIVATE_NETWORK_SENDER_H_

#include <libNetwork/sender.h>

/*****************************************
 * 
 * 			private header:
 *
******************************************/
/**
 *  @brief send the data
 * 	@param pSender the pointer on the Sender
 *	@note only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
void NETWORK_SenderSend(network_sender_t* pSender);

/**
 *  @brief add data to the sender buffer
 * 	@param pSender the pointer on the Sender
 *	@param pinputBuff
 * 	@param seqNum
 * 	@note only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
int NETWORK_SenderAddToBuffer(	network_sender_t* pSender,const network_ioBuffer_t* pinputBuff,
                                int seqNum);

#endif // _PRIVATE_NETWORK_SENDER_H_

