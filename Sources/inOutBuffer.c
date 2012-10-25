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
	
	if(pInOutBuff)
	{
		pInOutBuff->id = pParam->id;
		pInOutBuff->pBuffer = newRingBuffer(pParam->buffSize, pParam->buffCellSize);
		pInOutBuff->needAck = pParam->needAck;
		pInOutBuff->sendingWaitTime = pParam->sendingWaitTime;
		pInOutBuff->ackTimeoutMs =
		pInOutBuff->nbOfRetry;
		//	timeoutCallback(netWork_inOutBuffer_t* this)
		
		pInOutBuff->isWaitAck = 0;
		pInOutBuff->seqWaitAck = 0;
		pInOutBuff->waitTimeCount = pParam->sendingWaitTime;
		pInOutBuff->ackWaitTimeCount;
		pInOutBuff->retryCount;
		
		
		sal_print(PRINT_WARNING,"id :%d needAck :%d sendingWaitTime :%d waitTimeCount :%d pBuffer :%d  \n",pInOutBuff->id = pParam->id, pInOutBuff->needAck,
																											pInOutBuff->sendingWaitTime, pInOutBuff->pBuffer ); //!! sup
		
		if(pInOutBuff->pBuffer == NULL)
		{
			free(pInOutBuff);
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

			free(pInOutBuff->pBuffer);
		
			free(pInOutBuff);
		}
		*ppInOutBuff = NULL;
	}	

}

void inOutBufferAckReceived(netWork_inOutBuffer_t* pInOutBuff, int seqNum)
{
	// !!! mutex ?
	
	if(pInOutBuff->isWaitAck && pInOutBuff->seqWaitAck == seqNum)
	{
		pInOutBuff->isWaitAck = 0;
		ringBuffPopFront( pInOutBuff->pBuffer, NULL );
	}
	// !!! mutex ?
}

netWork_inOutBuffer_t* inOutBufferWithId(	netWork_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id)
{
	netWork_inOutBuffer_t** it = pptabInOutBuff ;
	netWork_inOutBuffer_t** itEnd = pptabInOutBuff + (tabSize * sizeof(netWork_inOutBuffer_t*));
	netWork_inOutBuffer_t* pInOutBuffSearched = NULL;
	
	int find = 0;
	
	for(it = pptabInOutBuff ; ( it != itEnd ) && !find ; ++it)
	{
		if( (*it)->id == id)
		{
			pInOutBuffSearched = *it;
			find = 1;
		}
	}
	
	return pInOutBuffSearched;
}
