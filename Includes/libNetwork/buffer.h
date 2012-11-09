/**
 *	@file buffer.h
 *  @brief basic buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _BUFFER_H_
#define _BUFFER_H_

/**
 *  @brief basic buffer 
 * 	@warning before to be used the buffer must be created through newBuffer()
 * 	@post after its using the buffer must be deleted through deleteBuffer()
**/
typedef struct network_buffer_t
{
    void* 	pStart;					/**< Pointer on the first data*/
	void* 	pFront;					/**< Pointer on the last data*/
	void* 	pEnd;					/**< Pointer on the end of the buffer*/
	unsigned int buffSize;			/**< Maximum number of data stored*/
    unsigned int buffCellSize;		/**< Size of one data in byte*/
}network_buffer_t;

/**
 *  @brief Create a new buffer
 * 	@warning This function allocate memory
 * 	@param[in] buffSize Maximum number of data cell of the buffer
 * 	@param[in] buffCellSize size of one data cell of the buffe
 *	@post deleteBuffer() must be called to delete the ring buffer and free the memory allocated
 * 	@return Pointer on the new buffer
 * 	@see deleteBuffer()
**/
network_buffer_t* newBuffer(unsigned int buffSize, unsigned int buffCellSize);

/**
 *  @brief Delete the buffer
 * 	@warning This function free memory
 * 	@param ppBuffPilotCmd address of the pointer on the buffer to delete
 * 	@see newBuffer()
**/
void deleteBuffer(network_buffer_t** ppBuffer);

/**
 *  @brief Return the number of free cell of the buffer
 * 	@param pBuffer pointer on the buffer
 * 	@return number of free cell of the buffer 
**/
static inline unsigned int bufferGetFreeCellNb(const network_buffer_t* pBuffer)
{
	return (pBuffer->pEnd - pBuffer->pFront) / pBuffer->buffCellSize;
}

/**
 *  @brief Check if the buffer is empty
 * 	@param pBuffer pointer on the buffer
 * 	@return equal 1 if the buffer is empty else 0
**/
static inline int bufferIsEmpty(network_buffer_t* pBuffer)
{
	return pBuffer->pStart == pBuffer->pFront;
}

/**
 *  @brief Clean the buffer
 * 	@param pBuffer pointer on the buffer
**/
static inline void bufferClean(network_buffer_t* pBuffer)
{
	pBuffer->pFront = pBuffer->pStart;
}

/**
 *  @brief Print the state of the buffer
 * 	@param pBuffer pointer on the buffer
**/
void bufferPrint(network_buffer_t* pBuffer);

/**
 *  @brief Print the contents of the buffer
 * 	@param pBuffer pointer on the buffer
**/
void bufferDataPrint(network_buffer_t* pBuffer);

#endif // _BUFFER_H_

