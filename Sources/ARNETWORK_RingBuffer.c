/**
 * @file ARNETWORK_RingBuffer.c
 * @brief Ring buffer, multithread safe with overwriting possibility.
 * @date 28/09/2012
 * @author maxime.maitre@parrot.com
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

#include <libARNetwork/ARNETWORK_Error.h>

#include "ARNETWORK_RingBuffer.h"

/*****************************************
 *
 *             define :
 *
 ******************************************/

#define ARNETWORK_RINGBUFFER_TAG "ARNETWORK_RingBuffer"

/*****************************************
 *
 *             internal functions :
 *
 ******************************************/

/* Normalize indices if they are both larger than the entire buffer size
 * so that they do not overflow. */
static inline void normalize_indices(ARNETWORK_RingBuffer_t *ringBufferPtr)
{
    size_t buffer_size = ringBufferPtr->cellSize * ringBufferPtr->numberOfCell;
    if (ringBufferPtr->indexInput >= buffer_size && ringBufferPtr->indexOutput >= buffer_size)
    {
        ringBufferPtr->indexInput %= buffer_size;
        ringBufferPtr->indexOutput %= buffer_size;
    }
}

/*****************************************
 *
 *             implementation :
 *
 ******************************************/

ARNETWORK_RingBuffer_t* ARNETWORK_RingBuffer_New(unsigned int numberOfCell, unsigned int cellSize)
{
    /** -- Create a new ring buffer not overwriarray -- */
    return ARNETWORK_RingBuffer_NewWithOverwriting( numberOfCell, cellSize, 0 );
}

ARNETWORK_RingBuffer_t* ARNETWORK_RingBuffer_NewWithOverwriting(unsigned int numberOfCell, unsigned int cellSize, int isOverwriting)
{
    /** -- Create a new ring buffer -- */

    /** local declarations */
    ARNETWORK_RingBuffer_t* ringBufferPtr =  malloc( sizeof(ARNETWORK_RingBuffer_t) );

    if(ringBufferPtr)
    {
        ringBufferPtr->numberOfCell = numberOfCell;
        ringBufferPtr->cellSize = cellSize;
        ringBufferPtr->indexInput = 0;
        ringBufferPtr->indexOutput = 0;
        ringBufferPtr->isOverwriting = isOverwriting;
        ARSAL_Mutex_Init( &(ringBufferPtr->mutex) );
        ringBufferPtr->dataBuffer = malloc( cellSize * numberOfCell );

        if( ringBufferPtr->dataBuffer == NULL)
        {
            ARNETWORK_RingBuffer_Delete(&ringBufferPtr);
        }
    }

    return ringBufferPtr;
}

void ARNETWORK_RingBuffer_Delete(ARNETWORK_RingBuffer_t **ringBufferPtrAddr)
{
    /** -- Delete the ring buffer -- */

    /** local declarations */
    ARNETWORK_RingBuffer_t* ringBufferPtr = NULL;

    if(ringBufferPtrAddr != NULL)
    {
        ringBufferPtr = *ringBufferPtrAddr;

        if(ringBufferPtr != NULL)
        {
            ARSAL_Mutex_Destroy(&(ringBufferPtr->mutex));
            free(ringBufferPtr->dataBuffer);
            ringBufferPtr->dataBuffer = NULL;

            free(ringBufferPtr);
            ringBufferPtr = NULL;
        }
        *ringBufferPtrAddr = NULL;
    }
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PushBack(ARNETWORK_RingBuffer_t *ringBufferPtr, const uint8_t *newDataPtr) //inline ?
{
    /** -- Add the new data at the back of the ring buffer -- */

    return ARNETWORK_RingBuffer_PushBackWithSize(ringBufferPtr, newDataPtr, ringBufferPtr->cellSize, NULL);
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PushBackWithSize(ARNETWORK_RingBuffer_t *ringBufferPtr, const uint8_t *newDataPtr, int dataSize, uint8_t **dataCopyPtrAddr)
{
    /** -- Add the new data at the back of the ring buffer with specification of the data size -- */

    /** local declarations */
    int error = ARNETWORK_OK;
    uint8_t* bufferPtr = NULL;

    ARSAL_Mutex_Lock(&(ringBufferPtr->mutex));

    /** check if the has enough free cell or the buffer is overwriting */
    if( (ARNETWORK_RingBuffer_GetFreeCellNumber(ringBufferPtr)) || (ringBufferPtr->isOverwriting) )
    {
        if( !ARNETWORK_RingBuffer_GetFreeCellNumber(ringBufferPtr) )
        {
            (ringBufferPtr->indexOutput) += ringBufferPtr->cellSize;
        }

        bufferPtr = ringBufferPtr->dataBuffer + ( ringBufferPtr->indexInput % (ringBufferPtr->numberOfCell * ringBufferPtr->cellSize) );

        memcpy(bufferPtr, newDataPtr, dataSize);

        /** return the pointer on the data copy in the ring buffer */
        if(dataCopyPtrAddr != NULL)
        {
            *dataCopyPtrAddr = bufferPtr;
        }

        ringBufferPtr->indexInput += ringBufferPtr->cellSize;
        normalize_indices(ringBufferPtr);
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_SIZE;
    }

    ARSAL_Mutex_Unlock(&(ringBufferPtr->mutex));

    return error;
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PopFront(ARNETWORK_RingBuffer_t *ringBufferPtr, uint8_t *dataPopPtr) //see inline
{
    /** -- Pop the oldest data -- */

    return ARNETWORK_RingBuffer_PopFrontWithSize(ringBufferPtr, dataPopPtr, ringBufferPtr->cellSize);
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PopFrontWithSize(ARNETWORK_RingBuffer_t *ringBufferPtr, uint8_t *dataPopPtr, int dataSize)
{
    /** -- Pop the oldest data -- */

    /** local declarations */
    uint8_t *bufferPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;

    ARSAL_Mutex_Lock(&(ringBufferPtr->mutex));

    if( !ARNETWORK_RingBuffer_IsEmpty(ringBufferPtr) )
    {
        if(dataPopPtr != NULL)
        {
            /** get the address of the front data */
            bufferPtr = ringBufferPtr->dataBuffer + (ringBufferPtr->indexOutput % (ringBufferPtr->numberOfCell * ringBufferPtr->cellSize));
            memcpy(dataPopPtr, bufferPtr, dataSize);
        }
        (ringBufferPtr->indexOutput) += ringBufferPtr->cellSize;
        normalize_indices(ringBufferPtr);
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_EMPTY;
    }

    ARSAL_Mutex_Unlock(&(ringBufferPtr->mutex));

    return error;
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_Front(ARNETWORK_RingBuffer_t *ringBufferPtr, uint8_t *frontDataPtr)
{
    /** -- Return a pointer on the front data -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint8_t *bufferPtr = NULL;

    ARSAL_Mutex_Lock(&(ringBufferPtr->mutex));

    /** get the address of the front data */
    bufferPtr = ringBufferPtr->dataBuffer + (ringBufferPtr->indexOutput % (ringBufferPtr->numberOfCell * ringBufferPtr->cellSize));

    if( !ARNETWORK_RingBuffer_IsEmpty(ringBufferPtr) )
    {
        memcpy(frontDataPtr, bufferPtr, ringBufferPtr->cellSize);
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_EMPTY;
    }

    ARSAL_Mutex_Unlock(&(ringBufferPtr->mutex));

    return error;
}

void ARNETWORK_RingBuffer_Print(ARNETWORK_RingBuffer_t *ringBufferPtr)
{
    /** -- Print the state of the ring buffer -- */

    ARSAL_Mutex_Lock(&(ringBufferPtr->mutex));

    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," pointer dataBuffer :%d \n",ringBufferPtr->dataBuffer);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," numberOfCell :%d \n",ringBufferPtr->numberOfCell);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," cellSize :%d \n",ringBufferPtr->cellSize);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," indexOutput :%d \n",ringBufferPtr->indexOutput);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," indexInput :%d \n",ringBufferPtr->indexInput);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," overwriting :%d \n",ringBufferPtr->isOverwriting);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," data : \n");

    ARSAL_Mutex_Unlock(&(ringBufferPtr->mutex));

    ARNETWORK_RingBuffer_DataPrint(ringBufferPtr);
}

void ARNETWORK_RingBuffer_DataPrint(ARNETWORK_RingBuffer_t *ringBufferPtr)
{
    /** -- Print the contents of the ring buffer -- */

    /** local declarations */
    uint8_t *it = NULL;
    int  index = 0;
    int  ii = 0;

    ARSAL_Mutex_Lock(&(ringBufferPtr->mutex));

    /** for all cell of the ringBuffer */
    for( index = ringBufferPtr->indexOutput ; index < ringBufferPtr->indexInput ; index += ringBufferPtr->cellSize )
    {
        it = ringBufferPtr->dataBuffer + (index % (ringBufferPtr->numberOfCell * ringBufferPtr->cellSize) );

        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG,"    - 0x: ");

        /** for all byte of the cell */
        for(ii = 0 ; ii < ringBufferPtr->cellSize ; ++ii)
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG,"%2x | ",*((uint8_t*)it));
            ++it;
        }
        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG,"\n");
    }

    ARSAL_Mutex_Unlock(&(ringBufferPtr->mutex));
}
