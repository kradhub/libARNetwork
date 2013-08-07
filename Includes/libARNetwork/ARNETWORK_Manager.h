/**
 *  @file ARNETWORK_Manager.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
 */

#ifndef _ARNETWORK_MANAGER_H_
#define _ARNETWORK_MANAGER_H_

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>
#include <libARNetworkAL/ARNETWORKAL_Manager.h>

/**
 *  @brief status return by the callback.
 */
typedef enum
{
    ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT = 0, /**< default value must be returned when the status callback differ of ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT */
    ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT, /**< pop the data (default behavior)*/
    ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY, /**< reset the number of retry */
    ARNETWORK_MANAGER_CALLBACK_RETURN_FLUSH /**< flush all input buffers */

} eARNETWORK_MANAGER_CALLBACK_RETURN;

/**
 *  @brief status sent by the callback.
 */
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
 *  @warning the callback can't call the ARNETWORK's functions
 *  @param[in] IoBufferId identifier of the IoBuffer is calling back
 *  @param[in] dataPtr pointer on the data
 *  @param[in] customData custom data
 *  @param[in] status indicating the reason of the callback. eARNETWORK_MANAGER_CALLBACK_STATUS type
 *  @return what do in timeout case
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
 */
typedef eARNETWORK_MANAGER_CALLBACK_RETURN (*ARNETWORK_Manager_Callback_t) (int IoBufferId, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

/**
 *  @brief network manager allow to send and receive data acknowledged or not.
 */
typedef struct ARNETWORK_Manager_t ARNETWORK_Manager_t;

/**
 *  @brief Create a new Manager
 *  @warning This function allocate memory
 *  @post ARNETWORKAL_Manager_t must be created to initialize the network.
 *  @post ARNETWORK_Manager_Delete() must be called to delete the Network and free the memory allocated.
 *  @param[in] numberOfInput Number of input buffer
 *  @param[in] inputParamArr array of the parameters of creation of the inputs. The array must contain as many parameters as the number of input buffer.
 *  @param[in] numberOfOutput Number of output buffer
 *  @param[in] outputParamArr array of the parameters of creation of the outputs. The array must contain as many parameters as the number of output buffer.
 *  @param[in] pingDelayMs Minimum amount of time (ms) between two pings. Put a negative value to disable ping, and zero to use default value
 *  @param[out] errorPtr pointer on the error output.
 *  @return Pointer on the new Manager
 *  @note This creator adds for all output, one other IOBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer arrays.
 *  @warning The identifiers of the IoBuffer should not exceed the value 128.
 *  @see ARNETWORK_Manager_Delete()
 */
ARNETWORK_Manager_t* ARNETWORK_Manager_New(ARNETWORKAL_Manager_t *networkALManager, unsigned int numberOfInput, ARNETWORK_IOBufferParam_t *inputParamArr, unsigned int numberOfOutput, ARNETWORK_IOBufferParam_t *outputParamArr, int pingDelayMs, eARNETWORK_ERROR *errorPtr);

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param managerPtrAddr address of the pointer on the Manager
 *  @see ARNETWORK_Manager_New()
 */
void ARNETWORK_Manager_Delete(ARNETWORK_Manager_t **managerPtrAddr);

/**
 *  @brief Manage the sending of the data
 *  @warning This function must be called in its own thread.
 *  @post Before join the thread calling this function, ARNETWORK_Manager_Stop() must be called.
 *  @param data thread data of type ARNETWORK_Manager_t*
 *  @return NULL
 *  @see ARNETWORK_Manager_Stop()
 */
void* ARNETWORK_Manager_SendingThreadRun(void *data);

/**
 *  @brief Manage the reception of the data.
 *  @warning This function must be called by a specific thread.
 *  @post Before join the thread calling this function, ARNETWORK_Manager_Stop() must be called.
 *  @param data thread datas of type ARNETWORK_Manager_t*
 *  @return NULL
 *  @see ARNETWORK_Manager_Stop()
 */
void* ARNETWORK_Manager_ReceivingThreadRun(void *data);

/**
 *  @brief stop the threads of sending and reception
 *  @details Used to kill the threads calling ARNETWORK_Manager_SendingThreadRun() and ARNETWORK_Manager_ReceivingThreadRun().
 *  @param managerPtr pointer on the Manager
 *  @see ARNETWORK_Manager_SendingThreadRun()
 *  @see ARNETWORK_Manager_ReceivingThreadRun()
 */
void ARNETWORK_Manager_Stop(ARNETWORK_Manager_t *managerPtr);

/**
 *  @brief Flush all buffers of the network manager
 *  @param managerPtr pointer on the Manager
 *  @return error eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_Manager_Flush(ARNETWORK_Manager_t *managerPtr);

/**
 *  @brief Add data to send in a IOBuffer
 *  @param managerPtr pointer on the Manager
 *  @param[in] inputBufferID identifier of the input buffer in which the data must be stored
 *  @param[in] dataPtr pointer on the data to send
 *  @param[in] dataSize size of the data to send
 *  @param[in] customData custom data sent to the callback
 *  @param[in] callback pointer on the callback to call when the data is sent or an error occurred
 *  @param[in] doDataCopy indicator to copy the data in the ARNETWORK_Manager
 *  @return error eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_Manager_SendData(ARNETWORK_Manager_t *managerPtr, int inputBufferID, uint8_t *dataPtr, int dataSize, void *customData, ARNETWORK_Manager_Callback_t callback, int doDataCopy);

/**
 *  @brief Read data received in a IOBuffer using variable size data (blocking function)
 *  @warning This is a blocking function.
 *  @warning Applications which need to close the manager should NOT use this function
 *  @param managerPtr pointer on the Manager
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] dataPtr pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] readSizePtr pointer to store the size of the data read
 *  @return error eARNETWORK_ERROR type
 */
eARNETWORK_ERROR ARNETWORK_Manager_ReadData(ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr);

/**
 *  @brief try to read data received in a IOBuffer using variable size data (non-blocking function)
 *  @param managerPtr pointer on the Manager
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] dataPtr pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] readSizePtr pointer to store the size of the data read
 *  @return error eARNETWORK_ERROR type
 */
eARNETWORK_ERROR ARNETWORK_Manager_TryReadData(ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr);

/**
 *  @brief Read, with timeout, a data received in IOBuffer using variable size data
 *  @param managerPtr pointer on the ARNETWORK_Manager_t
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] dataPtr pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] readSizePtr pointer to store the size of the data read
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eARNETWORK_ERROR type
 */
eARNETWORK_ERROR ARNETWORK_Manager_ReadDataWithTimeout(ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr, int timeoutMs);

/**
 * @brief Flush an input buffer
 * @param managerPtr Pointer to the ARNETWORK_Manager_t
 * @param[in] inBufferID Identifier of the buffer to flush
 * @return error eARNETWORK_ERROR type
 */
eARNETWORK_ERROR ARNETWORK_Manager_FlushInputBuffer(ARNETWORK_Manager_t *managerPtr, int inBufferID);

/**
 * @brief Flush an output buffer
 * @param managerPtr Pointer to the ARNETWORK_Manager_t
 * @param[in] outBufferID Identifier of the buffer to flush
 * @return error eARNETWORK_ERROR type
 */
eARNETWORK_ERROR ARNETWORK_Manager_FlushOutputBuffer(ARNETWORK_Manager_t *managerPtr, int outBufferID);

/**
 * @brief Gets the estimated latency of the ARNETWORK_Manager
 *
 * This function returns the maximum value between the last internal ping result,
 * and the current ping timer. This allows a greater reactivity for ping spikes
 *
 * @param managerPtr Pointer to the ARNETWORK_Manager_t
 * @return The estimated latency in ms, or a negative value if we couldn't determine it
 */
int ARNETWORK_Manager_GetEstimatedLatency (ARNETWORK_Manager_t *managerPtr);

#endif /* _ARNETWORK_MANAGER_H_ */
