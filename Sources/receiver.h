/**
 *	@file receiver.h
 *  @brief manage the data received, used by libNetwork/network
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _PRIVATE_NETWORK_RECEIVER_H_
#define _PRIVATE_NETWORK_RECEIVER_H_

#include <libNetwork/receiver.h>

/*****************************************
 * 
 * 			private header:
 *
******************************************/

/**
 *  @brief get the next Frame of the receiving buffer
 * 	@param pBuffsend the pointer on the receiver
 * 	@param prevFrame[in] pointer on the previous frame
 * 	@return nextFrame pointer on the next frame
 *	@pre only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
network_frame_t* NETWORK_GetNextFrame(network_receiver_t* pReceiver, network_frame_t* prevFrame);

#endif // _PRIVATE_NETWORK_RECEIVER_H_

