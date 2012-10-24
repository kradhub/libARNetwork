/**
 *	@file inOutBuffer.h
 *  @brief circular buffer for the commands to send
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _IN_OUT_BUFFER_H_
#define _IN_OUT_BUFFER_H_

#include <libNetWork/ringBuffer.h>

// static :

//Enumerations :


//Structures :

/**
 *  @brief used for pass the parameters to create a new InOutBuffer
**/
typedef struct netWork_paramNewInOutBuffer_t  
{
    int id;
    int needAck;
    int sendingWaitTime;
    unsigned int buffSize;
    unsigned int buffCellSize;

}netWork_paramNewInOutBuffer_t;

/**
 *  @brief used for buffering the acknowledge commands 
**/
typedef struct netWork_inOutBuffer_t  
{
    int id;
    int needAck;
    int sendingWaitTime;
    int isWaitAck;
    int seqWaitAck;
    int waitTimeCount; // must be positive
    netWork_ringBuffer_t* pBuffer;

}netWork_inOutBuffer_t;

/**
 *  @brief Create a new input or output buffer
 * 	@param pParam address of the parameters for a new input or output buffer
 *	@post Call deleteRingBuffer()
 * 	@return Pointer on the new ring buffer
**/
netWork_inOutBuffer_t* newInOutBuffer(const netWork_paramNewInOutBuffer_t *pParam); 

/**
 *  @brief Delete the input or output buffer
 * 	@param ppInOutBuff address of the pointer on the input or output buffer
 *	@see newRingBuffer()
**/
void deleteInOutBuffer(netWork_inOutBuffer_t** ppInOutBuff);

/**
 *  @brief transmit a acknowledge to a inOutBuffer in waiting about a acknowledge
 * 	@param pInOutBuff pointer on the input or output buffer
 * 	@param seqNum sequence number of the acknowledge
**/
void inOutBufferTransmitAck(netWork_inOutBuffer_t* pInOutBuff, int seqNum);

/**
 *  @brief search a buffer with id in a table
 * 	@param pptabInOutBuff pointer on table of input or output buffers
 * 	@param tabSize size of the table of input or output buffers
 * 	@param id identifier of the inOutBuffer searched
 * 	@return address of the inOutBuffer with the id searched
**/
netWork_inOutBuffer_t* inOutBufferWithId(	netWork_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id);


#endif // _IN_OUT_BUFFER_H_

