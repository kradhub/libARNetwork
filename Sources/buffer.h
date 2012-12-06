/**
 *	@file buffer.h
 *  @brief basic buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_BUFFER_H_
#define _NETWORK_BUFFER_H_

/**
 *  @brief basic buffer 
 * 	@warning before to be used the buffer must be created through NETWORK_NewBuffer()
 * 	@post after its using the buffer must be deleted through NETWORK_DeleteBuffer()
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
 *	@post NETWORK_DeleteBuffer() must be called to delete the ring buffer and free the memory allocated
 * 	@return Pointer on the new buffer
 * 	@see NETWORK_DeleteBuffer()
**/
network_buffer_t* NETWORK_NewBuffer(unsigned int buffSize, unsigned int buffCellSize);

/**
 *  @brief Delete the buffer
 * 	@warning This function free memory
 * 	@param ppBuffPilotCmd address of the pointer on the buffer to delete
 * 	@see NETWORK_NewBuffer()
**/
void NETWORK_DeleteBuffer(network_buffer_t** ppBuffer);

/**
 *  @brief Return the number of free cell of the buffer
 * 	@param pBuffer pointer on the buffer
 * 	@return number of free cell of the buffer 
**/
static inline unsigned int NETWORK_BufferGetFreeCellNb(const network_buffer_t* pBuffer)
{
	return (pBuffer->pEnd - pBuffer->pFront) / pBuffer->buffCellSize;
}

/**
 *  @brief Check if the buffer is empty
 * 	@param pBuffer pointer on the buffer
 * 	@return equal 1 if the buffer is empty else 0
**/
static inline int NETWORK_BufferIsEmpty(network_buffer_t* pBuffer)
{
	return pBuffer->pStart == pBuffer->pFront;
}

/**
 *  @brief Clean the buffer
 * 	@param pBuffer pointer on the buffer
**/
static inline void NETWORK_BufferClean(network_buffer_t* pBuffer)
{
	pBuffer->pFront = pBuffer->pStart;
}

/**
 *  @brief Print the state of the buffer
 * 	@param pBuffer pointer on the buffer
**/
void NETWORK_BufferPrint(network_buffer_t* pBuffer);

/**
 *  @brief Print the contents of the buffer
 * 	@param pBuffer pointer on the buffer
**/
void NETWORK_BufferDataPrint(network_buffer_t* pBuffer);

#endif // _NETWORK_BUFFER_H_

