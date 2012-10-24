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
 *  @brief used for buffering the acknowledge commands 
**/
typedef struct netWork_ringBuffer_t  
{
    void* dataBuffer;
    //unsigned int buffFreeCellNb;
    unsigned int buffIndexInput;
    unsigned int buffIndexOutput;
    unsigned int buffSize;
    unsigned int buffCellSize;
    unsigned int overwriting;
    sal_mutex_t mutex;

}netWork_ringBuffer_t;

/**
 *  @brief Create a new ring buffer
 * 	@param buffSize number of data cell of the ring buffer
 * 	@param buffCellSize size of one data cell of the ring buffer
 * 	@return Pointer on the new ring buffer
 * 	@post Call deleteRingBuffer()
 * 	@see newRingBufferWithOverwriting()
**/
netWork_ringBuffer_t* newRingBuffer(	unsigned int buffSize, unsigned int buffCellSize); 


/**
 *  @brief Create a new ring buffer
 * 	@param buffSize number of data cell of the ring buffer
 * 	@param buffCellSize size of one data cell of the ring buffer
 * 	@param overwriting allow the overwriting if the buffer is full
 *	@post Call deleteRingBuffer()
 * 	@return Pointer on the new ring buffer
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
/*inline*/ unsigned int ringBuffGetFreeCellNb(const netWork_ringBuffer_t* pRingBuff);
/*
{
	return pRingBuff->buffIndexOutput - pRingBuff->buffIndexInput ;
}
*/

/**
 *  @brief check if the buffer is empty
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return 1 if the buffer is empty else 0
**/
/*inline*/ int ringBuffIsEmpty(const netWork_ringBuffer_t* pRingBuff);
/*
{
	return ( pRingBuff->buffSize == ringBuffGetFreeCellNb(pRingBuff) );
}
*/

/**
 *  @brief check if the buffer is full
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return 1 if the buffer is full else 0
**/
/*inline*/ //int ringBuffIsFull(const netWork_ringBuffer_t* pRingBuff);
/*
{
	return ( ringBuffGetFreeCellNb(pRingBuff) < pRingBuff->buffSize ); /// !!! no
}
*/

/**
 *  @brief return a pointer on the front data
 * 	@param pRingBuff pointer on the ring buffer
 * 	@param pFrontData pointer on the data popped
 * 	@return error equal 1 if the buffer is empty 
**/
int ringBuffFront(netWork_ringBuffer_t* pRingBuff, void* pFrontData);

/**
 *  @brief clean the buffer
 * 	@param pRingBuff pointer on the ring buffer
**/
void ringBuffClean(netWork_ringBuffer_t* pRingBuff);

#endif // _RING_BUFFER_H_

