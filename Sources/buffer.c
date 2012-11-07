/**
 *	@file singleBuffer.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libNetWork/buffer.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

netWork_buffer_t* newBuffer(unsigned int buffSize, unsigned int buffCellSize)
{
	netWork_buffer_t* pBuffer = malloc( sizeof(netWork_buffer_t));
	
	if(pBuffer)
	{
		sal_mutex_init( &(pBuffer->mutex) );
		pBuffer->buffSize = buffSize;
		pBuffer->buffCellSize = buffCellSize;
		pBuffer->pStart = malloc( buffCellSize * buffSize );
		pBuffer->pEnd = pBuffer->pStart + ( buffCellSize * buffSize );
		pBuffer->pFront = pBuffer->pStart;
		
		if( pBuffer->pStart == NULL)
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
			sal_mutex_destroy(&(pBuffer->mutex));
			free(pBuffer->pStart);
			
			free(pBuffer);
		}
		*ppBuffer = NULL;
	}
}

unsigned int bufferGetFreeCellNb(const netWork_buffer_t* pBuffer)
{
	return (pBuffer->pEnd - pBuffer->pFront) / pBuffer->buffCellSize;
}

int bufferIsEmpty(netWork_buffer_t* pBuffer)
{
	return pBuffer->pStart == pBuffer->pFront;
}

void bufferClean(netWork_buffer_t* pBuffer)
{
	pBuffer->pFront = pBuffer->pStart;
}

void bufferPrint(netWork_buffer_t* pBuffer)
{
	void* it = pBuffer->pStart;
	
	void* itEnd = pBuffer->pFront;
	
	int  ii = 0;
	
	sal_print(PRINT_WARNING," pointer dataBuffer :%d \n",pBuffer->pStart);
	sal_print(PRINT_WARNING," buffSize :%d \n",pBuffer->buffSize);
	sal_print(PRINT_WARNING," buffCellSize :%d \n",pBuffer->buffCellSize);
	
	sal_print(PRINT_WARNING," data : \n");

	bufferDataPrint(pBuffer);
}

void bufferDataPrint(netWork_buffer_t* pBuffer)
{
	void* it = pBuffer->pStart;
	
	void* itEnd = pBuffer->pFront;
	
	int  ii = 0;
	
	while( it < itEnd )
	{
		sal_print(PRINT_WARNING,"	- 0x: ");
		for(ii = 0 ; ii < pBuffer->buffCellSize ; ++ii)
		{
			sal_print(PRINT_WARNING,"%2x | ",*((uint8_t*)it));
			++it;
		}
		sal_print(PRINT_WARNING,"\n");
	}
}
