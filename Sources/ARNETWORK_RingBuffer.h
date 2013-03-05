/**
 *  @file ARNETWORK_RingBuffer.h
 *  @brief Ring buffer, multithread safe with overwriting possibility.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_RINGBUFFER_PRIVATE_H_
#define _ARNETWORK_RINGBUFFER_PRIVATE_H_

#include <libARSAL/ARSAL_Mutex.h>
#include <inttypes.h>

/**
 *  @brief Basic ring buffer, multithread safe
 *  @warning before to be used the ring buffer must be created through ARNETWORK_RingBuffer_New() or ARNETWORK_RingBuffer_NewWithOverwriting()
 *  @post after its using the ring buffer must be deleted through ARNETWORK_RingBuffer_Delete()
**/
typedef struct  
{
    uint8_t *dataBuffer; /**< Pointer on the data buffer*/
    unsigned int numberOfCell; /**< Maximum number of data stored*/
    unsigned int cellSize; /**< Size of one data in byte*/
    unsigned int isOverwriting; /**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    
    unsigned int indexInput; /**< Index of the data input*/
    unsigned int indexOutput; /**< Index of the data output*/
    
    ARSAL_Mutex_t mutex; /**< Mutex to take before to use the ringBuffer*/

}ARNETWORK_RingBuffer_t;

/**
 *  @brief Create a new ring buffer
 *  @warning This function allocate memory
 *  @post ARNETWORK_RingBuffer_Delete() must be called to delete the ring buffer and free the memory allocated
 *  @param[in] numberOfCell Maximum number of data cell of the ring buffer
 *  @param[in] cellSize size of one data cell of the ring buffer
 *  @return Pointer on the new ring buffer
 *  @see ARNETWORK_RingBuffer_NewWithOverwriting()
 *  @see ARNETWORK_RingBuffer_Delete()
**/
ARNETWORK_RingBuffer_t* ARNETWORK_RingBuffer_New(unsigned int numberOfCell, unsigned int cellSize); 

/**
 *  @brief Create a new ring buffer.
 *  @warning This function allocate memory
 *  @post ARNETWORK_RingBuffer_Delete() must be called to delete the ring buffer and free the memory allocated
 *  @param[in] numberOfCell Maximum number of data cell of the ring buffer
 *  @param[in] cellSize size of one data cell of the ring buffer
 *  @param[in] isOverwriting set to 1 allow the overwriting if the buffer is full otherwise set 0
 *  @return Pointer on the new ring buffer
 *  @see ARNETWORK_RingBuffer_NewWithOverwriting()
 *  @see ARNETWORK_RingBuffer_Delete()
**/
ARNETWORK_RingBuffer_t* ARNETWORK_RingBuffer_NewWithOverwriting(unsigned int numberOfCell, unsigned int cellSize, int isOverwriting); 

/**
 *  @brief Delete the ring buffer
 *  @warning This function free memory
 *  @param ringBufferPtrAddr address of the pointer on the ring buffer to delete
 *  @see ARNETWORK_RingBuffer_New()
 *  @see ARNETWORK_RingBuffer_NewWithOverwriting()
**/
void ARNETWORK_RingBuffer_Delete(ARNETWORK_RingBuffer_t **ringBufferPtrAddr);

/**
 *  @brief Add the new data at the back of the ring buffer
 *  @warning newDataPtr must be different of NULL
 *  @param ringBufferPtr pointer on the ring buffer
 *  @param[in] newDataPtr pointer on the data to add
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_RingBuffer_PushBack(ARNETWORK_RingBuffer_t *ringBufferPtr, const uint8_t *newDataPtr);

/**
 *  @brief Add the new data at the back of the ring buffer
 *  @warning newDataPtr must be different of NULL
 *  @warning data size must not be more than ring buffer's cell size
 *  @note if data size is less than ring buffer's cell size, the bytes at the cell end are not set. 
 *  @param ringBufferPtr pointer on the ring buffer
 *  @param[in] newDataPtr pointer on the data to add
 *  @param[in] dataSize size in byte of the data to copy from newDataPtr
 *  @param[out] dataCopyPtrAddr address to return the pointer on the data copy in the ring buffer ; can be equal to NULL 
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_RingBuffer_PushBackWithSize(ARNETWORK_RingBuffer_t *ringBufferPtr, const uint8_t *newDataPtr, int dataSize, uint8_t **dataCopyPtrAddr);

/**
 *  @brief Pop the oldest data
 *  @param ringBufferPtr pointer on the ring buffer
 *  @param[out] dataPopPtr pointer on the data popped
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_RingBuffer_PopFront(ARNETWORK_RingBuffer_t *ringBufferPtr, uint8_t *dataPopPtr);

/**
 *  @brief Pop the oldest data
 *  @warning dataSize must be less or equal of the ring buffer's cell size.
 *  @note if data size is less than ring buffer's cell size, the bytes at the cell end are lost.
 *  @param ringBufferPtr pointer on the ring buffer
 *  @param[out] dataPopPtr pointer on the data popped
 *  @param[in] dataSize size to copy from the front data to the dataPopPtr
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_RingBuffer_PopFrontWithSize(ARNETWORK_RingBuffer_t *ringBufferPtr, uint8_t *dataPopPtr, int dataSize);

/**
 *  @brief Return the number of free cell of the ring buffer
 *  @param ringBufferPtr pointer on the ring buffer
 *  @return number of free cell of the ring buffer 
**/
static inline int ARNETWORK_RingBuffer_GetFreeCellNumber(const ARNETWORK_RingBuffer_t *ringBufferPtr)
{
    return ringBufferPtr->numberOfCell - ( (ringBufferPtr->indexInput - ringBufferPtr->indexOutput) / ringBufferPtr->cellSize );
}

/**
 *  @brief Check if the ring buffer is empty
 *  @param ringBufferPtr pointer on the ring buffer
 *  @return equal to 1 if the ring buffer is empty else 0
**/
static inline int ARNETWORK_RingBuffer_IsEmpty(const ARNETWORK_RingBuffer_t *ringBufferPtr)
{
    return (ringBufferPtr->indexInput == ringBufferPtr->indexOutput) ? 1 : 0;
}

/**
 *  @brief Return a pointer on the front data
 *  @param ringBufferPtr pointer on the ring buffer
 *  @param[out] frontDataPtr pointer on the front data
 *  @return error eARNETWORK_ERROR
**/
eARNETWORK_ERROR ARNETWORK_RingBuffer_Front(ARNETWORK_RingBuffer_t *ringBufferPtr, uint8_t *frontDataPtr);

/**
 *  @brief Clean the ring buffer
 *  @param ringBufferPtr pointer on the ring buffer
**/
static inline void ARNETWORK_RingBuffer_Clean(ARNETWORK_RingBuffer_t *ringBufferPtr)
{
    ringBufferPtr->indexInput = ringBufferPtr->indexOutput;
}

/**
 *  @brief Print the state of the ring buffer
 *  @param ringBufferPtr pointer on the ring buffer
**/
void ARNETWORK_RingBuffer_Print(ARNETWORK_RingBuffer_t *ringBufferPtr);

/**
 *  @brief Print the contents of the ring buffer
 *  @param ringBufferPtr pointer on the ring buffer
**/
void ARNETWORK_RingBuffer_DataPrint(ARNETWORK_RingBuffer_t *ringBufferPtr);

#endif /** _ARNETWORK_RINGBUFFER_PRIVATE_H_ */

