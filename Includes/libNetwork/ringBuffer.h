/**
 *	@file ringBuffer.h
 *  @brief Ring buffer, multithread safe with overwriting possibility.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <libSAL/mutex.h>

// static :

//Enumerations :


//Structures :

/**
 *  @brief Basic ring buffer, multithread safe
**/
typedef struct network_ringBuffer_t  
{
    void* dataBuffer;				/**< Pointer on the data buffer*/
    unsigned int buffIndexInput;	/**< Index of the data input*/
    unsigned int buffIndexOutput;	/**< Index of the data output*/
    unsigned int buffSize;			/**< Maximum number of data stored*/
    unsigned int buffCellSize;		/**< Size of one data in byte*/
    unsigned int overwriting;		/**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    sal_mutex_t mutex;				/**< Mutex to take before use the ringBuffer*/

}network_ringBuffer_t;

/**
 *  @brief Create a new ring buffer
 * 	@warning This function allocate memory
 * 	@param buffSize Maximum number of data cell of the ring buffer
 * 	@param buffCellSize size of one data cell of the ring buffer
 * 	@return Pointer on the new ring buffer
 * 	@post deleteRingBuffer() must be called to delete the ring buffer and free the memory allocated
 * 	@see newRingBufferWithOverwriting()
 * 	@see deleteRingBuffer()
**/
network_ringBuffer_t* newRingBuffer(	unsigned int buffSize, unsigned int buffCellSize); 


/**
 *  @brief Create a new ring buffer.
 * 	@warning This function allocate memory
 * 	@param buffSize Maximum number of data cell of the ring buffer
 * 	@param buffCellSize size of one data cell of the ring buffer
 * 	@param overwriting set to 1 allow the overwriting if the buffer is full otherwise set 0
 * 	@return Pointer on the new ring buffer
 * 	@post deleteRingBuffer() must be called to delete the ring buffer and free the memory allocated
 * 	@see newRingBufferWithOverwriting()
 * 	@see deleteRingBuffer()
**/
network_ringBuffer_t* newRingBufferWithOverwriting(	unsigned int buffSize, 
														unsigned int buffCellSize, 
														int overwriting ); 

/**
 *  @brief Delete the ring buffer
 * 	@warning This function free memory
 * 	@param ppRingBuff address of the pointer on the ring buffer to delete
 *	@see newRingBuffer()
 * 	@see newRingBufferWithOverwriting()
**/
void deleteRingBuffer(network_ringBuffer_t** ppRingBuff);

/**
 *  @brief Add the new data at the back of the ring buffer
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param newData pointer on the data to add
 * 	@return error equal 1 if the data is not correctly pushed
**/
int ringBuffPushBack(network_ringBuffer_t* pRingBuff, const void* pNewData);

/**
 *  @brief Pop the oldest data
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param popData pointer on the data popped
 * 	@return error equal 1 if the buffer is empty 
**/
int ringBuffPopFront(network_ringBuffer_t* pRingBuff, void* pPopData);

/**
 *  @brief Return the number of free cell of the ring buffer
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return number of free cell of the ring buffer 
**/
extern inline int ringBuffGetFreeCellNb(const network_ringBuffer_t* pRingBuff)
{
	return pRingBuff->buffSize - ( 
			(pRingBuff->buffIndexInput - pRingBuff->buffIndexOutput) / pRingBuff->buffCellSize );
}

/**
 *  @brief Check if the ring buffer is empty
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return equal 1 if the ring buffer is empty else 0
**/
extern inline int ringBuffIsEmpty(const network_ringBuffer_t* pRingBuff)
{
	return pRingBuff->buffIndexInput == pRingBuff->buffIndexOutput;
}

/**
 *  @brief Return a pointer on the front data
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param pFrontData pointer on the data popped
 * 	@return error equal 1 if the ring buffer is empty 
**/
int ringBuffFront( network_ringBuffer_t* pRingBuff, void* pFrontData);

/**
 *  @brief Clean the ring buffer
 * 	@param pRingBuff pointer on the ring buffer
**/
extern inline void ringBuffClean(network_ringBuffer_t* pRingBuff)
{
	pRingBuff->buffIndexInput = pRingBuff->buffIndexOutput;
}

/**
 *  @brief Print the state of the ring buffer
 * 	@param pRingBuff pointer on the ring buffer
**/
void ringBuffPrint(network_ringBuffer_t* pRingBuff);

/**
 *  @brief Print the contents of the ring buffer
 * 	@param pRingBuff pointer on the ring buffer
**/
void ringBuffDataPrint(network_ringBuffer_t* pRingBuff);

#endif // _RING_BUFFER_H_

