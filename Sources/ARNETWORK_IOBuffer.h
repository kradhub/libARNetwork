/**
 *  @file ARNETWORK_IOBuffer.h
 *  @brief input or output buffer, used by ARNetwork_Receiver or ARNetwork_Sender
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_IOBUFFER_PRIVATE_H_
#define _ARNETWORK_IOBUFFER_PRIVATE_H_

#include <libARNetwork/ARNETWORK_Frame.h>
#include "ARNETWORK_RingBuffer.h"
#include <libARNetwork/ARNETWORK_IOBufferParam.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Sem.h>

/*****************************************
 * 
 *             IOBuffer header:
 *
******************************************/

/**
 *  @brief Input buffer used by ARNetwork_Sender or output buffer used by ARNetwork_Receiver
 *  @warning before to be used the inOutBuffer must be created through ARNETWORK_IOBuffer_New()
 *  @post after its using the IOBuffer must be deleted through ARNETWORK_IOBuffer_Delete()
**/
typedef struct  
{
    int ID; /**< Identifier used to find the ioBuffer in a array*/
    ARNETWORK_RingBuffer_t *bufferPtr; /**< Pointer on the ringBuffer used to store the data*/
    eARNETWORK_FRAME_TYPE dataType; /**< Type of the data stored in the buffer*/
    int sendingWaitTimeMs;  /**< Time in millisecond between 2 send when the InOutBuffer if used with a libARNetwork/sender*/
    int ackTimeoutMs; /**< Timeout in millisecond after retry to send the data when the InOutBuffer is used with a libARNetwork/sender*/
    int numberOfRetry; /**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libARNetwork/sender*/
    //    timeoutcallback(ARNETWORK_IOBuffer_t* this)
    int isUsingVariableSizeData; /**< Indicator of using variable size data */
    
    int isWaitAck; /**< Indicator of waiting an acknowledgement  (1 = true | 0 = false). Must be accessed through ARNETWORK_IOBuffer_IsWaitAck()*/
    int seqWaitAck; /**< Sequence number of the acknowledge waiting if used with a libARNetwork/sender or of the last command stored if used with a libARNetwork/reveiver*/
    int waitTimeCount; /**< Counter of time to wait before the next sending*/
    int ackWaitTimeCount; /**< Counter of time to wait before to consider a timeout without receiving an acknowledgement*/
    int retryCount; /**< Counter of sending retry remaining before to consider a failure*/
    
    ARSAL_Mutex_t mutex;  /**< Mutex to take before to use the ringBuffer*/
    ARSAL_Sem_t outputSem; /**< Semaphore used, by the outputs, to know when a data is ready to be read */

}ARNETWORK_IOBuffer_t;

/**
 *  @brief Create a new input or output buffer
 *  @warning This function allocate memory
 *  @post ARNETWORK_IOBuffer_Delete() must be called to delete the input or output buffer and free the memory allocated
 *  @param[in] paramPtr Pointer on the parameters for the new input or output buffer
 *  @return Pointer on the new input or output buffer
 *  @see ARNETWORK_IOBuffer_Delete()
**/
ARNETWORK_IOBuffer_t* ARNETWORK_IOBuffer_New(const ARNETWORK_IOBufferParam_t *paramPtr); 

/**
 *  @brief Delete the input or output buffer
 *  @warning This function free memory
 *  @param IOBufferPtrAddr address of the pointer on the input or output buffer to delete
 *  @see ARNETWORK_IOBuffer_New()
**/
void ARNETWORK_IOBuffer_Delete( ARNETWORK_IOBuffer_t **IOBufferPtrAddr );

/**
 *  @brief Receive an acknowledgement to a IOBuffer.
 *  @details If the IOBuffer is waiting about an acknowledgement and seqNum is equal to the sequence number waited, the inOutBuffer pops the last data and delete its is waiting acknowledgement.
 *  @param[in] IOBufferPtr Pointer on the input or output buffer
 *  @param[in] seqNumber sequence number of the acknowledgement
 *  @return error equal to ARNETWORK_OK if the data has been correctly acknowledged otherwise equal to 1
**/
eARNETWORK_ERROR ARNETWORK_IOBuffer_AckReceived( ARNETWORK_IOBuffer_t *IOBufferPtr, int seqNumber );

/**
 *  @brief Get if the IOBuffer is waiting an acknowledgement.
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @return IsWaitAck equal to 1 if the IOBuffer is waiting an acknowledgement otherwise equal to 0
**/
int ARNETWORK_IOBuffer_IsWaitAck( ARNETWORK_IOBuffer_t *IOBufferPtr );

/**
 *  @brief call the callback of all variable size data with the ARNETWORK_MANAGER_CALLBACK_STATUS_FREE status.
 *  @warning the IOBuffer must store ARNETWORK_VariableSizeData_t
 *  @param IOBufferPtr Pointer on the IOBuffer using varaible size data
 *  @return error equal to ARNETWORK_OK if the data are correctly free otherwise see eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_IOBuffer_FreeAllVariableSizeData(ARNETWORK_IOBuffer_t *IOBufferPtr);

/**
 *  @brief delete the later data of the IOBuffer
 *  @warning used only when the data is correctly sent
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @param[in] callbackStatus status sent by the callback
 *  @return error equal to ARNETWORK_OK if the data are correctly deleted otherwise see eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_IOBuffer_DeleteData(ARNETWORK_IOBuffer_t *IOBufferPtr, int callbackStatus);

/**
 *  @brief flush the IOBuffer
 *  @param IOBufferPtr Pointer on the input or output buffer
 *  @return eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_IOBuffer_Flush( ARNETWORK_IOBuffer_t *IOBufferPtr );

#endif /** _ARNETWORK_IOBUFFER_PRIVATE_H_ */

