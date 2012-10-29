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
			sal_print(PRINT_WARNING,"deleteRingBuffer \n");//!! sup

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
	
	sal_mutex_lock(&(pRingBuff->mutex));
	
	//sal_print(PRINT_WARNING,"sal_mutex_lock ,");//!! sup
	
	//sal_print(PRINT_WARNING,"ringBuffGetFreeCellNb(pRingBuff) :%d ",ringBuffGetFreeCellNb(pRingBuff) );//!! sup
	
	if( pRingBuff->overwriting || ringBuffGetFreeCellNb(pRingBuff))
	{	
		buffPointor = pRingBuff->dataBuffer + 
					( pRingBuff->buffIndexInput % (pRingBuff->buffSize * pRingBuff->buffCellSize) );
		
		memcpy(buffPointor, pNewData, pRingBuff->buffCellSize);
		
		pRingBuff->buffIndexInput += pRingBuff->buffCellSize;
		//++pRingBuff->buffIndexInput;
		
		if( !ringBuffGetFreeCellNb(pRingBuff) && pRingBuff->overwriting )
		{
			sal_print(PRINT_WARNING," overwriting  " );//!! sup
			(pRingBuff->buffIndexOutput) += pRingBuff->buffCellSize;
		}
		
		error = 0; //!!
	}
	
	sal_mutex_unlock(&(pRingBuff->mutex));
	
	//sal_print(PRINT_WARNING,"\n");//!! sup

	return error;
}

int ringBuffPopFront(netWork_ringBuffer_t* pRingBuff, void* pPopData)
{
	void* buffPointor = NULL;
	int error = 0;
	
	sal_print(PRINT_WARNING,"ringBuffPopFront  \n"); //!! sup
	sal_mutex_lock(&(pRingBuff->mutex));
	
	if( !ringBuffIsEmpty(pRingBuff) )
	{
		if(pPopData != NULL)
		{
			buffPointor = 	pRingBuff->dataBuffer + 
					(pRingBuff->buffIndexOutput % (pRingBuff->buffSize * pRingBuff->buffCellSize) );
					
			memcpy(pPopData, buffPointor, pRingBuff->buffCellSize);
		}
		
		(pRingBuff->buffIndexOutput) += pRingBuff->buffCellSize;
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
	//sal_print(PRINT_WARNING," ringBuffGetFreeCellNb \n");//!! sup
	return pRingBuff->buffSize - ( 
			(pRingBuff->buffIndexInput - pRingBuff->buffIndexOutput) / pRingBuff->buffCellSize );
}

int ringBuffIsEmpty(const netWork_ringBuffer_t* pRingBuff)
{
	return pRingBuff->buffIndexInput == pRingBuff->buffIndexOutput;
}

/*
int ringBuffIsFull(const netWork_ringBuffer_t* pRingBuff)
{
	return ( ringBuffGetFreeCellNb(pRingBuff) < pRingBuff->buffSize );
}
*/

int ringBuffFront(const netWork_ringBuffer_t* pRingBuff, void* pFrontData)
{
	sal_print(PRINT_WARNING," ringBuffFront \n"); //!! sup
	
	int error = 1;
	void* buffPointor = pRingBuff->dataBuffer + 
					(pRingBuff->buffIndexOutput % ( pRingBuff->buffSize * pRingBuff->buffCellSize) );
	
	if(! ringBuffIsEmpty(pRingBuff) )
	{
		sal_mutex_lock(&(pRingBuff->mutex));
	
		memcpy(pFrontData, buffPointor, pRingBuff->buffCellSize);
	
		sal_mutex_unlock(&(pRingBuff->mutex));
		
		error = 0;
	}
	
	return error;
}

void ringBuffClean(netWork_ringBuffer_t* pRingBuff)
{
	sal_print(PRINT_WARNING," ringBuffClean \n"); //!! sup
	pRingBuff->buffIndexInput = pRingBuff->buffIndexOutput;
}


void ringPrint(netWork_ringBuffer_t* pRingBuff)
{
	void* it = pRingBuff->dataBuffer + (pRingBuff->buffIndexOutput * pRingBuff->buffCellSize);
	void* itEnd = pRingBuff->dataBuffer + (pRingBuff->buffIndexInput * pRingBuff->buffCellSize);
	
	int  ii = 0;
	
	sal_print(PRINT_WARNING," pointer dataBuffer :%d \n",pRingBuff->dataBuffer);
	sal_print(PRINT_WARNING," buffSize :%d \n",pRingBuff->buffSize);
	sal_print(PRINT_WARNING," buffCellSize :%d \n",pRingBuff->buffCellSize);
	sal_print(PRINT_WARNING," buffIndexOutput :%d \n",pRingBuff->buffIndexOutput);
	sal_print(PRINT_WARNING," buffIndexInput :%d \n",pRingBuff->buffIndexInput);
	sal_print(PRINT_WARNING," overwriting :%d \n",pRingBuff->overwriting);
	sal_print(PRINT_WARNING," data : \n",pRingBuff->overwriting);
	
	for( ; it < itEnd ; ++it)
	{
		sal_print(PRINT_WARNING,"	- ");
		
		for( ii = 0 ; ii < pRingBuff->buffCellSize ; ++ii)
		{
			sal_print(PRINT_WARNING,"%x",*((char*)it));
			++it;
		}
		
		sal_print(PRINT_WARNING,"\n");
	}
	
}
