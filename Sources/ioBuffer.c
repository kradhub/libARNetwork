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

#include <libNetwork/error.h>
#include <libNetwork/ringBuffer.h>
#include <libNetwork/frame.h>
#include <libNetwork/deportedData.h>
#include <libNetwork/ioBuffer.h>

/*****************************************
 * 
 * 			define :
 *
******************************************/

#define NETWORK_IOBUFFER_ID_DEFAULT -1
#define NETWORK_IOBUFFER_DATATYPE_DEFAULT network_frame_t_TYPE_UNINITIALIZED
#define NETWORK_IOBUFFER_SENDINGWAITTIME_DEFAULT 1
#define NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT -1
#define NETWORK_IOBUFFER_NBOFRETRY_DEFAULT -1
#define NETWORK_IOBUFFER_BUFFSIZE_DEFAULT 0
#define NETWORK_IOBUFFER_BUFFCELLSIZE_DEFAULT 0
#define NETWORK_IOBUFFER_OVERWRITING_DEFAULT 0
#define NETWORK_IOBUFFER_deportedData_DEFAULT 0

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

void NETWORK_ParamNewIoBufferDefaultInit(network_paramNewIoBuffer_t *pParam)
{
	/** -- initialization of the paramNewIoBuffer with default parameters -- */
	
	pParam->id = NETWORK_IOBUFFER_ID_DEFAULT;
    pParam->dataType = NETWORK_IOBUFFER_DATATYPE_DEFAULT;	
    pParam->sendingWaitTime = NETWORK_IOBUFFER_SENDINGWAITTIME_DEFAULT;
    pParam->ackTimeoutMs = NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT;
    pParam->nbOfRetry = NETWORK_IOBUFFER_NBOFRETRY_DEFAULT;
    
    pParam->buffSize = NETWORK_IOBUFFER_BUFFSIZE_DEFAULT;	
    pParam->buffCellSize = NETWORK_IOBUFFER_BUFFCELLSIZE_DEFAULT;
    pParam->overwriting = NETWORK_IOBUFFER_OVERWRITING_DEFAULT;
    pParam->deportedData = NETWORK_IOBUFFER_deportedData_DEFAULT;
}

int NETWORK_ParamNewIoBufferCheck( const network_paramNewIoBuffer_t* pParam )
{
    /** -- check the values of the paramNewIoBuffer -- */
    
    /** local declarations */
    int ok = 0;
    
    if( pParam->id > NETWORK_IOBUFFER_ID_DEFAULT && 
        pParam->dataType != network_frame_t_TYPE_UNINITIALIZED &&
        pParam->sendingWaitTime > 0 &&
        pParam->ackTimeoutMs >= -1 &&
        pParam->nbOfRetry >= -1 &&
        pParam->buffSize > 0 &&
        pParam->buffCellSize > 0)
    {
        ok = 1;
    }
    else
    {
        sal_print(PRINT_ERROR," parameters for new IoBuffer are not correct. \n \
values expected: \n \
    - id > -1 \n \
    - dataType != network_frame_t_TYPE_UNINITIALIZED \n \
    - sendingWaitTime > 0 \n \
    - ackTimeoutMs > 0 or -1 if not used \n \
    - nbOfRetry > 0 or -1 if not used  \n \
    - buffSize > 0 \n \
    - buffCellSize > 0 \n \
    - overwriting = 0 or 1 \n");
    }
    
   return ok; 
}

network_ioBuffer_t* NETWORK_NewIoBuffer( const network_paramNewIoBuffer_t* pParam )
{
	/** -- Create a new input or output buffer -- */
	
	/** local declarations */
	network_ioBuffer_t* pIoBuffer = NULL;
	int keepAliveData = 0x00;
    int error = NETWORK_OK;
	
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
            pIoBuffer->sendingWaitTime = pParam->sendingWaitTime;
            pIoBuffer->ackTimeoutMs = pParam->ackTimeoutMs;
            pIoBuffer->deportedData = pParam->deportedData;
        
            if(pParam->nbOfRetry > 0)
            {
                pIoBuffer->nbOfRetry = pParam->nbOfRetry;
            }
            else
            {
                /** if nbOfRetry equal 0 disable the retry function with -1 value */
                pIoBuffer->nbOfRetry = -1;
            }
            //	timeoutCallback(network_ioBuffer_t* this)
		
            pIoBuffer->isWaitAck = 0;
            pIoBuffer->seqWaitAck = 0;
            pIoBuffer->waitTimeCount = pParam->sendingWaitTime;
            pIoBuffer->ackWaitTimeCount = pParam->ackTimeoutMs;
            pIoBuffer->retryCount = 0;
		
            /** Create the RingBuffer */
            pIoBuffer->pBuffer = NETWORK_NewRingBufferWithOverwriting(	pParam->buffSize, pParam->buffCellSize,
                                                                pParam->overwriting);
            if(pIoBuffer->pBuffer != NULL)
            {
                /** if it is a keep alive buffer, push in the data send for keep alive */ 
                if( pIoBuffer->dataType == network_frame_t_TYPE_KEEP_ALIVE )
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
            sal_print(PRINT_ERROR,"error: %d occurred \n", error );
			NETWORK_DeleteIotBuffer(&pIoBuffer);
		}
    }
    
    return pIoBuffer;
}

void NETWORK_DeleteIotBuffer( network_ioBuffer_t** ppIoBuffer )
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

int NETWORK_IoBufferAckReceived( network_ioBuffer_t* pIoBuffer, int seqNum )
{
	/** -- Receive an acknowledgement to a inOutBuffer -- */ 
	
	/** local declarations */
	int error = NETWORK_OK;
	
	sal_mutex_lock( &(pIoBuffer->mutex) );
    
	/** delete the data if the sequence number received is same as the sequence number expected */
	if( pIoBuffer->isWaitAck && pIoBuffer->seqWaitAck == seqNum )
	{
		pIoBuffer->isWaitAck = 0;
        error = NETWORK_IoBufferDeleteData( pIoBuffer, NETWORK_DEPORTEDDATA_CALLBACK_SENT_WITH_ACK );
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

int NETWORK_IoBufferFreeAlldeportedData( network_ioBuffer_t* pIoBuffer )
{
    /** -- call the callback of all deportedData with the NETWORK_DEPORTEDDATA_CALLBACK_FREE status --*/
    
    /** local declarations */
    int error = NETWORK_OK;
    int deleteError = NETWORK_OK;
    
    if( pIoBuffer->deportedData )
    {
        while( deleteError == NETWORK_OK )
        {
            deleteError = NETWORK_IoBufferDeleteData(pIoBuffer, NETWORK_DEPORTEDDATA_CALLBACK_FREE);
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

int NETWORK_IoBufferDeleteData( network_ioBuffer_t* pIoBuffer, int callBackStatus )
{
    /** -- delete the later data of the IoBuffer - */
    
    /** local declarations */
    int error = NETWORK_OK;
    network_DeportedData_t deportedDataTemp;
    
    /** pop the data sent*/
    if( pIoBuffer->deportedData )
    {
        error = NETWORK_RingBuffPopFront( pIoBuffer->pBuffer, &deportedDataTemp );
        if( error == NETWORK_OK)
        {
            deportedDataTemp.callBack( pIoBuffer->id, deportedDataTemp.pData, callBackStatus);
        }
    }
    else
    {
        error = NETWORK_RingBuffPopFront(pIoBuffer->pBuffer, NULL);
    }
    
    return error;
}
