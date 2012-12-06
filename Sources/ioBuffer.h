/**
 *	@file ioBuffer.h
 *  @brief input or output buffer, used by libNetwork/receiver or libNetwork/sender
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_IOBUFFER_H_
#define _NETWORK_IOBUFFER_H_

#include <libNetwork/frame.h>
#include "ringBuffer.h"
#include <libNetwork/paramNewIoBuffer.h>

/*****************************************
 * 
 * 			IoBuffer header:
 *
******************************************/

/**
 *  @brief Input buffer used by libNetwork/sender or output buffer used by libNetwork/receiver
 * 	@warning before to be used the inOutBuffer must be created through NETWORK_NewIoBuffer()
 * 	@post after its using the inOutBuffer must be deleted through NETWORK_DeleteIotBuffer()
**/
typedef struct network_ioBuffer_t  
{
    int id;		/**< Identifier used to find the InOutBuffer in a table*/
    network_ringBuffer_t* pBuffer;	/**< Pointer on the ringBuffer used to store the data*/
    eNETWOK_Frame_Type dataType;		/**< Type of the data stored in the buffer*/
    int sendingWaitTime;	/**< Time in millisecond between 2 send when the InOutBuffer if used with a libNetwork/sender*/
    int ackTimeoutMs;	/**< Timeout in millisecond after retry to send the data when the InOutBuffer is used with a libNetwork/sender*/
    int nbOfRetry;	/**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork/sender*/
    //	timeoutCallback(network_ioBuffer_t* this)
    int deportedData; /**< Indicator of using data deported*/
    
    int isWaitAck;	/**< Indicator of waiting an acknowledgement  (1 = true | 0 = false). Must be accessed through NETWORK_IoBufferIsWaitAck()*/
    int seqWaitAck; /**< Sequence number of the acknowledge waiting if used with a libNetwork/sender or of the last command stored if used with a libNetwork/reveiver*/
    int waitTimeCount;	 /**< Counter of time to wait before the next sending*/
    int ackWaitTimeCount;	/**< Counter of time to wait before to consider a timeout without receiving an acknowledgement*/
    int retryCount;		/**< Counter of sending retry remaining before to consider a failure*/
    
    sal_mutex_t mutex;	/**< Mutex to take before to use the ringBuffer*/

}network_ioBuffer_t;

/**
 *  @brief Create a new input or output buffer
 * 	@warning This function allocate memory
 *	@post NETWORK_DeleteIotBuffer() must be called to delete the input or output buffer and free the memory allocated
 * 	@param[in] pParam Pointer on the parameters for the new input or output buffer
 * 	@return Pointer on the new input or output buffer
 * 	@see NETWORK_DeleteIotBuffer()
**/
network_ioBuffer_t* NETWORK_NewIoBuffer(const network_paramNewIoBuffer_t* pParam); 

/**
 *  @brief Delete the input or output buffer
 * 	@warning This function free memory
 * 	@param ppIoBuffer address of the pointer on the input or output buffer to delete
 *	@see NETWORK_NewIoBuffer()
**/
void NETWORK_DeleteIotBuffer( network_ioBuffer_t** ppIoBuffer );

/**
 *  @brief Receive an acknowledgement to a inOutBuffer.
 * 	@details If the inOutBuffer is waiting about an acknowledgement and seqNum is equal to the sequence number waited, the inOutBuffer pops the last data and delete its is waiting acknowledgement.
 * 	@param[in] pIoBuffer Pointer on the input or output buffer
 * 	@param[in] seqNum sequence number of the acknowledgement
 * 	@return error equal to NETWORK_OK if the data has been correctly acknowledged otherwise equal to 1
**/
int NETWORK_IoBufferAckReceived( network_ioBuffer_t* pIoBuffer, int seqNum );

/**
 *  @brief Search a inOutBuffer from its identifier, in a table
 * 	@param[in] pptabInOutBuff address of the table of pointer of inOutBuffer
 * 	@param[in] tabSize size of the table of pointers of inOutBuffer
 * 	@param[in] id identifier of the inOutBuffer searched
 * 	@return address of the inOutBuffer with the identifier searched (equal to NULL if the inOutBuffer is not find)
**/
network_ioBuffer_t* NETWORK_IoBufferFromId( network_ioBuffer_t** pptabInOutBuff,
												int tabSize, int id );

/**
 *  @brief Get if the inOutBuffer is waiting an acknowledgement.
 * 	@param pIoBuffer Pointer on the input or output buffer
 * 	@return IsWaitAck equal to 1 if the inOutBuffer is waiting an acknowledgement otherwise equal to 0
**/
int NETWORK_IoBufferIsWaitAck( network_ioBuffer_t* pIoBuffer );

/**
 *  @brief call the callback of all deportedData with the NETWORK_DEPORTEDDATA_CALLBACK_FREE status.
 *  @warning the IoBuffer must store network_DeportedData_t
 * 	@param pIoBuffer Pointer on the input or output buffer of deportedData type
 *  @return error equal to NETWORK_OK if the data are correctly free otherwise see eNETWORK_Error
**/
int NETWORK_IoBufferFreeAlldeportedData( network_ioBuffer_t* pIoBuffer );

/**
 *  @brief delete the later data of the IoBuffer
 *  @warning used only when the data is correctly sent
 * 	@param pIoBuffer Pointer on the input or output buffer
 *  @param[in] callBackStatus status sent py the callBack in case where the data is deported 
 *  @return error equal to NETWORK_OK if the data are correctly deleted otherwise see eNETWORK_Error
**/
int NETWORK_IoBufferDeleteData( network_ioBuffer_t* pIoBuffer, int callBackStatus );

#endif // _NETWORK_IOBUFFER_H_

