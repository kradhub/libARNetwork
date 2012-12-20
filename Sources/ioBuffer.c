/**
 *	@file ioBuffer.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h>

#include <libNetwork/status.h>
#include "ringBuffer.h"
#include <libNetwork/frame.h>
#include <libNetwork/deportedData.h>
#include "ioBuffer.h"

/*****************************************
 * 
 * 			define :
 *
******************************************/

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_ioBuffer_t* NETWORK_NewIoBuffer( const network_paramNewIoBuffer_t* pParam )
{
	/** -- Create a new input or output buffer -- */
	
	/** local declarations */
	network_ioBuffer_t* pIoBuffer = NULL;
	int keepAliveData = 0x00;
    eNETWORK_Error error = NETWORK_OK;
	
	/** Create the input or output buffer in accordance with parameters set in pParam */
	pIoBuffer = malloc( sizeof(network_ioBuffer_t) );
	
	if( pIoBuffer )
	{ 
        /** Initialize to default values */
        pIoBuffer->pBuffer = NULL;
        sal_mutex_init( &(pIoBuffer->mutex) );
        
        if( NETWORK_ParamNewIoBufferCheck( pParam ) )
        {
            pIoBuffer->id = pParam->id;
            pIoBuffer->dataType = pParam->dataType;
            pIoBuffer->sendingWaitTimeMs = pParam->sendingWaitTimeMs;
            pIoBuffer->ackTimeoutMs = pParam->ackTimeoutMs;
            pIoBuffer->deportedData = pParam->deportedData;
        
            if(pParam->nbOfRetry >= 0)
            {
                pIoBuffer->nbOfRetry = pParam->nbOfRetry;
            }
            else
            {
                /** if nbOfRetry equal 0 disable the retry function with -1 value */
                pIoBuffer->nbOfRetry = -1;
            }
            
            //	timeoutcallback(network_ioBuffer_t* this)
		
            pIoBuffer->isWaitAck = 0;
            pIoBuffer->seqWaitAck = 0;
            pIoBuffer->waitTimeCount = pParam->sendingWaitTimeMs;
            pIoBuffer->ackWaitTimeCount = pParam->ackTimeoutMs;
            pIoBuffer->retryCount = 0;
		
            /** Create the RingBuffer */
            pIoBuffer->pBuffer = NETWORK_NewRingBufferWithOverwriting(	pParam->numberOfCell, pParam->cellSize,
                                                                pParam->overwriting);
            if(pIoBuffer->pBuffer != NULL)
            {
                /** if it is a keep alive buffer, push in the data send for keep alive */ 
                if( pIoBuffer->dataType == NETWORK_FRAME_TYPE_KEEP_ALIVE )
                {
                    NETWORK_RingBuffPushBack(pIoBuffer->pBuffer, &keepAliveData);
                }
            }
            else
            {
                error = NETWORK_ERROR_NEW_RINGBUFFER;
            }
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
        
        if( error )
		{
			/** delete the inOutput buffer if an error occurred */
            SAL_PRINT(PRINT_ERROR,"error: %d occurred \n", error );
			NETWORK_DeleteIoBuffer(&pIoBuffer);
		}
    }
    
    return pIoBuffer;
}

void NETWORK_DeleteIoBuffer( network_ioBuffer_t** ppIoBuffer )
{	
	/** -- Delete the input or output buffer -- */
	
	/** local declarations */
	network_ioBuffer_t* pIoBuffer = NULL;
	
	if(ppIoBuffer)
	{
		pIoBuffer = *ppIoBuffer;
		
		if(pIoBuffer)
		{	
			sal_mutex_destroy( &(pIoBuffer->mutex) );
			
            if(pIoBuffer->deportedData)
            {
                /** if it is a deportedData IoBuffer call all callback for free the data--*/
                NETWORK_IoBufferFreeAlldeportedData(pIoBuffer);
            }
            
			NETWORK_DeleteRingBuffer( &(pIoBuffer->pBuffer) );
		
			free(pIoBuffer);
            pIoBuffer = NULL;
		}
		*ppIoBuffer = NULL;
	}	

}

eNETWORK_Error NETWORK_IoBufferAckReceived( network_ioBuffer_t* pIoBuffer, int seqNum )
{
	/** -- Receive an acknowledgement to a inOutBuffer -- */ 
	
	/** local declarations */
	eNETWORK_Error error = NETWORK_OK;
	
	sal_mutex_lock( &(pIoBuffer->mutex) );
    
	/** delete the data if the sequence number received is same as the sequence number expected */
	if( pIoBuffer->isWaitAck && pIoBuffer->seqWaitAck == seqNum )
	{
		pIoBuffer->isWaitAck = 0;
        error = NETWORK_IoBufferDeleteData( pIoBuffer, NETWORK_CALLBACK_STATUS_SENT_WITH_ACK );
	}
    else
    {
        error = NETWORK_IOBUFFER_ERROR_BAD_ACK;
    }
	
	sal_mutex_unlock(&(pIoBuffer->mutex));
	
	return error;
}

network_ioBuffer_t* NETWORK_IoBufferFromId( network_ioBuffer_t** pptabInOutBuff,
												int tabSize, int id )
{
	/** -- Search a inOutBuffer from its identifier, in a table -- */
	
	/** local declarations */
	network_ioBuffer_t** it = pptabInOutBuff ;
	network_ioBuffer_t** itEnd = pptabInOutBuff + (tabSize);
	network_ioBuffer_t* pIoBufferSearched = NULL;
	int find = 0;
	
	/** for each inoutBuffer of the table check if the ID is the same as the ID searched */
	for(it = pptabInOutBuff ; ( it != itEnd ) && !find ; ++it )
	{
		if( (*it)->id == id)
		{
			pIoBufferSearched = *it;
			find = 1;
		}
	}
	
	return pIoBufferSearched;
}

int NETWORK_IoBufferIsWaitAck(	network_ioBuffer_t* pIoBuffer)
{
	/** -- Get if the inOutBuffer is waiting an acknowledgement -- */
	
	/** local declarations */
	int isWaitAckCpy = 0;
	
	sal_mutex_lock(&(pIoBuffer->mutex));
	
	isWaitAckCpy = pIoBuffer->isWaitAck;
	
	sal_mutex_unlock(&(pIoBuffer->mutex));
	
	return isWaitAckCpy;
}

eNETWORK_Error NETWORK_IoBufferFreeAlldeportedData( network_ioBuffer_t* pIoBuffer )
{
    /** -- call the callback of all deportedData with the NETWORK_CALLBACK_STATUS_FREE status --*/
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    eNETWORK_Error deleteError = NETWORK_OK;
    
    if( pIoBuffer->deportedData )
    {
        while( deleteError == NETWORK_OK )
        {
            deleteError = NETWORK_IoBufferDeleteData(pIoBuffer, NETWORK_CALLBACK_STATUS_FREE);
        }
        
        if(deleteError != NETWORK_ERROR_BUFFER_EMPTY)
        {
            error = deleteError;
        }
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

eNETWORK_Error NETWORK_IoBufferDeleteData( network_ioBuffer_t* pIoBuffer, int callbackStatus )
{
    /** -- delete the later data of the IoBuffer -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_DeportedData_t deportedDataTemp;
    
    /** pop the data sent*/
    if( pIoBuffer->deportedData )
    {
        error = NETWORK_RingBuffPopFront( pIoBuffer->pBuffer, &deportedDataTemp );
        if( error == NETWORK_OK)
        {
            deportedDataTemp.callback( pIoBuffer->id, deportedDataTemp.pData, callbackStatus);
        }
    }
    else
    {
        error = NETWORK_RingBuffPopFront(pIoBuffer->pBuffer, NULL);
    }
    
    return error;
}

void NETWORK_IoBufferFlush( network_ioBuffer_t* pIoBuffer )
{
    /** -- flush the IoBuffer -- */
    
    /** local declarations */
    eNETWORK_Error errorDataDel = NETWORK_OK;
    
    /**  delete all data */
    while(errorDataDel == NETWORK_OK)
    {
        errorDataDel = NETWORK_IoBufferDeleteData(pIoBuffer, NETWORK_CALLBACK_STATUS_FREE);
    }
    
    /**  reset */
    pIoBuffer->isWaitAck = 0;
    pIoBuffer->seqWaitAck = 0;
    pIoBuffer->waitTimeCount = pIoBuffer->sendingWaitTimeMs;
    pIoBuffer->ackWaitTimeCount = pIoBuffer->ackTimeoutMs;
    pIoBuffer->retryCount = 0;
}
