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

#include "buffer.h"

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

network_buffer_t* NETWORK_NewBuffer(unsigned int buffSize, unsigned int buffCellSize)
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
			NETWORK_DeleteBuffer(&pBuffer);
		}
    }
    
    return pBuffer;
}

void NETWORK_DeleteBuffer(network_buffer_t** ppBuffer)
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
            pBuffer->pStart = NULL;
			
			free(pBuffer);
            pBuffer = NULL;
		}
		*ppBuffer = NULL;
	}
}

void NETWORK_BufferPrint(network_buffer_t* pBuffer)
{	
	/** -- Print the state of the buffer -- */
	
	SAL_PRINT(PRINT_WARNING," pointer dataBuffer :%d \n",pBuffer->pStart);
	SAL_PRINT(PRINT_WARNING," buffSize :%d \n",pBuffer->buffSize);
	SAL_PRINT(PRINT_WARNING," buffCellSize :%d \n",pBuffer->buffCellSize);
	
	SAL_PRINT(PRINT_WARNING," data : \n");

	NETWORK_BufferDataPrint(pBuffer);
}

void NETWORK_BufferDataPrint(network_buffer_t* pBuffer)
{
	/** -- Print the contents of the buffer --*/
	
	/** local declarations */
	void* it = pBuffer->pStart;
	void* itEnd = pBuffer->pFront;
	int  ii = 0;
	
	while( it < itEnd )
	{
		SAL_PRINT(PRINT_WARNING,"	- 0x: ");
		for(ii = 0 ; ii < pBuffer->buffCellSize ; ++ii)
		{
			SAL_PRINT(PRINT_WARNING,"%2x | ",*((uint8_t*)it));
			++it;
		}
		SAL_PRINT(PRINT_WARNING,"\n");
	}
}
