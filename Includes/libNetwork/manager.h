/**
 *	@file manager.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_MANGER_H_
#define _NETWORK_MANGER_H_

#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>

/**
 *  @brief NETWORK_Manager errors know.
**/
typedef enum
{
	NETWORK_MANAGER_OK = 0, /**< no error */
	NETWORK_MANAGER_ERROR_ALLOC = -1000 /**< flksdqljlk */ 
	
} eNETWORK_Manager_Error;

/**
 *  @brief network manager allow to send data acknowledged or not.
**/
typedef struct network_manager_t
{
    network_sender_t* pSender; /**< Pointer on the sender*/
    network_receiver_t* pReceiver; /**< Pointer on the receiver*/
    network_ioBuffer_t** ppTabInput; /**< Address of the table storing the input buffer*/
    network_ioBuffer_t** ppTabOutput; /**< Address of the table storing the output buffer*/
    int numOfInput; /**< Number of input buffer*/
    int numOfOutput; /**< Number of output buffer*/
    int numOfInputWithoutAck; /**< Number of input buffer without the  buffers of acknowledgement*/
    int numOfOutputWithoutAck; /**< Number of output buffer without the  buffers of acknowledgement*/
    
}network_manager_t;

/**
 *  @brief Create a new Manager
 * 	@warning This function allocate memory
 * 	@post NETWORK_SenderConnection() must be called to indicate on which address send the data.
 * 	@post NETWORK_ReceiverBind() must be called to indicate on which address receive the data.
 *  @post NETWORK_DeleteManager() must be called to delete the Network and free the memory allocated.
 * 	@param[in] recvBuffSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 * 	@param[in] sendBuffSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 * 	@param[in] numberOfInput Number of input buffer
 * 	@param[in] ptabParamInput Table of the parameters of creation of the inputs. The table must contain as many parameters as the number of input buffer.
 * 	@param[in] numberOfOutput Number of output buffer
 * 	@param[in] ptabParamOutput Table of the parameters of creation of the outputs. The table must contain as many parameters as the number of output buffer.
 * 	@return Pointer on the new Manager
 * 	@note This creator adds for all output, one other inOutBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer tables.
 * 	@see NETWORK_DeleteManager()
**/
network_manager_t* NETWORK_NewManager(	unsigned int recvBuffSize,unsigned int sendBuffSize,
				unsigned int numberOfInput, network_paramNewInOutBuffer_t* ptabParamInput,
				unsigned int numberOfOutput, network_paramNewInOutBuffer_t* ptabParamOutput);

/**
 *  @brief Delete the Manager
 * 	@warning This function free memory
 * 	@param ppManager address of the pointer on Network
 * 	@see NETWORK_NewManager()
**/
void NETWORK_DeleteManager(network_manager_t** ppManager);

/**
 *  @brief Add data to send
 * 	@param pManager pointer on the Manager
 * 	@param[in] inputBufferId identifier of the input buffer in which the data must be stored
 * 	@param[in] pData pointer on the data to send
 *  @return error equal to 1 if the data is not correctly pushed in the the input buffer
**/
int NETWORK_ManagerSendData(network_manager_t* pManager, int inputBufferId, const void* pData);

/**
 *  @brief Read data received
 * 	@param pManager pointer on the Manager
 * 	@param[in] outputBufferId identifier of the output buffer in which the data must be read
 * 	@param[out] pData pointer on the data read
 *  @return error equal to 1 if the buffer is empty or the ID doesn't exist
**/
int NETWORK_ManagerReadData(network_manager_t* pManager, int outputBufferId, void* pData);

#endif // _NETWORK_MANGER_H_

