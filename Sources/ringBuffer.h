/**
 *  @file ringBuffer.h
 *  @brief Ring buffer, multithread safe with overwriting possibility.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_RINGBUFFER_H_
#define _NETWORK_RINGBUFFER_H_

#include <libSAL/mutex.h>

/**
 *  @brief Basic ring buffer, multithread safe
 *  @warning before to be used the ring buffer must be created through NETWORK_NewRingBuffer() or NETWORK_NewRingBufferWithOverwriting()
 *  @post after its using the ring buffer must be deleted through NETWORK_DeleteRingBuffer()
**/
typedef struct network_ringBuffer_t  
{
    void* dataBuffer; /**< Pointer on the data buffer*/
    unsigned int numberOfCell; /**< Maximum number of data stored*/
    unsigned int cellSize; /**< Size of one data in byte*/
    unsigned int isOverwriting; /**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    
    unsigned int indexInput; /**< Index of the data input*/
    unsigned int indexOutput; /**< Index of the data output*/
    
    sal_mutex_t mutex; /**< Mutex to take before to use the ringBuffer*/

}network_ringBuffer_t;

/**
 *  @brief Create a new ring buffer
 *  @warning This function allocate memory
 *  @post NETWORK_DeleteRingBuffer() must be called to delete the ring buffer and free the memory allocated
 *  @param[in] numberOfCell Maximum number of data cell of the ring buffer
 *  @param[in] cellSize size of one data cell of the ring buffer
 *  @return Pointer on the new ring buffer
 *  @see NETWORK_NewRingBufferWithOverwriting()
 *  @see NETWORK_DeleteRingBuffer()
**/
network_ringBuffer_t* NETWORK_NewRingBuffer( unsigned int numberOfCell, unsigned int cellSize ); 

/**
 *  @brief Create a new ring buffer.
 *  @warning This function allocate memory
 *  @post NETWORK_DeleteRingBuffer() must be called to delete the ring buffer and free the memory allocated
 *  @param[in] numberOfCell Maximum number of data cell of the ring buffer
 *  @param[in] cellSize size of one data cell of the ring buffer
 *  @param[in] isOverwriting set to 1 allow the overwriting if the buffer is full otherwise set 0
 *  @return Pointer on the new ring buffer
 *  @see NETWORK_NewRingBufferWithOverwriting()
 *  @see NETWORK_DeleteRingBuffer()
**/
network_ringBuffer_t* NETWORK_NewRingBufferWithOverwriting( unsigned int numberOfCell, 
                                                               unsigned int cellSize, 
                                                               int isOverwriting ); 

/**
 *  @brief Delete the ring buffer
 *  @warning This function free memory
 *  @param ppRingBuff address of the pointer on the ring buffer to delete
 *  @see NETWORK_NewRingBuffer()
 *  @see NETWORK_NewRingBufferWithOverwriting()
**/
void NETWORK_DeleteRingBuffer(network_ringBuffer_t** ppRingBuff);

/**
 *  @brief Add the new data at the back of the ring buffer
 *  @param pRingBuff pointer on the ring buffer
 *  @param[in] newData pointer on the data to add
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_RingBuffPushBack( network_ringBuffer_t* pRingBuff, const void* pNewData );

/**
 *  @brief Pop the oldest data
 *  @param pRingBuff pointer on the ring buffer
 *  @param[out] popData pointer on the data popped
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_RingBuffPopFront( network_ringBuffer_t* pRingBuff, void* pPopData );

/**
 *  @brief Return the number of free cell of the ring buffer
 *  @param pRingBuff pointer on the ring buffer
 *  @return number of free cell of the ring buffer 
**/
static inline int NETWORK_RingBuffGetFreeCellNb( const network_ringBuffer_t* pRingBuff )
{
    return pRingBuff->numberOfCell - ( 
            (pRingBuff->indexInput - pRingBuff->indexOutput) / pRingBuff->cellSize );
}

/**
 *  @brief Check if the ring buffer is empty
 *  @param pRingBuff pointer on the ring buffer
 *  @return equal to 1 if the ring buffer is empty else 0
**/
static inline int NETWORK_RingBuffIsEmpty( const network_ringBuffer_t* pRingBuff )
{
    return pRingBuff->indexInput == pRingBuff->indexOutput;
}

/**
 *  @brief Return a pointer on the front data
 *  @param pRingBuff pointer on the ring buffer
 *  @param[out] pFrontData pointer on the front data
 *  @return error eNETWORK_Error
**/
eNETWORK_Error NETWORK_RingBuffFront( network_ringBuffer_t* pRingBuff, void* pFrontData );

/**
 *  @brief Clean the ring buffer
 *  @param pRingBuff pointer on the ring buffer
**/
static inline void NETWORK_RingBuffClean( network_ringBuffer_t* pRingBuff )
{
    pRingBuff->indexInput = pRingBuff->indexOutput;
}

/**
 *  @brief Print the state of the ring buffer
 *  @param pRingBuff pointer on the ring buffer
**/
void NETWORK_RingBuffPrint( network_ringBuffer_t* pRingBuff );

/**
 *  @brief Print the contents of the ring buffer
 *  @param pRingBuff pointer on the ring buffer
**/
void NETWORK_RingBuffDataPrint( network_ringBuffer_t* pRingBuff );

#endif // _NETWORK_RINGBUFFER_H_

