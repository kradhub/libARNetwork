/**
 *	@file circularBuffer.c
 *  @brief circular buffer for the commands to send
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libNetWork/ringBuffer.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_ringBuffer_t* newRingBuffer(	unsigned int buffSize, unsigned int buffCellSize)
{
	return newRingBufferWithOverwriting(	buffSize, buffCellSize, 0 );
}


netWork_ringBuffer_t* newRingBufferWithOverwriting(	unsigned int buffSize, 
														unsigned int buffCellSize, 
														int overwriting )
{
	sal_print(PRINT_WARNING,"newRingBuffer \n");//!! sup
	
	netWork_ringBuffer_t* pRingBuff =  malloc( sizeof(netWork_ringBuffer_t));
	
	if(pRingBuff)
	{
		pRingBuff->buffSize = buffSize;
		pRingBuff->buffCellSize = buffCellSize;
		//pRingBuff->buffFreeCellNb = buffCellSize;
		pRingBuff->buffIndexInput = 0;
		pRingBuff->buffIndexOutput = 0;
		pRingBuff->overwriting = overwriting;
		sal_mutex_init( &(pRingBuff->mutex) );
		pRingBuff->dataBuffer = malloc( buffCellSize * buffSize );
		
		if( pRingBuff->dataBuffer == NULL)
		{
			free(pRingBuff);
			pRingBuff = NULL;
		}
	}
	
	return pRingBuff;
}

void deleteRingBuffer(netWork_ringBuffer_t** ppRingBuff)
{
	netWork_ringBuffer_t* pRingBuff = NULL;
	
	if(ppRingBuff)
	{
		pRingBuff = *ppRingBuff;
		
		if(pRingBuff)
		{
			sal_print(PRINT_WARNING,"deleteBuffCmdAck \n");//!! sup

			sal_mutex_destroy(&(pRingBuff->mutex));
			free(pRingBuff->dataBuffer);
		
			free(pRingBuff);
		}
		*ppRingBuff = NULL;
	}
}

int ringBuffPushBack(netWork_ringBuffer_t* pRingBuff, const void* pNewData)
{
	int error = 1; //!!
	void* buffPointor = NULL;
	
	sal_print(PRINT_WARNING,"ringBuffPushBack ");//!! sup
	
	sal_mutex_lock(&(pRingBuff->mutex));
	
	if( ringBuffGetFreeCellNb(pRingBuff) /*pRingBuff->buffFreeCellNb*/)
	{
		buffPointor = pRingBuff->dataBuffer + (pRingBuff->buffIndexInput % pRingBuff->buffSize);
		(pRingBuff->buffIndexInput) += pRingBuff->buffCellSize;
		//--(pRingBuff->buffFreeCellNb);
		
		memcpy(buffPointor, pNewData, pRingBuff->buffCellSize);
		
		sal_print(PRINT_WARNING," ok " );//!! sup
		
		error = 0; //!!
	}
	
	sal_mutex_unlock(&(pRingBuff->mutex));
	
	sal_print(PRINT_WARNING,"\n");//!! sup

	return error;
}

int ringBuffPopFront(netWork_ringBuffer_t* pRingBuff, void* pPopData)
{
	void* buffPointor = NULL;
	int error = 0;
	
	sal_mutex_lock(&(pRingBuff->mutex));
	
	if( pRingBuff->overwriting || ringBuffGetFreeCellNb(pRingBuff) )
	{
		if(pPopData)
		{
			buffPointor = 	pRingBuff->dataBuffer + 
							(pRingBuff->buffIndexOutput % pRingBuff->buffSize);
		}
		
		(pRingBuff->buffIndexOutput) -= pRingBuff->buffCellSize;
		
		sal_print(PRINT_WARNING,"send data  \n"); //!! sup
	}
	else
	{
		error = 1;
	}
	
	sal_mutex_unlock(&(pRingBuff->mutex));
	
	return error;
}


unsigned int ringBuffGetFreeCellNb(const netWork_ringBuffer_t* pRingBuff)
{
	return pRingBuff->buffIndexOutput - pRingBuff->buffIndexInput ;
}

int ringBuffIsEmpty(const netWork_ringBuffer_t* pRingBuff)
{
	return ( pRingBuff->buffSize == ringBuffGetFreeCellNb(pRingBuff) );
}

/*
int ringBuffIsFull(const netWork_ringBuffer_t* pRingBuff)
{
	return ( ringBuffGetFreeCellNb(pRingBuff) < pRingBuff->buffSize );
}
*/

int ringBuffFront(netWork_ringBuffer_t* pRingBuff, void* pFrontData)
{
	void* buffPointor = pRingBuff->dataBuffer + (pRingBuff->buffIndexOutput % pRingBuff->buffSize);
	
	sal_mutex_lock(&(pRingBuff->mutex));
	
	memcpy(pFrontData, buffPointor, pRingBuff->buffCellSize);
	
	sal_mutex_unlock(&(pRingBuff->mutex));
}

void ringBuffClean(netWork_ringBuffer_t* pRingBuff)
{
	pRingBuff->buffIndexInput = pRingBuff->buffIndexOutput;
}
