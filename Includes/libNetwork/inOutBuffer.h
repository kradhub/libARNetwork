/**
 *	@file inOutBuffer.h
 *  @brief input or output buffer used by libNetwork::receiver or libNetwork::sender
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _IN_OUT_BUFFER_H_
#define _IN_OUT_BUFFER_H_

#include <libNetwork/ringBuffer.h>
#include <libNetwork/common.h>

/**
 *  @brief used to set the parameters of a new inOutBuffer
**/
typedef struct network_paramNewInOutBuffer_t  
{
    int id;						/**< Identifier used to find the InOutBuffer in a list*/
    eAR_CMD_TYPE dataType;		/**< Type of the data stored in the buffer*/
    int sendingWaitTime;		/**< Time in millisecond between 2 send when the InOutBuffer is used with a libNetwork::sender*/
    int ackTimeoutMs;			/**< Timeout in millisecond after retry to send the data when the InOutBuffer is used with a libNetwork::sender*/
    int nbOfRetry;				/**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork::sender*/
    
    unsigned int buffSize;		/**< Maximum number of data stored*/
    unsigned int buffCellSize;	/**< Size of one data in byte*/
    int overwriting;			/**< Indicator of overwriting possibility (1 = true | 0 = false)*/

}network_paramNewInOutBuffer_t;

/**
 *  @brief input or output buffer used by libNetwork::receiver or libNetwork::sender
**/
typedef struct network_inOutBuffer_t  
{
    int id;		/**< Identifier used to find the InOutBuffer in a list*/
    network_ringBuffer_t* pBuffer;	
    eAR_CMD_TYPE dataType;		/**< Type of the data stored in the buffer*/
    int sendingWaitTime;	/**< Time in millisecond between 2 send when the InOutBuffer is used with a libNetwork::sender*/
    int ackTimeoutMs;	/**< Timeout in millisecond after retry to send the data when the InOutBuffer is used with a libNetwork::sender*/
    int nbOfRetry;	/**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork::sender*/
    //	timeoutCallback(network_inOutBuffer_t* this)
    
    int isWaitAck;
    int seqWaitAck;
    int waitTimeCount; // must be positive
    int ackWaitTimeCount;
    int retryCount;
    
    sal_mutex_t mutex;
    

}network_inOutBuffer_t;

/**
 *  @brief Create a new input or output buffer
 * 	@param pParam address of the parameters for a new input or output buffer
 *	@post Call deleteRingBuffer()
 * 	@return Pointer on the new ring buffer
**/
network_inOutBuffer_t* newInOutBuffer(const network_paramNewInOutBuffer_t *pParam); 

/**
 *  @brief Delete the input or output buffer
 * 	@param ppInOutBuff address of the pointer on the input or output buffer
 *	@see newRingBuffer()
**/
void deleteInOutBuffer(network_inOutBuffer_t** ppInOutBuff);

/**
 *  @brief received a acknowledge to a inOutBuffer in waiting about a acknowledge
 * 	@param pInOutBuff pointer on the input or output buffer
 * 	@param seqNum sequence number of the acknowledge
**/
void inOutBufferAckReceived(network_inOutBuffer_t* pInOutBuff, int seqNum);

/**
 *  @brief search a buffer with id in a table
 * 	@param pptabInOutBuff pointer on table of input or output buffers
 * 	@param tabSize size of the table of input or output buffers
 * 	@param id identifier of the inOutBuffer searched
 * 	@return address of the inOutBuffer with the id searched
**/
network_inOutBuffer_t* inOutBufferWithId(	network_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id);

int inOutBuffeIsWaitAck( network_inOutBuffer_t* pInOutBuff);


#endif // _IN_OUT_BUFFER_H_

