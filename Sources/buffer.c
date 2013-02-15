/**
 *  @file buffer.c
 *  @brief basic buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Mutex.h>

#include "buffer.h"

/*****************************************
 * 
 *             define :
 *
******************************************/
#define TAG "Buffer"

/*****************************************
 * 
 *             implementation :
 *
******************************************/

network_buffer_t* NETWORK_NewBuffer(unsigned int numberOfCell, unsigned int cellSize)
{
    /** -- Create a new buffer -- */
    
    /** local declarations */
    network_buffer_t* pBuffer = malloc( sizeof(network_buffer_t));
    
    if(pBuffer)
    {
        pBuffer->numberOfCell = numberOfCell;
        pBuffer->cellSize = cellSize;
        pBuffer->pStart = malloc( cellSize * numberOfCell );
        pBuffer->pEnd = pBuffer->pStart + ( cellSize * numberOfCell );
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
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG," pointer dataBuffer :%d \n",pBuffer->pStart);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG," numberOfCell :%d \n",pBuffer->numberOfCell);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG," cellSize :%d \n",pBuffer->cellSize);
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG," data : \n");

    NETWORK_BufferDataPrint(pBuffer);
}

void NETWORK_BufferDataPrint(network_buffer_t* pBuffer)
{
    /** -- Print the contents of the buffer --*/
    
    /** local declarations */
    uint8_t* it = pBuffer->pStart;
    uint8_t* itEnd = pBuffer->pFront;
    int  ii = 0;
    
    while( it < itEnd )
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG,"    - 0x: ");
        for(ii = 0 ; ii < pBuffer->cellSize ; ++ii)
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG,"%2x | ",*((uint8_t*)it));
            ++it;
        }
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG,"\n");
    }
}
