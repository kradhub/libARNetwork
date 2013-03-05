/**
 *  @file ARNETWORK_Buffer.h
 *  @brief basic buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_BUFFER_PRIVATE_H_
#define _ARNETWORK_BUFFER_PRIVATE_H_

/**
 *  @brief basic buffer 
 *  @warning before to be used the buffer must be created through ARNETWORK_Buffer_New()
 *  @post after its using the buffer must be deleted through ARNETWORK_Buffer_Delete()
**/
typedef struct
{
    uint8_t *startPtr; /**< Pointer on the first data*/
    uint8_t *frontPtr; /**< Pointer on the last data*/
    uint8_t *endPtr; /**< Pointer on the end of the buffer*/
    unsigned int numberOfCell; /**< Maximum number of data stored*/
    unsigned int cellSize; /**< Size of one data in byte*/
}ARNETWORK_Buffer_t;

/**
 *  @brief Create a new buffer
 *  @warning This function allocate memory
 *  @param[in] numberOfCell Maximum number of data cell of the buffer
 *  @param[in] cellSize size of one data cell of the buffe
 *  @post ARNETWORK_Buffer_Delete() must be called to delete the buffer and free the memory allocated
 *  @return Pointer on the new buffer
 *  @see ARNETWORK_Buffer_Delete()
**/
ARNETWORK_Buffer_t* ARNETWORK_Buffer_New(unsigned int numberOfCell, unsigned int cellSize);

/**
 *  @brief Delete the buffer
 *  @warning This function free memory
 *  @param bufferPtrAddr address of the pointer on the buffer to delete
 *  @see ARNETWORK_Buffer_New()
**/
void ARNETWORK_Buffer_Delete(ARNETWORK_Buffer_t **bufferPtrAddr);

/**
 *  @brief Return the number of free cell of the buffer
 *  @param bufferPtr pointer on the buffer
 *  @return number of free cell of the buffer 
**/
static inline unsigned int ARNETWORK_Buffer_GetFreeCellNumber(const ARNETWORK_Buffer_t *bufferPtr)
{
    return (bufferPtr->endPtr - bufferPtr->frontPtr) / bufferPtr->cellSize;
}

/**
 *  @brief Check if the buffer is empty
 *  @param bufferPtr pointer on the buffer
 *  @return equal 1 if the buffer is empty else 0
**/
static inline int ARNETWORK_Buffer_IsEmpty(ARNETWORK_Buffer_t *bufferPtr)
{
    return (bufferPtr->startPtr == bufferPtr->frontPtr) ? 1 : 0;
}

/**
 *  @brief Clean the buffer
 *  @param bufferPtr pointer on the buffer
**/
static inline void ARNETWORK_Buffer_Clean(ARNETWORK_Buffer_t *bufferPtr)
{
    bufferPtr->frontPtr = bufferPtr->startPtr;
}

/**
 *  @brief Print the state of the buffer
 *  @param bufferPtr pointer on the buffer
**/
void ARNETWORK_Buffer_Print(ARNETWORK_Buffer_t *bufferPtr);

/**
 *  @brief Print the contents of the buffer
 *  @param bufferPtr pointer on the buffer
**/
void ARNETWORK_Buffer_DataPrint(ARNETWORK_Buffer_t *bufferPtr);

#endif /** _ARNETWORK_BUFFER_PRIVATE_H_ */

