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
static inline void ARNETWORK_RingBuffer_NormalizeIndexes(ARNETWORK_RingBuffer_t *ringBuffer)
{
    size_t buffer_size = ringBuffer->cellSize * ringBuffer->numberOfCell;
    if (ringBuffer->indexInput >= buffer_size && ringBuffer->indexOutput >= buffer_size)
    {
        ringBuffer->indexInput %= buffer_size;
        ringBuffer->indexOutput %= buffer_size;
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
    ARNETWORK_RingBuffer_t* ringBuffer =  malloc( sizeof(ARNETWORK_RingBuffer_t) );

    if(ringBuffer)
    {
        ringBuffer->numberOfCell = numberOfCell;
        ringBuffer->cellSize = cellSize;
        ringBuffer->indexInput = 0;
        ringBuffer->indexOutput = 0;
        ringBuffer->isOverwriting = isOverwriting;
        ARSAL_Mutex_Init( &(ringBuffer->mutex) );
        ringBuffer->dataBuffer = malloc( cellSize * numberOfCell );

        if( ringBuffer->dataBuffer == NULL)
        {
            ARNETWORK_RingBuffer_Delete(&ringBuffer);
        }
    }

    return ringBuffer;
}

void ARNETWORK_RingBuffer_Delete(ARNETWORK_RingBuffer_t **ringBuffer)
{
    /** -- Delete the ring buffer -- */

    if(ringBuffer != NULL)
    {
        if((*ringBuffer) != NULL)
        {
            ARSAL_Mutex_Destroy(&((*ringBuffer)->mutex));
            free((*ringBuffer)->dataBuffer);
            (*ringBuffer)->dataBuffer = NULL;

            free(*ringBuffer);
            (*ringBuffer) = NULL;
        }
    }
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PushBack(ARNETWORK_RingBuffer_t *ringBuffer, const uint8_t *newData) //inline ?
{
    /** -- Add the new data at the back of the ring buffer -- */

    return ARNETWORK_RingBuffer_PushBackWithSize(ringBuffer, newData, ringBuffer->cellSize, NULL);
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PushBackWithSize(ARNETWORK_RingBuffer_t *ringBuffer, const uint8_t *newData, int dataSize, uint8_t **dataCopy)
{
    /** -- Add the new data at the back of the ring buffer with specification of the data size -- */

    /** local declarations */
    int error = ARNETWORK_OK;
    uint8_t* buffer = NULL;

    ARSAL_Mutex_Lock(&(ringBuffer->mutex));

    /** check if the has enough free cell or the buffer is overwriting */
    if( (ARNETWORK_RingBuffer_GetFreeCellNumber(ringBuffer)) || (ringBuffer->isOverwriting) )
    {
        if( !ARNETWORK_RingBuffer_GetFreeCellNumber(ringBuffer) )
        {
            (ringBuffer->indexOutput) += ringBuffer->cellSize;
        }

        buffer = ringBuffer->dataBuffer + ( ringBuffer->indexInput % (ringBuffer->numberOfCell * ringBuffer->cellSize) );

        memcpy(buffer, newData, dataSize);

        /** return the pointer on the data copy in the ring buffer */
        if(dataCopy != NULL)
        {
            *dataCopy = buffer;
        }

        ringBuffer->indexInput += ringBuffer->cellSize;
        ARNETWORK_RingBuffer_NormalizeIndexes(ringBuffer);
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_SIZE;
    }

    ARSAL_Mutex_Unlock(&(ringBuffer->mutex));

    return error;
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PopFront(ARNETWORK_RingBuffer_t *ringBuffer, uint8_t *dataPop) //see inline
{
    /** -- Pop the oldest data -- */

    return ARNETWORK_RingBuffer_PopFrontWithSize(ringBuffer, dataPop, ringBuffer->cellSize);
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_PopFrontWithSize(ARNETWORK_RingBuffer_t *ringBuffer, uint8_t *dataPop, int dataSize)
{
    /** -- Pop the oldest data -- */

    /** local declarations */
    uint8_t *buffer = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;

    ARSAL_Mutex_Lock(&(ringBuffer->mutex));

    if( !ARNETWORK_RingBuffer_IsEmpty(ringBuffer) )
    {
        if(dataPop != NULL)
        {
            /** get the address of the front data */
            buffer = ringBuffer->dataBuffer + (ringBuffer->indexOutput % (ringBuffer->numberOfCell * ringBuffer->cellSize));
            memcpy(dataPop, buffer, dataSize);
        }
        (ringBuffer->indexOutput) += ringBuffer->cellSize;
        ARNETWORK_RingBuffer_NormalizeIndexes(ringBuffer);
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_EMPTY;
    }

    ARSAL_Mutex_Unlock(&(ringBuffer->mutex));

    return error;
}

eARNETWORK_ERROR ARNETWORK_RingBuffer_Front(ARNETWORK_RingBuffer_t *ringBuffer, uint8_t *frontData)
{
    /** -- Return a pointer on the front data -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint8_t *buffer = NULL;

    ARSAL_Mutex_Lock(&(ringBuffer->mutex));

    /** get the address of the front data */
    buffer = ringBuffer->dataBuffer + (ringBuffer->indexOutput % (ringBuffer->numberOfCell * ringBuffer->cellSize));

    if( !ARNETWORK_RingBuffer_IsEmpty(ringBuffer) )
    {
        memcpy(frontData, buffer, ringBuffer->cellSize);
    }
    else
    {
        error = ARNETWORK_ERROR_BUFFER_EMPTY;
    }

    ARSAL_Mutex_Unlock(&(ringBuffer->mutex));

    return error;
}

void ARNETWORK_RingBuffer_Print(ARNETWORK_RingBuffer_t *ringBuffer)
{
    /** -- Print the state of the ring buffer -- */

    ARSAL_Mutex_Lock(&(ringBuffer->mutex));

    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," pointer dataBuffer :%d \n",ringBuffer->dataBuffer);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," numberOfCell :%d \n",ringBuffer->numberOfCell);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," cellSize :%d \n",ringBuffer->cellSize);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," indexOutput :%d \n",ringBuffer->indexOutput);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," indexInput :%d \n",ringBuffer->indexInput);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," overwriting :%d \n",ringBuffer->isOverwriting);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG," data : \n");

    ARSAL_Mutex_Unlock(&(ringBuffer->mutex));

    ARNETWORK_RingBuffer_DataPrint(ringBuffer);
}

void ARNETWORK_RingBuffer_DataPrint(ARNETWORK_RingBuffer_t *ringBuffer)
{
    /** -- Print the contents of the ring buffer -- */

    /** local declarations */
    uint8_t *byteIterator = NULL;
    int  cellIndex = 0;
    int  byteIndex = 0;

    ARSAL_Mutex_Lock(&(ringBuffer->mutex));

    /** for all cell of the ringBuffer */
    for (cellIndex = ringBuffer->indexOutput ; cellIndex < ringBuffer->indexInput ; cellIndex += ringBuffer->cellSize )
    {
        byteIterator = ringBuffer->dataBuffer + (cellIndex % (ringBuffer->numberOfCell * ringBuffer->cellSize) );

        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG,"    - 0x: ");

        /** for all byte of the cell */
        for(byteIndex = 0 ; byteIndex < ringBuffer->cellSize ; ++byteIndex)
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG,"%2x | ",*((uint8_t*)byteIterator));
            ++byteIterator;
        }
        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RINGBUFFER_TAG,"\n");
    }

    ARSAL_Mutex_Unlock(&(ringBuffer->mutex));
}
