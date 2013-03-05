/**
 *  @file ARNETWORK_IOBuffer.c
 *  @brief input or output buffer, used by ARNetwork_Receiver or ARNetwork_Sender
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <stdlib.h>
#include <string.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Mutex.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include "ARNETWORK_RingBuffer.h"
#include <libARNetwork/ARNETWORK_Frame.h>
#include "ARNETWORK_DataDescriptor.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_IOBuffer.h"

/*****************************************
 * 
 *             define :
 *
******************************************/

#define ARNETWORK_IOBUFFER_TAG "ARNETWORK_IOBuffer"

/**
 *  @brief free the data pointed by the data descriptor
 *  @param IOBufferPtr Pointer on the IOBuffer
 *  @param dataDescriptorPtr Pointer on the data descriptor of the data to free
 *  @return error equal to ARNETWORK_OK if the data are correctly deleted otherwise see eARNETWORK_ERROR
**/
static inline eARNETWORK_ERROR ARNETWORK_IOBuffer_FreeData(ARNETWORK_IOBuffer_t *IOBufferPtr, ARNETWORK_DataDescriptor_t *dataDescriptorPtr)
{
    /** -- free the last data of the IOBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    if(dataDescriptorPtr->isUsingDataCopy)
    {
        /** if the data has been copied in the dataCopyRBuffer */
        /** pop data copy*/
        error = ARNETWORK_RingBuffer_PopFrontWithSize(IOBufferPtr->dataCopyRBufferPtr, NULL, dataDescriptorPtr->dataSize);
    }
    else
    {
        /** if the data is store out of the ARNetwork */
        /** callback with free status */
        if(dataDescriptorPtr->callback != NULL)
        {
            dataDescriptorPtr->callback(IOBufferPtr->ID, dataDescriptorPtr->dataPtr, dataDescriptorPtr->customData, ARNETWORK_MANAGER_CALLBACK_STATUS_FREE);
        }
    }
    
    return error;
}

/*****************************************
 * 
 *             implementation :
 *
******************************************/

ARNETWORK_IOBuffer_t* ARNETWORK_IOBuffer_New(const ARNETWORK_IOBufferParam_t *paramPtr)
{
    /** -- Create a new input or output buffer -- */
    
    /** local declarations */
    ARNETWORK_IOBuffer_t *IOBufferPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    /** Create the input or output buffer in accordance with parameters set in the ARNETWORK_IOBufferParam_t */
    IOBufferPtr = malloc( sizeof(ARNETWORK_IOBuffer_t) );
    
    if(IOBufferPtr != NULL)
    {
        /** Initialize to default values */
        IOBufferPtr->dataDescriptorRBufferPtr = NULL;
        IOBufferPtr->dataCopyRBufferPtr = NULL;
        ARSAL_Mutex_Init(&(IOBufferPtr->mutex));
        ARSAL_Sem_Init(&(IOBufferPtr->outputSem), 0, 0);
        
        if(ARNETWORK_IOBufferParam_Check(paramPtr))
        {
            IOBufferPtr->ID = paramPtr->ID;
            IOBufferPtr->dataType = paramPtr->dataType;
            IOBufferPtr->sendingWaitTimeMs = paramPtr->sendingWaitTimeMs;
            IOBufferPtr->ackTimeoutMs = paramPtr->ackTimeoutMs;
            
            if(paramPtr->numberOfRetry >= 0)
            {
                IOBufferPtr->numberOfRetry = paramPtr->numberOfRetry;
            }
            else
            {
                /** if numberOfRetry equal 0 disable the retry function with -1 value */
                IOBufferPtr->numberOfRetry = -1;
            }
            
            IOBufferPtr->isWaitAck = 0;
            IOBufferPtr->seqWaitAck = 0;
            IOBufferPtr->waitTimeCount = paramPtr->sendingWaitTimeMs;
            IOBufferPtr->ackWaitTimeCount = paramPtr->ackTimeoutMs;
            IOBufferPtr->retryCount = 0;
            
            /** Create the RingBuffer for the information of the data*/
            IOBufferPtr->dataDescriptorRBufferPtr = ARNETWORK_RingBuffer_NewWithOverwriting(paramPtr->numberOfCell, sizeof(ARNETWORK_DataDescriptor_t), paramPtr->isOverwriting);
            if(IOBufferPtr->dataDescriptorRBufferPtr == NULL)
            {
                error = ARNETWORK_ERROR_NEW_RINGBUFFER;
            }
            
            /** if the parameters have a size of data copy */ 
            if( (error == ARNETWORK_OK) && (paramPtr->dataCopyMaxSize > 0) )
            {
                /** Create the RingBuffer for the copy of the data*/
                IOBufferPtr->dataCopyRBufferPtr = ARNETWORK_RingBuffer_NewWithOverwriting(paramPtr->numberOfCell, paramPtr->dataCopyMaxSize, paramPtr->isOverwriting);
                if(IOBufferPtr->dataCopyRBufferPtr == NULL)
                {
                    error = ARNETWORK_ERROR_NEW_BUFFER;
                }
            }
        }
        else
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        if(error != ARNETWORK_OK)
        {
            /** delete the inOutput buffer if an error occurred */
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_IOBUFFER_TAG,"error: %d occurred \n", error);
            ARNETWORK_IOBuffer_Delete(&IOBufferPtr);
        }
    }
    
    return IOBufferPtr;
}

void ARNETWORK_IOBuffer_Delete(ARNETWORK_IOBuffer_t **IOBufferPtrAddr)
{
    /** -- Delete the input or output buffer -- */
    
    /** local declarations */
    ARNETWORK_IOBuffer_t *IOBufferPtr = NULL;
    
    if(IOBufferPtrAddr != NULL)
    {
        IOBufferPtr = *IOBufferPtrAddr;
        
        if(IOBufferPtr != NULL)
        {
            ARSAL_Mutex_Destroy(&(IOBufferPtr->mutex));
            ARSAL_Sem_Destroy(&(IOBufferPtr->outputSem));
            
            ARNETWORK_IOBuffer_CancelAllData(IOBufferPtr);
            
            ARNETWORK_RingBuffer_Delete(&(IOBufferPtr->dataDescriptorRBufferPtr));
            ARNETWORK_RingBuffer_Delete(&(IOBufferPtr->dataCopyRBufferPtr));
            
            free(IOBufferPtr);
            IOBufferPtr = NULL;
        }
        *IOBufferPtrAddr = NULL;
    }
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_AckReceived(ARNETWORK_IOBuffer_t *IOBufferPtr, int seqNumber)
{
    /** -- Receive an acknowledgement to a IOBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    ARSAL_Mutex_Lock(&(IOBufferPtr->mutex));
    
    /** delete the data if the sequence number received is same as the sequence number expected */
    if(IOBufferPtr->isWaitAck && IOBufferPtr->seqWaitAck == seqNumber)
    {
        IOBufferPtr->isWaitAck = 0;
        error = ARNETWORK_IOBuffer_PopDataWithCallBack(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_ACK_RECEIVED);
    }
    else
    {
        error = ARNETWORK_ERROR_IOBUFFER_BAD_ACK;
    }
    
    ARSAL_Mutex_Unlock(&(IOBufferPtr->mutex));
    
    return error;
}

int ARNETWORK_IOBuffer_IsWaitAck(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    /** -- Get if the IOBuffer is waiting an acknowledgement -- */
    
    /** local declarations */
    int isWaitAckCpy = 0;
    
    ARSAL_Mutex_Lock(&(IOBufferPtr->mutex));
    
    isWaitAckCpy = IOBufferPtr->isWaitAck;
    
    ARSAL_Mutex_Unlock(&(IOBufferPtr->mutex));
    
    return isWaitAckCpy;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_CancelAllData(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    /** -- cancel all remaining data */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORK_ERROR deleteError = ARNETWORK_OK;
    
    /** pop all data with the ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL status --*/ 
    while(deleteError == ARNETWORK_OK)
    {
        deleteError = ARNETWORK_IOBuffer_PopDataWithCallBack(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL);
    }
    
    if(deleteError != ARNETWORK_ERROR_BUFFER_EMPTY)
    {
        error = deleteError;
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_PopData(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    /** -- Pop the later data of the IOBuffer and free it -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_DataDescriptor_t dataDescriptor;
    
    /** pop and get the data descriptor */
    error = ARNETWORK_RingBuffer_PopFront(IOBufferPtr->dataDescriptorRBufferPtr, (uint8_t*) &dataDescriptor);
    if(error == ARNETWORK_OK)
    {
        /** free data */
        error = ARNETWORK_IOBuffer_FreeData(IOBufferPtr, &dataDescriptor);
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_PopDataWithCallBack(ARNETWORK_IOBuffer_t *IOBufferPtr, eARNETWORK_MANAGER_CALLBACK_STATUS callbackStatus)
{
    /** -- Pop the later data of the IOBuffer with callback calling and free it -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_DataDescriptor_t dataDescriptor;
    
    /** pop and get the data descriptor */
    error = ARNETWORK_RingBuffer_PopFront(IOBufferPtr->dataDescriptorRBufferPtr, (uint8_t*) &dataDescriptor);
    if(error == ARNETWORK_OK)
    {
        /** callback with the reason of the data popping */
        if(dataDescriptor.callback != NULL)
        {
            dataDescriptor.callback(IOBufferPtr->ID, dataDescriptor.dataPtr, dataDescriptor.customData, callbackStatus);
        }
        
        /** free data */
        error = ARNETWORK_IOBuffer_FreeData(IOBufferPtr, &dataDescriptor);
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_Flush(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    /** -- Flush the IoBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    ARSAL_Mutex_Lock(&(IOBufferPtr->mutex));
    
    /**  delete all data */
    while(error == ARNETWORK_OK)
    {
        error = ARNETWORK_IOBuffer_PopDataWithCallBack(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL);
    }
    
    /** if the error occurred is "buffer empty" there is no error */
    if(error == ARNETWORK_ERROR_BUFFER_EMPTY)
    {
        error = ARNETWORK_OK;
    }
    
    /** state reset */
    IOBufferPtr->isWaitAck = 0;
    IOBufferPtr->seqWaitAck = 0;
    IOBufferPtr->waitTimeCount = IOBufferPtr->sendingWaitTimeMs;
    IOBufferPtr->ackWaitTimeCount = IOBufferPtr->ackTimeoutMs;
    IOBufferPtr->retryCount = 0;
    
    /** reset semaphore */
    ARSAL_Sem_Destroy(&(IOBufferPtr->outputSem));
    ARSAL_Sem_Init(&(IOBufferPtr->outputSem), 0, 0);
    
    ARSAL_Mutex_Unlock(&(IOBufferPtr->mutex));
    
    return error; 
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_AddData(ARNETWORK_IOBuffer_t *IOBufferPtr, uint8_t *dataPtr, int dataSize, void *customData, ARNETWORK_Manager_Callback_t callback, int doDataCopy)
{
    /** -- Add data in a IOBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_DataDescriptor_t dataDescriptor;
    int numberOfFreeCell = 0;
    
    /** initialize dataDescriptor */
    dataDescriptor.dataPtr = dataPtr;
    dataDescriptor.dataSize = dataSize;
    dataDescriptor.customData = customData;
    dataDescriptor.callback = callback;
    dataDescriptor.isUsingDataCopy = 0;
    
    /** get the number of free cell */
    numberOfFreeCell = ARNETWORK_RingBuffer_GetFreeCellNumber(IOBufferPtr->dataDescriptorRBufferPtr);
    
    /** if the buffer is not full or it is overwriting */
    if( (IOBufferPtr->dataDescriptorRBufferPtr->isOverwriting == 1) || (numberOfFreeCell > 0) )
    {
        /** if there is overwriting */
        if(numberOfFreeCell == 0)
        {
            /** if the buffer is full, cancel the data lost by the overwriting */
            /** Delete the data Overwritten */
            error = ARNETWORK_IOBuffer_PopDataWithCallBack(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL);
        }
    
        /** if data copy is asked */
        if( (error == ARNETWORK_OK) && (doDataCopy) )
        {
            /** check if the IOBuffer can copy and if the size of the copy buffer is large enough */
            if( (ARNETWORK_IOBuffer_CanCopyData(IOBufferPtr)) && (dataSize <= IOBufferPtr->dataCopyRBufferPtr->cellSize) )
            {
                /** copy data in the dataCopyRBufferPtr and get the address of the data copy in descDataPtr */
                error =  ARNETWORK_RingBuffer_PushBackWithSize(IOBufferPtr->dataCopyRBufferPtr, dataPtr, dataSize, &(dataDescriptor.dataPtr));
                
                /** set the flag to indicate the copy of the data */
                dataDescriptor.isUsingDataCopy = 1;
            }
            else
            {
                error = ARNETWORK_ERROR_BAD_PARAMETER;
            }
        }
        
        if(error == ARNETWORK_OK)
        {
            /** push dataDescriptor in the IOBufferPtr */
            error = ARNETWORK_RingBuffer_PushBack(IOBufferPtr->dataDescriptorRBufferPtr, (uint8_t*) &dataDescriptor);
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_SIZE;
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_ReadData(ARNETWORK_IOBuffer_t *IOBufferPtr, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr)
{
    /** -- read data received in a IOBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_DataDescriptor_t dataDescriptor;
    int readSize = 0;
    
    /** get data descriptor*/
    error = ARNETWORK_RingBuffer_Front(IOBufferPtr->dataDescriptorRBufferPtr, (uint8_t*) &dataDescriptor);
    
    if( error == ARNETWORK_OK )
    {
        /** data size check */
        if(dataDescriptor.dataSize <= dataLimitSize)
        {
            /** data copy */
            memcpy(dataPtr, dataDescriptor.dataPtr, dataDescriptor.dataSize);
            
            /** set size of data read */
            readSize = dataDescriptor.dataSize;
            
            /** pop the data */
            ARNETWORK_IOBuffer_PopData(IOBufferPtr);
        }
        else
        {
            error = ARNETWORK_ERROR_BUFFER_SIZE;
        }
    }
    
    /** return the size of the data read */
    if(readSizePtr != NULL)
    {
        *readSizePtr = readSize;
    }
    
    return error;
}
