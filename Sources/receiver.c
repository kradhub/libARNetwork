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

#include <libNetwork/common.h>
#include <libNetwork/buffer.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>

#include "receiver.h"



typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

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
	int error = 0;
	
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
			error = 1;
		}
	
		/** delete the receiver if an error occurred */
		if(error)
		{
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
	int pushError = 0;

	
	while( pReceiver->isAlive )
	{	
		/** wait a receipt */
		if( NETWORK_ReceiverRead( pReceiver ) > 0)
		{	
			/** for each command present in the receiver buffer */		
            pFrame = NETWORK_GetNextFrame(pReceiver, pFrame);
            while( pFrame != NULL )
			{	
				/** management by the command type */			
				switch (pFrame->type)
				{
					case network_frame_t_TYPE_ACK:
						sal_print(PRINT_DEBUG,"	- TYPE: network_frame_t_TYPE_ACK | SEQ:%d | ID:%d \n",
												pFrame->seq, pFrame->id);
												
						/** transmit the acknowledgement to the sender */
						NETWORK_SenderAckReceived( pReceiver->pSender,idAckToIdInput(pFrame->id),
											(int) pFrame->data );
					break;
					
					case network_frame_t_TYPE_DATA:
						sal_print(PRINT_DEBUG," - TYPE: network_frame_t_TYPE_DATA | SEQ:%d | ID:%d \n",
												pFrame->seq, pFrame->id);
						
						/** push the data received in the output buffer targeted */
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															pFrame->id);
						
						if(pOutBufferTemp != NULL)
						{
							ringBuffPushBack(	pOutBufferTemp->pBuffer, &(pFrame->data) );
						}							
					break;
					
					case network_frame_t_TYPE_DATA_WITH_ACK:
						sal_print(PRINT_DEBUG," - TYPE: network_frame_t_TYPE_DATA_WITH_ACK | SEQ:%d | ID:%d \n", 
													pFrame->seq, pFrame->id);
						
						/** 
						 * push the data received in the output buffer targeted, 
						 * save the sequence of the command and return an acknowledgement
						**/
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															pFrame->id);
						if(pOutBufferTemp != NULL)
						{
							/** OutBuffer->seqWaitAck used to save the last seq */
							if( pFrame->seq != pOutBufferTemp->seqWaitAck )
							{
								pushError = ringBuffPushBack( pOutBufferTemp->pBuffer, 
														&(pFrame->data) );
								if( !pushError)
								{
									NETWORK_ReturnASK(pReceiver, pFrame->id, pFrame->seq);
									pOutBufferTemp->seqWaitAck = pFrame->seq;
								}
							}
						}	
												
					break;
					
					default:
						sal_print(PRINT_WARNING," !!! command type not known !!! \n");
					break;
				}
                /** get the next frame*/
                pFrame = NETWORK_GetNextFrame(pReceiver, pFrame);
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
	network_ioBuffer_t* pBufferASK = inOutBufferWithId(	pReceiver->pptab_outputBuffer,
																pReceiver->numOfOutputBuff,
																idOutputToIdAck(id) );
	if(pBufferASK != NULL)
	{
		ringBuffPushBack(pBufferASK->pBuffer, &seq );
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
	SOCKADDR_IN recvSin;
	
	/** socket initialization */
	recvSin.sin_addr.s_addr = htonl(INADDR_ANY);   
	recvSin.sin_family = AF_INET;
	recvSin.sin_port = htons(port);
	
	pReceiver->socket = sal_socket( AF_INET, SOCK_DGRAM, 0 );
	
	/** set the socket timeout */
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0; 
	sal_setsockopt(pReceiver->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	
	return sal_bind(pReceiver->socket, (SOCKADDR*)&recvSin, sizeof(recvSin));
}

/*****************************************
 * 
 * 			private implementation:
 *
******************************************/

network_frame_t* NETWORK_GetNextFrame(network_receiver_t* pReceiver, network_frame_t* prevFrame)
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
