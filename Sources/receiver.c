/**
 *	@file receiver.c
 *  @brief manage the data received, used by libNetwork/network
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>

#include <stddef.h>

#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libSAL/endianness.h>

#include <libNetwork/error.h>
#include <libNetwork/frame.h>
#include "buffer.h"
#include <libNetwork/deportedData.h>
#include "ioBuffer.h"
#include "sender.h"

#include "receiver.h"

/*****************************************
 * 
 * 			private header:
 *
******************************************/

/**
 *  @brief get the next Frame of the receiving buffer
 * 	@param pReceiver the pointer on the receiver
 * 	@param prevFrame[in] pointer on the previous frame
 * 	@return nextFrame pointer on the next frame
 *	@pre only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
network_frame_t* NETWORK_ReceiverGetNextFrame( network_receiver_t* pReceiver,
                                                 network_frame_t* prevFrame );

/**
 *  @brief copy the data received to the output buffer
 * 	@param pReceiver the pointer on the receiver
 * 	@param pOutBuffer[in] pointer on the output buffer
 *  @param pFrame[in] pointer on the frame received
 * 	@return error equal to NETWORK_OK if the Bind if successful otherwise equal to error of eNETWORK_Error.
 *	@pre only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
int NETWORK_ReceiverCopyDataRecv( network_receiver_t* pReceiver,
                                   network_ioBuffer_t* pOutBuffer,
                                   network_frame_t* pFrame );
                                   
/**
 *  @brief call back use to free deported data 
 *  @param[in] OutBufferId IoBuffer identifier of the IoBuffer is calling back
 *  @param[in] pData pointer on the data
 *  @param[in] status status indicating the reason of the callback. eNETWORK_DEPORTEDDATA_callback_Status type
 *  @return   
 *  @see eNETWORK_DEPORTEDDATA_callback_Status
**/
int NETWORK_freedeportedData(int OutBufferId, void* pData, int status);

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_receiver_t* NETWORK_NewReceiver(	unsigned int recvBufferSize, unsigned int numOfOutputBuff,
									network_ioBuffer_t** pptab_output)
{	
	/** -- Create a new receiver -- */
	
	/** local declarations */
	network_receiver_t* pReceiver = NULL;
	int error = NETWORK_OK;
	
	/** Create the receiver */
	pReceiver =  malloc( sizeof(network_receiver_t) );
	
	if(pReceiver)
	{
		pReceiver->isAlive = 1;
		pReceiver->pSender = NULL;
		
		pReceiver->numOfOutputBuff = numOfOutputBuff;
		pReceiver->pptab_outputBuffer = pptab_output;
		
		pReceiver->pRecvBuffer = NETWORK_NewBuffer(recvBufferSize,1);
		if(pReceiver->pRecvBuffer == NULL)
		{
			error = NETWORK_ERROR_NEW_BUFFER;
		}
	
		/** delete the receiver if an error occurred */
		if(error != NETWORK_OK)
		{
            SAL_PRINT(PRINT_ERROR,"error: %d occurred \n", error );
			NETWORK_DeleteReceiver(&pReceiver);
		}
	}
	
	return pReceiver;
}

void NETWORK_DeleteReceiver(network_receiver_t** ppReceiver)
{
	/** -- Delete the Receiver -- */
	
	/** local declarations */
	network_receiver_t* pReceiver = NULL;
	
	if(ppReceiver)
	{
		pReceiver = *ppReceiver;
		
		if(pReceiver)
		{
			NETWORK_DeleteBuffer( &(pReceiver->pRecvBuffer) );
	
			free(pReceiver);
            pReceiver = NULL;
		}
		*ppReceiver = NULL;
	}
}

void* NETWORK_RunReceivingThread(void* data)
{	
	/** -- Manage the reception of the data on the Receiver' scoket. -- */
	
	/** local declarations */
	network_receiver_t* pReceiver = data;
    network_frame_t* pFrame = NULL;
	network_ioBuffer_t* pOutBufferTemp = NULL;
	int error = 0;

	
	while( pReceiver->isAlive )
	{	
		/** wait a receipt */
		if( NETWORK_ReceiverRead( pReceiver ) > 0)
		{	
			/** for each command present in the receiver buffer */		
            pFrame = NETWORK_ReceiverGetNextFrame(pReceiver, pFrame);
            while( pFrame != NULL )
			{	
				/** management by the command type */			
				switch (pFrame->type)
				{
					case network_frame_t_TYPE_ACK:
						SAL_PRINT(PRINT_DEBUG," - TYPE: network_frame_t_TYPE_ACK | SEQ:%d | ID:%d \n",
												pFrame->seq, pFrame->id);
												
						/** transmit the acknowledgement to the sender */
						error = NETWORK_SenderAckReceived( pReceiver->pSender,
                                                           idAckToIdInput(pFrame->id),
                                                           *( (int*) &pFrame->data ) );
                        if( error != NETWORK_OK )
                        {
                            SAL_PRINT(PRINT_ERROR,"acknowledge received, error: %d occurred \n", error);
                        }
                        
					break;
					
					case network_frame_t_TYPE_DATA:
						SAL_PRINT(PRINT_DEBUG," - TYPE: network_frame_t_TYPE_DATA | SEQ:%d | ID:%d \n",
												pFrame->seq, pFrame->id);
                        
						/** push the data received in the output buffer targeted */
						pOutBufferTemp = NETWORK_IoBufferFromId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															pFrame->id);
						
						if(pOutBufferTemp != NULL)
						{
                            error = NETWORK_ReceiverCopyDataRecv( pReceiver, pOutBufferTemp, pFrame);
                            
                            if( error != NETWORK_OK )
                            {
                                SAL_PRINT(PRINT_ERROR,"data received, error: %d occurred \n", error);
                            }
                            
						}							
					break;
					
					case network_frame_t_TYPE_DATA_WITH_ACK:
						SAL_PRINT(PRINT_DEBUG," - TYPE: network_frame_t_TYPE_DATA_WITH_ACK | SEQ:%d | ID:%d \n", 
													pFrame->seq, pFrame->id);
						
						/** 
						 * push the data received in the output buffer targeted, 
						 * save the sequence of the command and return an acknowledgement
						**/
						pOutBufferTemp = NETWORK_IoBufferFromId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															pFrame->id);
						if(pOutBufferTemp != NULL)
						{
							/** OutBuffer->seqWaitAck used to save the last seq */
							if( pFrame->seq != pOutBufferTemp->seqWaitAck )
							{        
                                error = NETWORK_ReceiverCopyDataRecv( pReceiver, pOutBufferTemp,
                                                                          pFrame);
								if( error == NETWORK_OK)
								{
									NETWORK_ReturnASK(pReceiver, pFrame->id, pFrame->seq);
                                    
									pOutBufferTemp->seqWaitAck = pFrame->seq;
								}
                                else
                                {
                                    SAL_PRINT(PRINT_ERROR,"data acknowledgeed received, error: %d occurred \n", error);
                                }
							}
						}	
												
					break;
					
					default:
						SAL_PRINT(PRINT_WARNING," !!! command type not known !!! \n");
					break;
				}
                /** get the next frame*/
                pFrame = NETWORK_ReceiverGetNextFrame(pReceiver, pFrame);
			}
			NETWORK_BufferClean(pReceiver->pRecvBuffer);
		}
	}

    sal_close(pReceiver->socket);
       
    return NULL;
}

void NETWORK_StopReceiver(network_receiver_t* pReceiver)
{
	/** -- stop the reception -- */
	pReceiver->isAlive = 0;
}


void NETWORK_ReturnASK(network_receiver_t* pReceiver, int id, int seq)
{
	/** -- return an acknowledgement -- */
	
	/** local declarations */
	network_ioBuffer_t* pBufferASK = NETWORK_IoBufferFromId(	pReceiver->pptab_outputBuffer,
																pReceiver->numOfOutputBuff,
																idOutputToIdAck(id) );
                                                                
	if(pBufferASK != NULL)
	{
		NETWORK_RingBuffPushBack(pBufferASK->pBuffer, &seq );
	}
}

int NETWORK_ReceiverRead(network_receiver_t* pReceiver)
{
	/** -- receiving data present on the socket -- */
	
	/** local declarations */
	int readDataSize =  sal_recv(	pReceiver->socket, pReceiver->pRecvBuffer->pStart,
								pReceiver->pRecvBuffer->buffSize, 0);

	pReceiver->pRecvBuffer->pFront =  pReceiver->pRecvBuffer->pStart + readDataSize;
	
	return readDataSize;
}

int NETWORK_ReceiverBind(network_receiver_t* pReceiver, unsigned short port, int timeoutSec)
{
	/** -- receiving data present on the socket -- */
	
	/** local declarations */
	struct timeval timeout;  
	struct sockaddr_in recvSin;
	
	/** socket initialization */
	recvSin.sin_addr.s_addr = htonl(INADDR_ANY);   
	recvSin.sin_family = AF_INET;
	recvSin.sin_port = htons(port);
	
	pReceiver->socket = sal_socket( AF_INET, SOCK_DGRAM, 0 );
	
	/** set the socket timeout */
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0; 
	sal_setsockopt(pReceiver->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	
	return sal_bind(pReceiver->socket, (struct sockaddr*)&recvSin, sizeof(recvSin));
}

/*****************************************
 * 
 * 			private implementation:
 *
******************************************/

network_frame_t* NETWORK_ReceiverGetNextFrame( network_receiver_t* pReceiver, 
                                                 network_frame_t* prevFrame)
{
    /** -- get the next Frame of the receiving buffer -- */
	
	/** local declarations */
    network_frame_t* nextFrame = NULL;
    
    if (prevFrame == NULL)
    {
        /** if no previous frame, get the first frame of the receiving buffer */
        nextFrame = pReceiver->pRecvBuffer->pStart;
    }
    else
    {
        /** get the next frame */
        nextFrame = (network_frame_t*) (prevFrame + prevFrame->size) ;
    }
    
    /** if the receiving buffer not contain enough data */
    if ( (void*) nextFrame > pReceiver->pRecvBuffer->pFront - offsetof(network_frame_t, data) )
    {
        nextFrame = NULL;
    }
    
    return nextFrame;
}

int NETWORK_ReceiverCopyDataRecv( network_receiver_t* pReceiver,
                                   network_ioBuffer_t* pOutBuffer,
                                   network_frame_t* pFrame )
{
    /** -- copy the data received to the output buffer -- */
    
    /** local declarations */
    int error = NETWORK_OK;
    network_DeportedData_t deportedDataTemp;
    
    if( pOutBuffer->deportedData )
    {
        
        if( NETWORK_RingBuffGetFreeCellNb(pOutBuffer->pBuffer) )
        {
            /** alloc data deported */
            deportedDataTemp.dataSize = pFrame->size - offsetof( network_frame_t,data );
            deportedDataTemp.pData = malloc( deportedDataTemp.dataSize );
            deportedDataTemp.callback = &(NETWORK_freedeportedData);
            
            /** copy the data received in the space allocated */
            memcpy(deportedDataTemp.pData, &(pFrame->data), deportedDataTemp.dataSize);
            
            /** push the deportedDataTemp in the output buffer targeted */
            error = NETWORK_RingBuffPushBack( pOutBuffer->pBuffer, &deportedDataTemp );
        }
        else
        {
            error = NETWORK_ERROR_BUFFER_SIZE;
        }
    }
    else
    {
        /** push the data received in the output buffer targeted */
        error = NETWORK_RingBuffPushBack( pOutBuffer->pBuffer, &(pFrame->data) );
    }
    
    return error;
}

int NETWORK_freedeportedData(int OutBufferId, void* pData, int status)
{
    /** call back use to free deported data */
    
    free(pData);
    
    return NETWORK_OK;
}
