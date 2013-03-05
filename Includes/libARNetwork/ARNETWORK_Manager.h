/**
 *  @file ARNETWORK_Manager.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_MANAGER_H_
#define _ARNETWORK_MANAGER_H_

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>


/**
 *  @brief status return by the callback.
**/
typedef enum
{
    ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT = 0, /**< default value must be returned when the status callback differ of ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT */
    ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT, /**< pop the data (default behavior)*/
    ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY, /**< reset the number of retry */
    ARNETWORK_MANAGER_CALLBACK_RETURN_FLUSH /**< flush all input buffers */
    
} eARNETWORK_MANAGER_CALLBACK_RETURN;

/**
 *  @brief status sent by the callback.
**/
typedef enum
{
    ARNETWORK_MANAGER_CALLBACK_STATUS_SENT = 0, /**< data sent */
    ARNETWORK_MANAGER_CALLBACK_STATUS_ACK_RECEIVED, /**< acknowledged received */
    ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT, /**< timeout occurred, data not received; the callback must return what the network manager must do with the data. */
    ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL, /**< data will not sent */
    ARNETWORK_MANAGER_CALLBACK_STATUS_FREE /**< free the data not sent; in case of variable size data, this data can be reused or freed. */
    
} eARNETWORK_MANAGER_CALLBACK_STATUS;

/**
 *  @brief callback use when the data are sent or have a timeout 
 *  @param[in] IoBufferId identifier of the IoBuffer is calling back
 *  @param[in] dataPtr pointer on the data
 *  @param[in] customData custom data
 *  @param[in] status indicating the reason of the callback. eARNETWORK_MANAGER_CALLBACK_STATUS type
 *  @return what do in timeout case
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
**/
typedef eARNETWORK_MANAGER_CALLBACK_RETURN (*ARNETWORK_Manager_Callback_t) (int IoBufferId, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

/**
 *  @brief network manager allow to send and receive data acknowledged or not.
**/
typedef struct ARNETWORK_Manager_t ARNETWORK_Manager_t;

/**
 *  @brief Create a new Manager
 *  @warning This function allocate memory
 *  @post ARNETWORK_Manager_SocketsInit() must be called to initialize the sockets, indicate on which address send the data, the sending port the receiving port and the timeout.
 *  @post ARNETWORK_Manager_Delete() must be called to delete the Network and free the memory allocated.
 *  @param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 *  @param[in] sendBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 *  @param[in] numberOfInput Number of input buffer
 *  @param[in] inputParamArr array of the parameters of creation of the inputs. The array must contain as many parameters as the number of input buffer.
 *  @param[in] numberOfOutput Number of output buffer
 *  @param[in] outputParamArr array of the parameters of creation of the outputs. The array must contain as many parameters as the number of output buffer.
 *  @param[out] errorPtr pointor on the error output.
 *  @return Pointer on the new Manager
 *  @note This creator adds for all output, one other IOBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer arrays.
 *  @warning The identifiers of the IoBuffer should not exceed the value 128.
 *  @see ARNETWORK_Manager_Delete()
**/
ARNETWORK_Manager_t* ARNETWORK_Manager_New(unsigned int recvBufferSize, unsigned int sendBufferSize, unsigned int numberOfInput, ARNETWORK_IOBufferParam_t *inputParamArr, unsigned int numberOfOutput, ARNETWORK_IOBufferParam_t *outputParamArr, eARNETWORK_ERROR *errorPtr);

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param managerPtrAddr address of the pointer on the Manager
 *  @see ARNETWORK_Manager_New()
**/
void ARNETWORK_Manager_Delete(ARNETWORK_Manager_t **managerPtrAddr);

/**
 *  @brief initialize UDP sockets of sending and receiving the data.
 *  @param managerPtr pointer on the Manager
 *  @param[in] addr address of connection at which the data will be sent.
 *  @param[in] sendingPort port on which the data will be sent.
 *  @param[in] receivingPort port on which the data will be received.
 *  @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function ARNETWORK_Receiver_Read().
 *  @return error equal to ARNETWORK_OK if the initialization if successful otherwise see eARNETWORK_ERROR.
**/
eARNETWORK_ERROR ARNETWORK_Manager_SocketsInit(ARNETWORK_Manager_t *managerPtr, const char *addr, int sendingPort, int receivingPort, int recvTimeoutSec);

/**
 *  @brief Manage the sending of the data 
 *  @warning This function must be called in its own thread.
 *  @pre The sockets must be initialized through ARNETWORK_Manager_SocketsInit().
 *  @post Before join the thread calling this function, ARNETWORK_Manager_Stop() must be called.
 *  @note This function send the data stored in the input buffer through ARNETWORK_Manager_SendFixedSizeData().
 *  @param data thread datas of type ARNETWORK_Manager_t*
 *  @return NULL
 *  @see ARNETWORK_Manager_SocketsInit()
 *  @see ARNETWORK_Manager_Stop()
**/
void* ARNETWORK_Manager_SendingThreadRun(void *data);

/**
 *  @brief Manage the reception of the data.
 *  @warning This function must be called by a specific thread.
 *  @pre The socket of the receiver must be initialized through ARNETWORK_Manager_SocketsInit().
 *  @post Before join the thread calling this function, ARNETWORK_Manager_Stop() must be called.
 *  @note This function receives the data through ARNETWORK_Manager_ReadFixedSizeData() and stores them in the output buffers according to their parameters.
 *  @param data thread datas of type ARNETWORK_Manager_t*
 *  @return NULL
 *  @see ARNETWORK_Manager_SocketsInit()
 *  @see ARNETWORK_Manager_Stop()
 *  @see ARNETWORK_Manager_ReadFixedSizeData()
**/
void* ARNETWORK_Manager_ReceivingThreadRun(void *data);

/**
 *  @brief stop the threads of sending and reception
 *  @details Used to kill the threads calling ARNETWORK_Manager_SendingThreadRun() and ARNETWORK_Manager_ReceivingThreadRun().
 *  @param managerPtr pointer on the Manager
 *  @see ARNETWORK_Manager_SendingThreadRun()
 *  @see ARNETWORK_Manager_ReceivingThreadRun()
**/
void ARNETWORK_Manager_Stop(ARNETWORK_Manager_t *managerPtr);

/**
 *  @brief Flush all buffers of the network manager
 *  @param managerPtr pointer on the Manager
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_Manager_Flush(ARNETWORK_Manager_t *managerPtr);

/**
 *  @brief Add data to send in a IOBuffer
 *  @param managerPtr pointer on the Manager
 *  @param[in] inputBufferID identifier of the input buffer in which the data must be stored
 *  @param[in] dataPtr pointer on the data to send
 *  @param[in] dataSize size of the data to send
 *  @param[in] customData custom data sent to the callback
 *  @param[in] callback pointer on the callback to call when the data is sent or an error occurred
 *  @param[in] doDataCopy indocator to copy the data in the ARNETWORK_Manager
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_Manager_SendData(ARNETWORK_Manager_t *managerPtr, int inputBufferID, uint8_t *dataPtr, int dataSize, void *customData, ARNETWORK_Manager_Callback_t callback, int doDataCopy);

/**
 *  @brief Read data received in a IOBuffer using variable size data
 *  @warning the outputBuffer must be using of variable Size Data type
 *  @param managerPtr pointer on the Manager
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] dataPtr pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] readSizePtr pointer to store the size of the data read
 *  @return error eARNETWORK_ERROR type
**/
eARNETWORK_ERROR ARNETWORK_Manager_ReadData(ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr);

/**
 *  @brief Read, with timeout, a data received in IOBuffer using variable size data
 *  @warning the outputBuffer must be using variable data
 *  @param managerPtr pointer on the ARNETWORK_Manager_t
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] dataPtr pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] readSizePtr pointer to store the size of the data read
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eARNETWORK_ERROR type
**/
eARNETWORK_ERROR ARNETWORK_Manager_ReadDataWithTimeout(ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr, int timeoutMs);

#endif /** _ARNETWORK_MANAGER_H_ */

