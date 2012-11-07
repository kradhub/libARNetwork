/**
 *	@file singleBuffer.h
 *  @brief single buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _BUFFER_H_
#define _BUFFER_H_

// static :

//Enumerations :


//Structures :

/**
 *  @brief used for buffering 
**/
typedef struct netWork_buffer_t
{
    void* 	pStart;
	void* 	pFront;
	void* 	pEnd;
	unsigned int buffSize;
    unsigned int buffCellSize;
	sal_mutex_t mutex;
}netWork_buffer_t;


/**
 *  @brief Create a buffer
 *	@post Call deleteBuffPilotCmd
 * 	@return Pointer on the new buffer of piloting commands
**/
netWork_buffer_t* newBuffer(unsigned int buffSize, unsigned int buffCellSize);

/**
 *  @brief Delete the buffer of piloting command
 * 	@param ppBuffPilotCmd address of the pointer on the buffer of piloting commands
 * 	@see newBuffPilotCmd()
**/
void deleteBuffer(netWork_buffer_t** ppBuffer);

/**
 *  @brief return the number of free cell of the buffer
 * 	@param pRingBuff pointer on the ring buffer
 * 	@return number of free cell of the buffer 
**/
/*inline*/ unsigned int bufferGetFreeCellNb(const netWork_buffer_t* pBuffer);

int bufferIsEmpty(netWork_buffer_t* pBuffer);

void bufferClean(netWork_buffer_t* pBuffer);

void bufferPrint(netWork_buffer_t* pBuffer);

void bufferDataPrint(netWork_buffer_t* pBuffer);

#endif // _BUFFER_H_

