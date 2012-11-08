/**
 *	@file network.h
 *  @brief single buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NET_WORK_H_
#define _NET_WORK_H_

// static :

//Enumerations :


//Structures :

/**
 *  @brief piloting command
**/
typedef struct network_t
{
    network_Sender_t* pSender;
    network_Receiver_t* pReceiver;
    network_inOutBuffer_t** ppTabInput;
    network_inOutBuffer_t** ppTabOutput;
    int numOfInput;
    int numOfOutput;
    
}network_t;


/**
 *  @brief Create a Network
 *	@post Call deleteNetwork
 * 	@return Pointer on the new Network
**/
network_t* newNetwork(	unsigned int recvBuffSize,unsigned int sendBuffSize,
						unsigned int numberOfOutput, unsigned int numberOfInput, ...);

/**
 *  @brief Delete the Network
 * 	@param ppNetwork address of the pointer on Network
 * 	@see newNetwork()
**/
void deleteNetwork(network_t** ppNetwork);

#endif // _NET_WORK_H_

