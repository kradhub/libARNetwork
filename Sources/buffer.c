/**
 *	@file buffer.c
 *  @brief basic buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libNetwork/buffer.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

network_buffer_t* newBuffer(unsigned int buffSize, unsigned int buffCellSize)
{
	/** -- Create a new buffer -- */
	
	/** local declarations */
	network_buffer_t* pBuffer = malloc( sizeof(network_buffer_t));
	
	if(pBuffer)
	{
		pBuffer->buffSize = buffSize;
		pBuffer->buffCellSize = buffCellSize;
		pBuffer->pStart = malloc( buffCellSize * buffSize );
		pBuffer->pEnd = pBuffer->pStart + ( buffCellSize * buffSize );
		pBuffer->pFront = pBuffer->pStart;
		
		if( pBuffer->pStart == NULL)
		{
			deleteBuffer(&pBuffer);
		}
    }
    
    return pBuffer;
}

void deleteBuffer(network_buffer_t** ppBuffer)
{	
	/** -- Delete the buffer -- */
	
	/** local declarations */
	network_buffer_t* pBuffer = NULL;
	
	if(ppBuffer)
	{
		pBuffer = *ppBuffer;
		
		if(pBuffer)
		{
			free(pBuffer->pStart);
			
			free(pBuffer);
		}
		*ppBuffer = NULL;
	}
}

void bufferPrint(network_buffer_t* pBuffer)
{	
	/** -- Print the state of the buffer -- */
	
	sal_print(PRINT_WARNING," pointer dataBuffer :%d \n",pBuffer->pStart);
	sal_print(PRINT_WARNING," buffSize :%d \n",pBuffer->buffSize);
	sal_print(PRINT_WARNING," buffCellSize :%d \n",pBuffer->buffCellSize);
	
	sal_print(PRINT_WARNING," data : \n");

	bufferDataPrint(pBuffer);
}

void bufferDataPrint(network_buffer_t* pBuffer)
{
	/** -- Print the contents of the buffer --*/
	
	/** local declarations */
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
