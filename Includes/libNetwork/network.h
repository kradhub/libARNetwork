/**
 *	@file network.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NET_WORK_H_
#define _NET_WORK_H_

/**
 *  @brief network manager allow to send data acknowledged or not.
**/
typedef struct network_t
{
    network_Sender_t* pSender; /**< Pointer on the sender*/
    network_Receiver_t* pReceiver; /**< Pointer on the receiver*/
    network_inOutBuffer_t** ppTabInput; /**< Address of the table storing the input buffer*/
    network_inOutBuffer_t** ppTabOutput; /**< Address of the table storing the output buffer*/
    int numOfInput; /**< Numbur of input buffer*/
    int numOfOutput; /**< Numbur of output buffer*/
    
}network_t;

/**
 *  @brief Create a new Network
 * 	@warning This function allocate memory
 * 	@post senderConnection() must be called to indicate on which address send the data.
 * 	@post receiverBind() must be called to indicate on which address receive the data.
 *  @post deleteNetwork() must be called to delete the Network and free the memory allocated.
 * 	@param[in] recvBuffSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 * 	@param[in] sendBuffSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 * 	@param[in] numberOfOutput Number of output buffer
 * 	@param[in] numberOfInput Number of input buffer
 * 	@param[in] ... Parameters of creation of the output buffers followed by the parameters of creation of the input buffers.
 * All these parameters must be network_paramNewInOutBuffer_t type.
 * The number of parameters must be equal of the sum of numberOfOutput and numberOfInput.
 * 	@return Pointer on the new Network
 * 	@note This creator adds for all output, one other inOutBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer tables.
 * 	@see deleteNetwork()
**/
network_t* newNetwork(	unsigned int recvBuffSize,unsigned int sendBuffSize,
						unsigned int numberOfOutput, unsigned int numberOfInput, ...);

/**
 *  @brief Delete the Network
 * 	@warning This function free memory
 * 	@param ppNetwork address of the pointer on Network
 * 	@see newNetwork()
**/
void deleteNetwork(network_t** ppNetwork);

#endif // _NET_WORK_H_

