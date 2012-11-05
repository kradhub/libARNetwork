/**
 *	@file netWork.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h> //sup ?
#include <libNetWork/ringBuffer.h>
#include <libNetWork/common.h>//!! modif
#include <libNetWork/inOutBuffer.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

netWork_inOutBuffer_t* newInOutBuffer(const netWork_paramNewInOutBuffer_t *pParam )
{
	netWork_inOutBuffer_t* pInOutBuff = malloc( sizeof(netWork_inOutBuffer_t));
	sal_print(PRINT_WARNING,"newInOutBuffer \n"); //!! sup
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
		//	timeoutCallback(netWork_inOutBuffer_t* this)
		
		pInOutBuff->isWaitAck = 0;
		pInOutBuff->seqWaitAck = 0;
		pInOutBuff->waitTimeCount = pParam->sendingWaitTime;
		pInOutBuff->ackWaitTimeCount = pParam->ackTimeoutMs;
		pInOutBuff->retryCount = 0;
		
		sal_mutex_init( &(pInOutBuff->mutex) );
		
		sal_print(PRINT_WARNING,"id :%d dataType :%d sendingWaitTime :%d pBuffer :%p  \n",
								pInOutBuff->id, pInOutBuff->dataType,
								pInOutBuff->sendingWaitTime, pInOutBuff->pBuffer ); //!! sup
		
		if(pInOutBuff->pBuffer != NULL)
		{
			if(pInOutBuff->dataType == CMD_TYPE_KEEP_ALIVE)
			{
				ringBuffPushBack(pInOutBuff->pBuffer, &keepAliveData);
			}
		}
		else
		{
			deleteInOutBuffer(&pInOutBuff); //free(pInOutBuff);
		}
    }
    
    return pInOutBuff;
}

void deleteInOutBuffer(netWork_inOutBuffer_t** ppInOutBuff)
{	
	netWork_inOutBuffer_t* pInOutBuff = NULL;
	
	if(ppInOutBuff)
	{
		pInOutBuff = *ppInOutBuff;
		
		if(pInOutBuff)
		{
			sal_print(PRINT_WARNING,"deleteInOutBuffer \n");//!! sup
			
			sal_mutex_destroy(&(pInOutBuff->mutex));
			
			free(pInOutBuff->pBuffer);
		
			free(pInOutBuff);
		}
		*ppInOutBuff = NULL;
	}	

}

void inOutBufferAckReceived(netWork_inOutBuffer_t* pInOutBuff, int seqNum)
{
	sal_mutex_lock(&(pInOutBuff->mutex)); // !!! mutex ?
	
	if(pInOutBuff->isWaitAck && pInOutBuff->seqWaitAck == seqNum)
	{
		pInOutBuff->isWaitAck = 0;
		ringBuffPopFront( pInOutBuff->pBuffer, NULL );
	}
	sal_mutex_unlock(&(pInOutBuff->mutex)); // !!! mutex ?
}

netWork_inOutBuffer_t* inOutBufferWithId(	netWork_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id)
{
	netWork_inOutBuffer_t** it = pptabInOutBuff ;
	netWork_inOutBuffer_t** itEnd = pptabInOutBuff + (tabSize);
	netWork_inOutBuffer_t* pInOutBuffSearched = NULL;
	
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

int inOutBuffeIsWaitAck(	netWork_inOutBuffer_t* pInOutBuff)
{
	int isWaitAckCpy = 0;
	sal_mutex_lock(&(pInOutBuff->mutex));
	
	isWaitAckCpy = pInOutBuff->isWaitAck;
	
	sal_mutex_unlock(&(pInOutBuff->mutex));
	
	return isWaitAckCpy;
}
