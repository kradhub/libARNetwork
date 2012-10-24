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
	sal_print(PRINT_WARNING,"newNetWork \n"); //!! sup
	
	if(pInOutBuff)
	{
		pInOutBuff->id = pParam->id;
		pInOutBuff->needAck = pParam->needAck;
		pInOutBuff->sendingWaitTime = pParam->sendingWaitTime;
		pInOutBuff->isWaitAck = 0;
		pInOutBuff->seqWaitAck = 0;
		pInOutBuff->waitTimeCount = pParam->sendingWaitTime;
		pInOutBuff->pBuffer = newRingBuffer(pParam->buffSize, pParam->buffCellSize);
		
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
			sal_print(PRINT_WARNING,"deleteBuffCmdAck \n");//!! sup

			free(pInOutBuff->pBuffer);
		
			free(pInOutBuff);
		}
		*ppInOutBuff = NULL;
	}	

}

void inOutBufferTransmitAck(netWork_inOutBuffer_t* pInOutBuff, int seqNum)
{
	// !!! mutex ?
	
	if(pInOutBuff->isWaitAck && pInOutBuff->seqWaitAck == seqNum)
	{
		pInOutBuff->isWaitAck = 0;
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
