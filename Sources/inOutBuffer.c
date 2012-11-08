/**
 *	@file network.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h> //sup ?
#include <libNetwork/ringBuffer.h>
#include <libNetwork/common.h>//!! modif
#include <libNetwork/inOutBuffer.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

network_inOutBuffer_t* newInOutBuffer(const network_paramNewInOutBuffer_t *pParam )
{
	network_inOutBuffer_t* pInOutBuff = malloc( sizeof(network_inOutBuffer_t));
	
	int keepAliveData = ntohl (0x50505055); //!!!!!!!!!!!!!!!!!!
	
	if(pInOutBuff)
	{
		pInOutBuff->id = pParam->id;
		pInOutBuff->pBuffer = newRingBufferWithOverwriting(	pParam->buffSize, pParam->buffCellSize,
															pParam->overwriting);
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
		
		if(pInOutBuff->pBuffer != NULL)
		{
			if(pInOutBuff->dataType == CMD_TYPE_KEEP_ALIVE)
			{
				ringBuffPushBack(pInOutBuff->pBuffer, &keepAliveData);
			}
		}
		else
		{
			deleteInOutBuffer(&pInOutBuff);
		}
    }
    
    return pInOutBuff;
}

void deleteInOutBuffer(network_inOutBuffer_t** ppInOutBuff)
{	
	network_inOutBuffer_t* pInOutBuff = NULL;
	
	if(ppInOutBuff)
	{
		pInOutBuff = *ppInOutBuff;
		
		if(pInOutBuff)
		{	
			sal_mutex_destroy(&(pInOutBuff->mutex));
			
			free(pInOutBuff->pBuffer);
		
			free(pInOutBuff);
		}
		*ppInOutBuff = NULL;
	}	

}

void inOutBufferAckReceived(network_inOutBuffer_t* pInOutBuff, int seqNum)
{
	sal_mutex_lock(&(pInOutBuff->mutex)); // !!! mutex ?
	
	if(pInOutBuff->isWaitAck && pInOutBuff->seqWaitAck == seqNum)
	{
		pInOutBuff->isWaitAck = 0;
		ringBuffPopFront( pInOutBuff->pBuffer, NULL );
	}
	sal_mutex_unlock(&(pInOutBuff->mutex)); // !!! mutex ?
}

network_inOutBuffer_t* inOutBufferWithId(	network_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id)
{
	network_inOutBuffer_t** it = pptabInOutBuff ;
	network_inOutBuffer_t** itEnd = pptabInOutBuff + (tabSize);
	network_inOutBuffer_t* pInOutBuffSearched = NULL;
	
	int find = 0;
	
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
	int isWaitAckCpy = 0;
	
	sal_mutex_lock(&(pInOutBuff->mutex));
	
	isWaitAckCpy = pInOutBuff->isWaitAck;
	
	sal_mutex_unlock(&(pInOutBuff->mutex));
	
	return isWaitAckCpy;
}
