/**
 *	@file NETWORK_IoBuffer.c
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
#include <libNetwork/ringBuffer.h>
#include <libNetwork/common.h>
#include <libNetwork/inOutBuffer.h>

/*****************************************
 * 
 * 			define :
 *
******************************************/

#define NETWORK_IOBUFFER_ID_DEFAULT -1
#define NETWORK_IOBUFFER_DATATYPE_DEFAULT CMD_TYPE_DEFAULT
#define NETWORK_IOBUFFER_SENDINGWAITTIME_DEFAULT 1
#define NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT 0
#define NETWORK_IOBUFFER_NBOFRETRY_DEFAULT 0
#define NETWORK_IOBUFFER_BUFFSIZE_DEFAULT 1
#define NETWORK_IOBUFFER_BUFFCELLSIZE_DEFAULT 1
#define NETWORK_IOBUFFER_OVERWRITING_DEFAULT 0

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

void paramNewInOutBufferDefaultInit(network_paramNewInOutBuffer_t *pParam)
{
	/** -- initialization of the paramNewInOutBuffer with default parameters -- */
	
	pParam->id = NETWORK_IOBUFFER_ID_DEFAULT;
    pParam->dataType = NETWORK_IOBUFFER_DATATYPE_DEFAULT;	
    pParam->sendingWaitTime = NETWORK_IOBUFFER_SENDINGWAITTIME_DEFAULT;
    pParam->ackTimeoutMs = NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT;
    pParam->nbOfRetry = NETWORK_IOBUFFER_NBOFRETRY_DEFAULT;
    
    pParam->buffSize = NETWORK_IOBUFFER_BUFFSIZE_DEFAULT;	
    pParam->buffCellSize = NETWORK_IOBUFFER_BUFFCELLSIZE_DEFAULT;
    pParam->overwriting = NETWORK_IOBUFFER_OVERWRITING_DEFAULT;
}


network_ioBuffer_t* newInOutBuffer( const network_paramNewInOutBuffer_t *pParam )
{
	/** -- Create a new input or output buffer -- */
	
	/** local declarations */
	network_ioBuffer_t* pInOutBuff = NULL;
	int keepAliveData = 0x00;
	
	/** Create the input or output buffer in accordance with parameters set in pParam */
	pInOutBuff = malloc( sizeof(network_ioBuffer_t) );
	
	if( pInOutBuff )
	{ 
		pInOutBuff->id = pParam->id;
		pInOutBuff->dataType = pParam->dataType;
		pInOutBuff->sendingWaitTime = pParam->sendingWaitTime;
		pInOutBuff->ackTimeoutMs = pParam->ackTimeoutMs;
		pInOutBuff->nbOfRetry = pParam->nbOfRetry;
		//	timeoutCallback(network_ioBuffer_t* this)
		
		pInOutBuff->isWaitAck = 0;
		pInOutBuff->seqWaitAck = 0;
		pInOutBuff->waitTimeCount = pParam->sendingWaitTime;
		pInOutBuff->ackWaitTimeCount = pParam->ackTimeoutMs;
		pInOutBuff->retryCount = 0;
		
		sal_mutex_init( &(pInOutBuff->mutex) );
		
		/** Create the RingBuffer */
		pInOutBuff->pBuffer = newRingBufferWithOverwriting(	pParam->buffSize, pParam->buffCellSize,
															pParam->overwriting);
															
		if(pInOutBuff->pBuffer != NULL)
		{
			/** if it is a keep alive buffer, push in the data send for keep alive */ 
			if( pInOutBuff->dataType == CMD_TYPE_KEEP_ALIVE )
			{
				ringBuffPushBack(pInOutBuff->pBuffer, &keepAliveData);
			}
		}
		else
		{
			/** delete the inOutput buffer if an error occurred */
			deleteInOutBuffer(&pInOutBuff);
		}
    }
    
    return pInOutBuff;
}

void deleteInOutBuffer( network_ioBuffer_t** ppInOutBuff )
{	
	/** -- Delete the input or output buffer -- */
	
	/** local declarations */
	network_ioBuffer_t* pInOutBuff = NULL;
	
	if(ppInOutBuff)
	{
		pInOutBuff = *ppInOutBuff;
		
		if(pInOutBuff)
		{	
			sal_mutex_destroy( &(pInOutBuff->mutex) );
			
			deleteRingBuffer( &(pInOutBuff->pBuffer) );
		
			free(pInOutBuff);
		}
		*ppInOutBuff = NULL;
	}	

}

int inOutBufferAckReceived( network_ioBuffer_t* pInOutBuff, int seqNum )
{
	/** -- Receive an acknowledgement to a inOutBuffer -- */ 
	
	/** local declarations */
	int error = 1;
	
	sal_mutex_lock( &(pInOutBuff->mutex) );
	
	/** delete the data if the sequence number received is same as the sequence number expected */
	if( pInOutBuff->isWaitAck && pInOutBuff->seqWaitAck == seqNum )
	{
		pInOutBuff->isWaitAck = 0;
		ringBuffPopFront( pInOutBuff->pBuffer, NULL );
		error = 0;
	}
	
	sal_mutex_unlock(&(pInOutBuff->mutex));
	
	return error;
}

network_ioBuffer_t* inOutBufferWithId( network_ioBuffer_t** pptabInOutBuff,
												int tabSize, int id )
{
	/** -- Search a inOutBuffer with its identifier, in a table -- */
	
	/** local declarations */
	network_ioBuffer_t** it = pptabInOutBuff ;
	network_ioBuffer_t** itEnd = pptabInOutBuff + (tabSize);
	network_ioBuffer_t* pInOutBuffSearched = NULL;
	int find = 0;
	
	/** for each inoutBuffer of the table check if the ID is the same as the ID searched */
	for(it = pptabInOutBuff ; ( it != itEnd ) && !find ; ++it )
	{
		if( (*it)->id == id)
		{
			pInOutBuffSearched = *it;
			find = 1;
		}
	}
	
	return pInOutBuffSearched;
}

int inOutBuffeIsWaitAck(	network_ioBuffer_t* pInOutBuff)
{
	/** -- Get if the inOutBuffer is waiting an acknowledgement -- */
	
	/** local declarations */
	int isWaitAckCpy = 0;
	
	sal_mutex_lock(&(pInOutBuff->mutex));
	
	isWaitAckCpy = pInOutBuff->isWaitAck;
	
	sal_mutex_unlock(&(pInOutBuff->mutex));
	
	return isWaitAckCpy;
}
