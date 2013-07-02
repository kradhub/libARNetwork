/**
 *  @file ARNETWORK_IOBuffer.h
 *  @brief input or output buffer, used by ARNetwork_Receiver or ARNetwork_Sender
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
 */

#ifndef _ARNETWORK_IOBUFFER_PRIVATE_H_
#define _ARNETWORK_IOBUFFER_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Frame.h>
#include "ARNETWORK_RingBuffer.h"
#include <libARNetwork/ARNETWORK_IOBufferParam.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Sem.h>

/*****************************************
 *
 *             IOBuffer header:
 *
 *****************************************/

/**
 *  @brief Input buffer used by ARNetwork_Sender or output buffer used by ARNetwork_Receiver
 *  @warning before to be used the inOutBuffer must be created through ARNETWORK_IOBuffer_New()
 *  @post after its using the IOBuffer must be deleted through ARNETWORK_IOBuffer_Delete()
 */
typedef struct
{
    int ID; /**< Identifier used to find the ioBuffer in a array*/
    ARNETWORK_RingBuffer_t *dataCopyRBufferPtr; /**< Pointer on the ringBuffer used to store the data copy */
    ARNETWORK_RingBuffer_t *dataDescriptorRBufferPtr; /**< Pointer on the ringBuffer used to store the data description */
    eARNETWORKAL_FRAME_TYPE dataType; /**< Type of the data stored in the buffer*/
    int sendingWaitTimeMs;  /**< Time in millisecond between 2 send when the InOutBuffer if used with a libARNetwork/sender*/
    int ackTimeoutMs; /**< Timeout in millisecond after retry to send the data when the InOutBuffer is used with a libARNetwork/sender*/
    int numberOfRetry; /**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libARNetwork/sender*/

    int isWaitAck; /**< Indicator of waiting an acknowledgement  (1 = true | 0 = false). Must be accessed through ARNETWORK_IOBuffer_IsWaitAck()*/
    uint8_t seqWaitAck; /**< Sequence number of the acknowledge waiting if used with a libARNetwork/sender or of the last command stored if used with a libARNetwork/reveiver*/
    uint8_t seq; /**< Sequence number for data sent from this buffer */
    int waitTimeCount; /**< Counter of time to wait before the next sending*/
    int ackWaitTimeCount; /**< Counter of time to wait before to consider a timeout without receiving an acknowledgement*/
    int retryCount; /**< Counter of sending retry remaining before to consider a failure*/

    ARSAL_Mutex_t mutex;  /**< Mutex to take before to use the IOBuffer.
                           *   @warning This mutex is not managed by the IOBuffer itself but by the user
                           *   @see ARNETWORK_IOBuffer_Lock()
                           *   @see ARNETWORK_IOBuffer_Unlock()
                           */
    ARSAL_Sem_t outputSem; /**< Semaphore used, by the outputs, to know when a data is ready to be read */

}ARNETWORK_IOBuffer_t;

/**
 *  @brief Create a new input or output buffer
 *  @warning This function allocate memory
 *  @post ARNETWORK_IOBuffer_Delete() must be called to delete the input or output buffer and free the memory allocated
 *  @param[in] paramPtr Pointer on the parameters for the new input or output buffer
 *  @param[in] isInternal Flag to disable value checks on internal buffers
 *  @return Pointer on the new input or output buffer
 *  @see ARNETWORK_IOBuffer_Delete()
 */
ARNETWORK_IOBuffer_t* ARNETWORK_IOBuffer_New(const ARNETWORK_IOBufferParam_t *paramPtr, int isInternal);

/**
 *  @brief Delete the input or output buffer
 *  @warning This function free memory
 *  @param IOBufferPtrAddr address of the pointer on the input or output buffer to delete
 *  @see ARNETWORK_IOBuffer_New()
 */
void ARNETWORK_IOBuffer_Delete( ARNETWORK_IOBuffer_t **IOBufferPtrAddr );

/**
 *  @brief Possibility of the IOBuffer to copy the data in itself
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @return 1 if the IOBuffer can copy the data otherwise 0
 */
static inline int ARNETWORK_IOBuffer_CanCopyData(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    return (IOBufferPtr->dataCopyRBufferPtr != NULL) ? 1 : 0;
}

/**
 *  @brief Receive an acknowledgement to a IOBuffer.
 *  @warning The IOBuffer mutex must lock before the calling of this function and unlock after.
 *  @details If the IOBuffer is waiting about an acknowledgement and seqNum is equal to the sequence number waited, the inOutBuffer pops the last data and delete its is waiting acknowledgement.
 *  @param[in] IOBufferPtr Pointer on the input or output buffer
 *  @param[in] seqNumber sequence number of the acknowledgement
 *  @return error equal to ARNETWORK_OK if the data has been correctly acknowledged otherwise equal to 1
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_AckReceived( ARNETWORK_IOBuffer_t *IOBufferPtr, uint8_t seqNumber );

/**
 *  @brief Get if the IOBuffer is waiting an acknowledgement.
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @return IsWaitAck equal to 1 if the IOBuffer is waiting an acknowledgement otherwise equal to 0
 */
int ARNETWORK_IOBuffer_IsWaitAck( ARNETWORK_IOBuffer_t *IOBufferPtr );

/**
 *  @brief Lock the IOBuffer's mutex.
 *  @param IOBufferPtr Pointer on the IOBuffer.
 *  @return error equal to ARNETWORK_OK if the data are correctly locked, otherwise see eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_Lock( ARNETWORK_IOBuffer_t *IOBufferPtr);

/**
 *  @brief Unlock the IOBuffer's mutex.
 *  @param IOBufferPtr Pointer on the IOBuffer.
 *  @return error equal to ARNETWORK_OK if the data are correctly locked, otherwise see eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_Unlock( ARNETWORK_IOBuffer_t *IOBufferPtr);

/**
 *  @brief cancel all remaining data.
 *  @warning the IOBuffer must store ARNETWORK_DataDescriptor_t
 *  @param IOBufferPtr Pointer on the IOBuffer.
 *  @return error equal to ARNETWORK_OK if the data are correctly free otherwise see eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_CancelAllData(ARNETWORK_IOBuffer_t *IOBufferPtr);

/**
 *  @brief Pop the later data of the IOBuffer and free it
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @return error equal to ARNETWORK_OK if the data are correctly deleted otherwise see eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_PopData(ARNETWORK_IOBuffer_t *IOBufferPtr);

/**
 *  @brief Pop the later data of the IOBuffer with callback calling and free it
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @param[in] callbackStatus status sent by the callback
 *  @return error equal to ARNETWORK_OK if the data are correctly deleted otherwise see eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_PopDataWithCallBack(ARNETWORK_IOBuffer_t *IOBufferPtr, eARNETWORK_MANAGER_CALLBACK_STATUS callbackStatus);

/**
 *  @brief flush the IOBuffer
 *  @warning The IOBuffer mutex must lock before the calling of this function and unlock after.
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @return eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_Flush(ARNETWORK_IOBuffer_t *IOBufferPtr);

/**
 *  @brief Add data in a IOBuffer
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @param[in] dataPtr pointer on the data to add
 *  @param[in] dataSize size of the data to add
 *  @param[in] customData custom data sent to the callback
 *  @param[in] callback pointer on the callback to call when the data is sent or an error occurred
 *  @param[in] doDataCopy indocator to copy the data in the IOBuffer
 *  @return error eARNETWORK_ERROR
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_AddData(ARNETWORK_IOBuffer_t *IOBufferPtr, uint8_t *dataPtr, int dataSize, void *customData, ARNETWORK_Manager_Callback_t callback, int doDataCopy);

/**
 *  @brief read data received in a IOBuffer
 *  @warning the data read is pop
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @param[out] dataPtr pointer on the data read
 *  @param[in] dataLimitSize limit size of the copy
 *  @param[out] readSizePtr pointer to store the size of the data read ; can be equal to NULL
 *  @return error eARNETWORK_ERROR type
 */
eARNETWORK_ERROR ARNETWORK_IOBuffer_ReadData(ARNETWORK_IOBuffer_t *IOBufferPtr, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr);

#endif /** _ARNETWORK_IOBUFFER_PRIVATE_H_ */
