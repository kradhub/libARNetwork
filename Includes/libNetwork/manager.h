/**
 *  @file manager.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <libNetwork/status.h>
#include <libNetwork/paramNewIoBuffer.h>
#include <libNetwork/deportedData.h>

/**
 *  @brief network manager allow to send data acknowledged or not.
**/
typedef struct network_manager_t network_manager_t;

/**
 *  @brief Create a new Manager
 *  @warning This function allocate memory
 *  @post NETWORK_ManagerSocketsInit() must be called to initialize the sockets, indicate on which address send the data, the sending port the receiving port and the timeout.
 *  @post NETWORK_DeleteManager() must be called to delete the Network and free the memory allocated.
 *  @param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 *  @param[in] sendBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 *  @param[in] numberOfInput Number of input buffer
 *  @param[in] ptabParamInput Table of the parameters of creation of the inputs. The table must contain as many parameters as the number of input buffer.
 *  @param[in] numberOfOutput Number of output buffer
 *  @param[in] ptabParamOutput Table of the parameters of creation of the outputs. The table must contain as many parameters as the number of output buffer.
 *  @param[out] pError pointor on the error output.
 *  @return Pointer on the new Manager
 *  @note This creator adds for all output, one other inOutBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer tables.
 *  @warning The identifiers of the IoBuffer should not exceed the value 1000.
 *  @see NETWORK_DeleteManager()
**/
network_manager_t* NETWORK_NewManager( unsigned int recvBufferSize,unsigned int sendBufferSize,
                unsigned int numberOfInput, network_paramNewIoBuffer_t* ptabParamInput,
                unsigned int numberOfOutput, network_paramNewIoBuffer_t* ptabParamOutput,
                eNETWORK_Error* pError );

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param ppManager address of the pointer on Network
 *  @see NETWORK_NewManager()
**/
void NETWORK_DeleteManager(network_manager_t** ppManager);

/**
 *  @brief initialize UDP sockets of sending and receiving the data.
 *  @param pManager pointer on the Manager
 *  @param[in] addr address of connection at which the data will be sent.
 *  @param[in] recvPort port on which the data will be received.
 *  @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function NETWORK_ReceiverRead().
 *  @return error equal to NETWORK_OK if the Bind if successful otherwise see eNETWORK_Manager_Error.
**/
eNETWORK_Error NETWORK_ManagerSocketsInit(network_manager_t* pManager,const char* addr,
                                            int sendingPort,int recvPort, int recvTimeoutSec);

/**
 *  @brief Manage the sending of the data 
 *  @warning This function must be called by a specific thread.
 *  @pre The sockets must be initialized through NETWORK_ManagerSocketsInit().
 *  @post Before join the thread calling this function, NETWORK_ManagerStop() must be called.
 *  @note This function send the data stored in the input buffer through NETWORK_ManagerSendData().
 *  @param data thread datas of type network_manager_t*
 *  @return NULL
 *  @see NETWORK_ManagerSocketsInit()
 *  @see NETWORK_ManagerStop()
**/
void* NETWORK_ManagerRunSendingThread(void* data);

/**
 *  @brief Manage the reception of the data.
 *     @warning This function must be called by a specific thread.
 *     @pre The socket of the receiver must be initialized through NETWORK_ManagerSocketsInit().
 *     @post Before join the thread calling this function, NETWORK_ManagerStop() must be called.
 *     @note This function receives the data through NETWORK_ManagerReadData() and stores them in the output buffers according to their parameters.
 *  @param data thread datas of type network_manager_t*
 *  @return NULL
 *  @see NETWORK_ManagerSocketsInit()
 *  @see NETWORK_ManagerStop()
 *  @see NETWORK_ManagerReadData()
**/
void* NETWORK_ManagerRunReceivingThread(void* data);

/**
 *  @brief stop the threads of sending and reception
 *  @details Used to kill the threads calling NETWORK_ManagerRunSendingThread() and NETWORK_ManagerRunReceivingThread().
 *  @param @param pManager pointer on the Manager
 *  @see NETWORK_ManagerRunSendingThread()
 *  @see NETWORK_ManagerRunReceivingThread()
**/
void NETWORK_ManagerStop(network_manager_t* pManager);

/**
 *  @brief Add data to send
 *  @param pManager pointer on the Manager
 *  @param[in] inputBufferId identifier of the input buffer in which the data must be stored
 *  @param[in] pData pointer on the data to send
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_ManagerSendData(network_manager_t* pManager, int inputBufferId, const uint8_t* pData);

/**
 *  @brief Add deported data to send
 *  @param pManager pointer on the Manager
 *  @param[in] inputBufferId identifier of the input buffer in which the data must be stored
 *  @param[in] pData pointer on the data to send
 *  @param[in] dataSize size of the data to send
 *  @param[in] void* pCustomData Pointer on a custom data 
 *  @param[in] callback pointer on the callback to call when the data is sent or an error occurred
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_ManagerSendDeportedData( network_manager_t* pManager, int inputBufferId,
                                                  uint8_t* pData, int dataSize, void* pCustomData,
                                                  network_deportDatacallback callback );

/**
 *  @brief Read data received
 *  @warning the outputBuffer should not be deportedData type
 *  @param pManager pointer on the Manager
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] pData pointer on the data read
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_ManagerReadData(network_manager_t* pManager, int outputBufferId, uint8_t* pData);

/**
 *  @brief Read deported data received
 *  @warning the outputBuffer must be deportedData type
 *  @param pManager pointer on the Manager
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] pData pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] pReadSize pointer to store the size of the data read
 *  @return error eNETWORK_Error type
**/
eNETWORK_Error NETWORK_ManagerReadDeportedData( network_manager_t* pManager, int outputBufferId,
                                                  uint8_t* pData, int dataLimitSize, int* pReadSize );


/**
 *  @brief Read data received
 *  @details This function is blocking
 *  @warning the outputBuffer should not be deportedData type
 *  @param pManager pointer on the Manager
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] pData pointer on the data read
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_ManagerReadDataWithTimeout( network_manager_t* pManager, 
                                                     int outputBufferId, 
                                                     uint8_t* pData,
                                                     int timeoutMs );

/**
 *  @brief Read deported data received
 *  @warning the outputBuffer must be deportedData type
 *  @param pManager pointer on the Manager
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] pData pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] pReadSize pointer to store the size of the data read
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eNETWORK_Error type
**/
eNETWORK_Error NETWORK_ManagerReadDeportedDataWithTimeout( network_manager_t* pManager, 
                                                             int outputBufferId,
                                                             uint8_t* pData, 
                                                             int dataLimitSize, 
                                                             int* pReadSize,
                                                             int timeoutMs );

#endif // _NETWORK_MANAGER_H_

