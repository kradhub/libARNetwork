/**
 *	@file singleBuffer.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libNetWork/singleBuffer.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

netWork_buffer_t* newBuffer(unsigned int buffSize, unsigned int buffCellSize)
{
	netWork_buffer_t* pBuffer= malloc( sizeof(netWork_buffer_t));
	sal_print(PRINT_WARNING,"newBuffer \n"); //!! sup
	
	if(pBuffer)
	{
		sal_mutex_init( &(pBuffer->mutex) );
		pBuffer->buffCellSize = buffCellSize;
		pBuffer->buffSize = buffSize;
		pBuffer->pBack = malloc( buffCellSize * buffSize );
		pBuffer->pEnd = pBuffer->pBack + ( buffCellSize * buffSize );
		pBuffer->pFront = pBuffer->pBack;
		
		if( pBuffer->pBack == NULL)
		{
			free(pBuffer);
			pBuffer = NULL;
		}
    }
    
    return pBuffer;
}

void deleteBuffer(netWork_buffer_t** ppBuffer)
{	
	netWork_buffer_t* pBuffer = NULL;
	
	if(ppBuffer)
	{
		pBuffer = *ppBuffer;
		
		if(pBuffer)
		{
			sal_print(PRINT_WARNING,"deleteBuffer \n");//!! sup

			sal_mutex_destroy(&(pBuffer->mutex));
			free(pBuffer->pBack);
			
			free(pBuffer);
		}
		*ppBuffer = NULL;
	}
}

int bufferPushFront(netWork_buffer_t* pBuffer, void* pData)
{
	int error = 1; //!!
	
	sal_print(PRINT_WARNING,"bufferPuchFront ");//!! sup
	
	sal_mutex_lock(&(pBuffer->mutex));
	
	if( !bufferIsEmpty(pBuffer) )
	{	
		memcpy(pBuffer->pFront, pData, pBuffer->buffCellSize);
		
		pBuffer->pFront += pBuffer->buffCellSize;
		
		sal_print(PRINT_WARNING," ok " );//!! sup
		
		error = 0; //!!
	}
	
	sal_mutex_unlock(&(pBuffer->mutex));
	
	sal_print(PRINT_WARNING,"\n");//!! sup

	return error;
}

unsigned int bufferGetFreeCellNb(const netWork_buffer_t* pBuffer)
{
	return (pBuffer->pEnd - pBuffer->pFront) / pBuffer->buffCellSize;
}

int bufferIsEmpty(netWork_buffer_t* pBuffer)
{
	return pBuffer->pBack == pBuffer->pFront;
}

void bufferClean(netWork_buffer_t* pBuffer)
{
	pBuffer->pFront = pBuffer->pBack;
}
