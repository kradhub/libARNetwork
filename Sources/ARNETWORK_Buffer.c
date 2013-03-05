/**
 *  @file ARNETWORK_Buffer.c
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

#include "ARNETWORK_Buffer.h"

/*****************************************
 * 
 *             define :
 *
******************************************/

#define ARNETWORK_BUFFER_TAG "ARNETWORK_Buffer"

/*****************************************
 * 
 *             implementation :
 *
******************************************/

ARNETWORK_Buffer_t* ARNETWORK_Buffer_New(unsigned int numberOfCell, unsigned int cellSize)
{
    /** -- Create a new buffer -- */
    
    /** local declarations */
    ARNETWORK_Buffer_t *bufferPtr = malloc(sizeof(ARNETWORK_Buffer_t));
    
    if(bufferPtr)
    {
        bufferPtr->numberOfCell = numberOfCell;
        bufferPtr->cellSize = cellSize;
        bufferPtr->startPtr = malloc( cellSize * numberOfCell );
        bufferPtr->endPtr = bufferPtr->startPtr + ( cellSize * numberOfCell );
        bufferPtr->frontPtr = bufferPtr->startPtr;
        
        if(bufferPtr->startPtr == NULL)
        {
            ARNETWORK_Buffer_Delete(&bufferPtr);
        }
    }
    
    return bufferPtr;
}

void ARNETWORK_Buffer_Delete(ARNETWORK_Buffer_t **bufferPtrAddr)
{
    /** -- Delete the buffer -- */
    
    /** local declarations */
    ARNETWORK_Buffer_t *bufferPtr = NULL;
    
    if(bufferPtrAddr != NULL)
    {
        bufferPtr = *bufferPtrAddr;
        
        if(bufferPtr != NULL)
        {
            free(bufferPtr->startPtr);
            bufferPtr->startPtr = NULL;
            
            free(bufferPtr);
            bufferPtr = NULL;
        }
        *bufferPtrAddr = NULL;
    }
}

void ARNETWORK_Buffer_Print(ARNETWORK_Buffer_t *bufferPtr)
{
    /** -- Print the state of the buffer -- */
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG, " pointer dataBuffer :%d \n",bufferPtr->startPtr);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG, " numberOfCell :%d \n",bufferPtr->numberOfCell);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG, " cellSize :%d \n",bufferPtr->cellSize);
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG, " data : \n");
    
    ARNETWORK_Buffer_DataPrint(bufferPtr);
}

void ARNETWORK_Buffer_DataPrint(ARNETWORK_Buffer_t *bufferPtr)
{
    /** -- Print the contents of the buffer -- */
    
    /** local declarations */
    uint8_t *it = bufferPtr->startPtr;
    uint8_t *itEnd = bufferPtr->frontPtr;
    int  ii = 0;
    
    while( it < itEnd )
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG,"    - 0x: ");
        for(ii = 0; ii < bufferPtr->cellSize; ++ii)
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG,"%2x | ",*((uint8_t*)it));
            ++it;
        }
        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_BUFFER_TAG,"\n");
    }
}
