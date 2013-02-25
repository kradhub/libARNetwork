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
#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Mutex.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include "ARNETWORK_RingBuffer.h"
#include <libARNetwork/ARNETWORK_Frame.h>
#include "ARNETWORK_VariableSizeData.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_IOBuffer.h"

/*****************************************
 * 
 *             define :
 *
******************************************/

#define ARNETWORK_IOBUFFER_TAG "ARNETWORK_IOBuffer"

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
    int keepAliveData = 0x00;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    /** Create the input or output buffer in accordance with parameters set in the ARNETWORK_IOBufferParam_t */
    IOBufferPtr = malloc( sizeof(ARNETWORK_IOBuffer_t) );
    
    if(IOBufferPtr != NULL)
    {
        /** Initialize to default values */
        IOBufferPtr->bufferPtr = NULL;
        ARSAL_Mutex_Init(&(IOBufferPtr->mutex));
        ARSAL_Sem_Init(&(IOBufferPtr->outputSem), 0, 0);
        
        if(ARNETWORK_IOBufferParam_Check(paramPtr))
        {
            IOBufferPtr->ID = paramPtr->ID;
            IOBufferPtr->dataType = paramPtr->dataType;
            IOBufferPtr->sendingWaitTimeMs = paramPtr->sendingWaitTimeMs;
            IOBufferPtr->ackTimeoutMs = paramPtr->ackTimeoutMs;
            IOBufferPtr->isUsingVariableSizeData = paramPtr->isUsingVariableSizeData;
            
            if(paramPtr->numberOfRetry >= 0)
            {
                IOBufferPtr->numberOfRetry = paramPtr->numberOfRetry;
            }
            else
            {
                /** if numberOfRetry equal 0 disable the retry function with -1 value */
                IOBufferPtr->numberOfRetry = -1;
            }
            
            //    timeoutcallback(ARNETWORK_IOBuffer_t* this)
            
            IOBufferPtr->isWaitAck = 0;
            IOBufferPtr->seqWaitAck = 0;
            IOBufferPtr->waitTimeCount = paramPtr->sendingWaitTimeMs;
            IOBufferPtr->ackWaitTimeCount = paramPtr->ackTimeoutMs;
            IOBufferPtr->retryCount = 0;
            
            /** Create the RingBuffer */
            IOBufferPtr->bufferPtr = ARNETWORK_RingBuffer_NewWithOverwriting(paramPtr->numberOfCell, paramPtr->cellSize, paramPtr->isOverwriting);
            if(IOBufferPtr->bufferPtr != NULL)
            {
                /** if it is a keep alive buffer, push in the data send for keep alive */ 
                if(IOBufferPtr->dataType == ARNETWORK_FRAME_TYPE_KEEP_ALIVE)
                {
                    ARNETWORK_RingBuffer_PushBack(IOBufferPtr->bufferPtr, (uint8_t*) &keepAliveData);
                }
            }
            else
            {
                error = ARNETWORK_ERROR_NEW_RINGBUFFER;
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
            
            if(IOBufferPtr->isUsingVariableSizeData)
            {
                /** if the IOBuffer uses variable size data, call all callback for free the data */
                ARNETWORK_IOBuffer_FreeAllVariableSizeData(IOBufferPtr);
            }
            
            ARNETWORK_RingBuffer_Delete(&(IOBufferPtr->bufferPtr));
            
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
        error = ARNETWORK_IOBuffer_DeleteData(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_SENT_WITH_ACK);
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

eARNETWORK_ERROR ARNETWORK_IOBuffer_FreeAllVariableSizeData(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    /** -- call the callback of all variable size data with the ARNETWORK_MANAGER_CALLBACK_STATUS_FREE status --*/
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORK_ERROR deleteError = ARNETWORK_OK;
    
    if(IOBufferPtr->isUsingVariableSizeData)
    {
        while(deleteError == ARNETWORK_OK)
        {
            deleteError = ARNETWORK_IOBuffer_DeleteData(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_FREE);
        }
        
        if(deleteError != ARNETWORK_ERROR_BUFFER_EMPTY)
        {
            error = deleteError;
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_DeleteData(ARNETWORK_IOBuffer_t *IOBufferPtr, int callbackStatus)
{
    /** -- delete the later data of the IoBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_VariableSizeData_t variableSizeDataTemp;
    
    /** pop the data sent*/
    if( IOBufferPtr->isUsingVariableSizeData )
    {
        /** IOBuffer using variable size data */
        error = ARNETWORK_RingBuffer_PopFront(IOBufferPtr->bufferPtr, (uint8_t*) &variableSizeDataTemp);
        if(error == ARNETWORK_OK)
        {
            variableSizeDataTemp.callback(IOBufferPtr->ID, variableSizeDataTemp.dataPtr, variableSizeDataTemp.customData, callbackStatus);
        }
    }
    else
    {
        /** IOBuffer using fixed size data */
        error = ARNETWORK_RingBuffer_PopFront(IOBufferPtr->bufferPtr, NULL);
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_IOBuffer_Flush(ARNETWORK_IOBuffer_t *IOBufferPtr)
{
    /** -- flush the IoBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    ARSAL_Mutex_Lock(&(IOBufferPtr->mutex));
    
    /**  delete all data */
    while(error == ARNETWORK_OK)
    {
        error = ARNETWORK_IOBuffer_DeleteData(IOBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_FREE);
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

