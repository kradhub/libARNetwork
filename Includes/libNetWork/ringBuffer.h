/**
 *	@file circularBuffer.h
 *  @brief circular buffer for the commands to send
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
typedef struct netWork_ringBuffer_t  
{
    void* dataBuffer;				/**< Pointer on the data buffer */
    unsigned int buffIndexInput;	/**< Index of the data input*/
    unsigned int buffIndexOutput;	/**< Index of the data output*/
    unsigned int buffSize;			/**< Maximum number of data stored*/
    unsigned int buffCellSize;		/**< Size of one data in byte*/
    unsigned int overwriting;		/**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    sal_mutex_t mutex;				/**< Mutex to take before use the ringBuffer*/

}netWork_ringBuffer_t;

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
netWork_ringBuffer_t* newRingBuffer(	unsigned int buffSize, unsigned int buffCellSize); 


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
netWork_ringBuffer_t* newRingBufferWithOverwriting(	unsigned int buffSize, 
														unsigned int buffCellSize, 
														int overwriting ); 

/**
 *  @brief Delete the ring buffer
 * 	@param ppRingBuff address of the pointer on the ring buffer
 *	@see newRingBuffer()
**/
void deleteRingBuffer(netWork_ringBuffer_t** ppRingBuff);

/**
 *  @brief Add the new data at the back of the ring buffer
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param newData pointer on the data to add
 * 	@return error equal 1 if the buffer is full 
**/
int ringBuffPushBack(netWork_ringBuffer_t* pRingBuff, const void* pNewData);

/**
 *  @brief pop the oldest data
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param popData pointer on the data popped
 * 	@return error equal 1 if the buffer is empty 
**/
int ringBuffPopFront(netWork_ringBuffer_t* pRingBuff, void* pPopData);

/**
 *  @brief return the number of free cell of the buffer
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return number of free cell of the buffer 
**/
extern inline int ringBuffGetFreeCellNb(const netWork_ringBuffer_t* pRingBuff)
{
	return pRingBuff->buffSize - ( 
			(pRingBuff->buffIndexInput - pRingBuff->buffIndexOutput) / pRingBuff->buffCellSize );
}


/**
 *  @brief check if the buffer is empty
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return 1 if the buffer is empty else 0
**/
extern inline int ringBuffIsEmpty(const netWork_ringBuffer_t* pRingBuff)
{
	return pRingBuff->buffIndexInput == pRingBuff->buffIndexOutput;
}

/**
 *  @brief return a pointer on the front data
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param pFrontData pointer on the data popped
 * 	@return error equal 1 if the buffer is empty 
**/
int ringBuffFront( netWork_ringBuffer_t* pRingBuff, void* pFrontData);

/**
 *  @brief clean the buffer
 * 	@param pRingBuff pointer on the ring buffer
**/
extern inline void ringBuffClean(netWork_ringBuffer_t* pRingBuff)

{
	pRingBuff->buffIndexInput = pRingBuff->buffIndexOutput;
}


/**
 *  @brief print the contents of the buffer
 * 	@param pRingBuff pointer on the ring buffer
**/
void ringBuffPrint(netWork_ringBuffer_t* pRingBuff);

void ringBuffDataPrint(netWork_ringBuffer_t* pRingBuff);

#endif // _RING_BUFFER_H_

