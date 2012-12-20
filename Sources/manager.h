/**
 *	@file manager.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_MANAGER_NOEXPORTED_H_
#define _NETWORK_MANAGER_NOEXPORTED_H_

#include "ioBuffer.h"
#include "sender.h"
#include "receiver.h"
#include <libNetwork/manager.h>

/**
 *  @brief network manager allow to send data acknowledged or not.
**/
struct network_manager_t
{
    network_sender_t* pSender; /**< Pointer on the sender*/
    network_receiver_t* pReceiver; /**< Pointer on the receiver*/
    network_ioBuffer_t** ppTabInput; /**< Address of the table storing the input buffer*/
    network_ioBuffer_t** ppTabOutput; /**< Address of the table storing the output buffer*/
    int numOfInput; /**< Number of input buffer*/
    int numOfOutput; /**< Number of output buffer*/
    int numOfInputWithoutAck; /**< Number of input buffer without the  buffers of acknowledgement*/
    int numOfOutputWithoutAck; /**< Number of output buffer without the  buffers of acknowledgement*/
    
};


#endif // _NETWORK_MANAGER_NOEXPORTED_H_

