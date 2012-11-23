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

#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libSAL/endianness.h>

#include <libNetwork/common.h>
#include <libNetwork/buffer.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/*****************************************
 * 
 * 			private header:
 *
******************************************/
/**
 *  @brief get a command in the receiving buffer
 * 	@param pBuffsend the pointer on the receiver
 * 	@param pCmd adress of the pointer on the command
 * 	@return error 1 if buffer is empty
 *	@pre only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
int NETWORK_GetCmd(network_receiver_t* pReceiver, uint8_t** ppCmd);

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
		
		pReceiver->pRecvBuffer = newBuffer(recvBufferSize,1);
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
			deleteBuffer( &(pReceiver->pRecvBuffer) );
	
			free(pReceiver);
		}
		*ppReceiver = NULL;
	}
}

void* NETWORK_RunReceivingThread(void* data)
{	
	/** -- Manage the reception of the data on the Receiver' scoket. -- */
	
	/** local declarations */
	network_receiver_t* pReceiver = data;
	UNION_CMD recvCmd;
	network_ioBuffer_t* pOutBufferTemp = NULL;
	int pushError = 0;
	
	/** initialization local */
	recvCmd.pTabUint8 = NULL;
	
	while( pReceiver->isAlive )
	{	
		/** wait a receipt */
		if( NETWORK_ReceiverRead( pReceiver ) > 0)
		{	
			/** for each command present in the receiver buffer */		
			while( !NETWORK_GetCmd( pReceiver, &(recvCmd.pTabUint8) ) )
			{	
				/** management by the command type */			
				switch (recvCmd.pCmd->type)
				{
					case CMD_TYPE_ACK:
						sal_print(PRINT_DEBUG,"	- TYPE: CMD_TYPE_ACK | SEQ:%d | ID:%d \n",
												recvCmd.pCmd->seq, recvCmd.pCmd->id);
												
						/** transmit the acknowledgement to the sender */
						NETWORK_SenderAckReceived( pReceiver->pSender,idAckToIdInput(recvCmd.pCmd->id),
											(int) recvCmd.pTabUint8[AR_CMD_INDEX_DATA] );
					break;
					
					case CMD_TYPE_DATA:
						sal_print(PRINT_DEBUG," - TYPE: CMD_TYPE_DATA | SEQ:%d | ID:%d \n",
												recvCmd.pCmd->seq, recvCmd.pCmd->id);
						
						/** push the data received in the output buffer targeted */
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															recvCmd.pCmd->id);
						
						if(pOutBufferTemp != NULL)
						{
							ringBuffPushBack(	pOutBufferTemp->pBuffer,
												( recvCmd.pTabUint8 + AR_CMD_INDEX_DATA ) );
						}							
					break;
					
					case CMD_TYPE_DATA_WITH_ACK:
						sal_print(PRINT_DEBUG," - TYPE: CMD_TYPE_DATA_WITH_ACK | SEQ:%d | ID:%d \n", 
													recvCmd.pCmd->seq, recvCmd.pCmd->id);
						
						/** 
						 * push the data received in the output buffer targeted, 
						 * save the sequence of the command and return an acknowledgement
						**/
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															recvCmd.pCmd->id);
						if(pOutBufferTemp != NULL)
						{
							/** OutBuffer->seqWaitAck used to save the last seq */
							if( recvCmd.pCmd->seq != pOutBufferTemp->seqWaitAck )
							{
								pushError = ringBuffPushBack(	pOutBufferTemp->pBuffer,
														&(recvCmd.pTabUint8[AR_CMD_INDEX_DATA]));
								if( !pushError)
								{
									NETWORK_ReturnASK(pReceiver, recvCmd.pCmd->id, recvCmd.pCmd->seq);
									pOutBufferTemp->seqWaitAck = recvCmd.pCmd->seq;
								}
							}
						}	
												
					break;
					
					default:
						sal_print(PRINT_WARNING," !!! command type not known !!! \n");
					break;
				}
			}
			bufferClean(pReceiver->pRecvBuffer);
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

int NETWORK_GetCmd(network_receiver_t* pReceiver, uint8_t** ppCmd)
{
	/** -- get a command in the receiving buffer-- */
	
	/** local declarations */
	int error = 1;
	UNION_CMD recvCmd ;

	if(*ppCmd == NULL)
	{
		*ppCmd = pReceiver->pRecvBuffer->pStart;
	}
	else
	{
		/** point the next command */
		(*ppCmd) += *( (uint32_t*) (*ppCmd + AR_CMD_INDEX_SIZE) );
	}
	
	recvCmd.pTabUint8 = *ppCmd;
	
	/** check if the buffer stores enough data */
	if(*ppCmd <= (uint8_t*) pReceiver->pRecvBuffer->pFront - AR_CMD_HEADER_SIZE)
	{	
		/** pass the command to the host endian */
		recvCmd.pCmd->type = dtohl( *( (uint32_t*) (*ppCmd + AR_CMD_INDEX_TYPE) ) );
		recvCmd.pCmd->id = dtohl( *( (uint32_t*) (*ppCmd + AR_CMD_INDEX_ID) ) );
		recvCmd.pCmd->seq = dtohl( *( (uint32_t*) (*ppCmd + AR_CMD_INDEX_SEQ) ) );
		recvCmd.pCmd->size = dtohl( *( (uint32_t*) (*ppCmd + AR_CMD_INDEX_SIZE) ) );
		
		if(*ppCmd <= (uint8_t*) pReceiver->pRecvBuffer->pFront - recvCmd.pCmd->size)
		{
			error = 0;
		}
	}
	
	if( error != 0)
	{
		*ppCmd = NULL;
	}
	
	return error;
}
