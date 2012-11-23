/**
 *	@file network.c
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

#define INOUTBUFFER_ID_DEFAULT -1
#define INOUTBUFFER_DATATYPE_DEFAULT CMD_TYPE_DEFAULT
#define INOUTBUFFER_SENDINGWAITTIME_DEFAULT 1
#define INOUTBUFFER_ACKTILEOUTMS_DEFAULT 0
#define INOUTBUFFER_NBOFRETRY_DEFAULT 0
#define INOUTBUFFER_BUFFSIZE_DEFAULT 1
#define INOUTBUFFER_BUFFCELLSIZE_DEFAULT 1
#define INOUTBUFFER_OVERWRITING_DEFAULT 0

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

void paramNewInOutBufferDefaultInit(network_paramNewInOutBuffer_t *pParam)
{
	/** -- initialization of the paramNewInOutBuffer with default parameters -- */
	
	pParam->id = INOUTBUFFER_ID_DEFAULT;
    pParam->dataType = INOUTBUFFER_DATATYPE_DEFAULT;	
    pParam->sendingWaitTime = INOUTBUFFER_SENDINGWAITTIME_DEFAULT;
    pParam->ackTimeoutMs = INOUTBUFFER_ACKTILEOUTMS_DEFAULT;
    pParam->nbOfRetry = INOUTBUFFER_NBOFRETRY_DEFAULT;
    
    pParam->buffSize = INOUTBUFFER_BUFFSIZE_DEFAULT;	
    pParam->buffCellSize = INOUTBUFFER_BUFFCELLSIZE_DEFAULT;
    pParam->overwriting = INOUTBUFFER_OVERWRITING_DEFAULT;
}


network_inOutBuffer_t* newInOutBuffer( const network_paramNewInOutBuffer_t *pParam )
{
	/** -- Create a new input or output buffer -- */
	
	/** local declarations */
	network_inOutBuffer_t* pInOutBuff = NULL;
	int keepAliveData = 0x00;
	
	/** Create the input or output buffer in accordance with parameters set in pParam */
	pInOutBuff = malloc( sizeof(network_inOutBuffer_t) );
	
	if( pInOutBuff )
	{ 
		pInOutBuff->id = pParam->id;
		pInOutBuff->dataType = pParam->dataType;
		pInOutBuff->sendingWaitTime = pParam->sendingWaitTime;
		pInOutBuff->ackTimeoutMs = pParam->ackTimeoutMs;
		pInOutBuff->nbOfRetry = pParam->nbOfRetry;
		//	timeoutCallback(network_inOutBuffer_t* this)
		
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

void deleteInOutBuffer( network_inOutBuffer_t** ppInOutBuff )
{	
	/** -- Delete the input or output buffer -- */
	
	/** local declarations */
	network_inOutBuffer_t* pInOutBuff = NULL;
	
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

int inOutBufferAckReceived( network_inOutBuffer_t* pInOutBuff, int seqNum )
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

network_inOutBuffer_t* inOutBufferWithId( network_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id )
{
	/** -- Search a inOutBuffer with its identifier, in a table -- */
	
	/** local declarations */
	network_inOutBuffer_t** it = pptabInOutBuff ;
	network_inOutBuffer_t** itEnd = pptabInOutBuff + (tabSize);
	network_inOutBuffer_t* pInOutBuffSearched = NULL;
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

int inOutBuffeIsWaitAck(	network_inOutBuffer_t* pInOutBuff)
{
	/** -- Get if the inOutBuffer is waiting an acknowledgement -- */
	
	/** local declarations */
	int isWaitAckCpy = 0;
	
	sal_mutex_lock(&(pInOutBuff->mutex));
	
	isWaitAckCpy = pInOutBuff->isWaitAck;
	
	sal_mutex_unlock(&(pInOutBuff->mutex));
	
	return isWaitAckCpy;
}
