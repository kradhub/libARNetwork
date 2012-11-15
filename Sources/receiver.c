/**
 *	@file receiver.c
 *  @brief manage the data receiving
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>

#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libSAL/endianness.h>

#include <libNetwork/common.h>
#include <libNetwork/buffer.h>
#include <libNetwork/inOutBuffer.h>
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
 *	@pre only call by runSendingThread()
 * 	@see runSendingThread()
**/
int getCmd(network_Receiver_t* pReceiver, uint8_t** ppCmd);


int initRecvBuffer(network_Receiver_t* pReceiver);

#define OUTPUT_PARAM_NUM 4
#define MICRO_SECOND 1000

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_Receiver_t* newReceiver(	unsigned int recvBufferSize, unsigned int numOfOutputBuff,
									network_inOutBuffer_t** pptab_output)
{	
	network_Receiver_t* pReceiver =  malloc( sizeof(network_Receiver_t));

	int error = 0;
	
	if(pReceiver)
	{
		pReceiver->isAlive = 1;
		pReceiver->pSender = NULL;
		
		pReceiver->numOfOutputBuff = numOfOutputBuff;
		pReceiver->pptab_outputBuffer = pptab_output;
		
		if(!error)
		{
			pReceiver->pRecvBuffer = newBuffer(recvBufferSize,1);
			if(pReceiver->pRecvBuffer == NULL)
			{
				error = 1;
			}
		}
	
		if(error)
		{
			deleteReceiver(&pReceiver);
		}
	}
	
	return pReceiver;
}

void deleteReceiver(network_Receiver_t** ppReceiver)
{
	network_Receiver_t* pReceiver = NULL;
	
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

void* runReceivingThread(void* data)
{	
	network_Receiver_t* pReceiver = data;
	
	UNION_CMD recvCmd;
	network_inOutBuffer_t* pOutBufferTemp = NULL;
	
	int pushError = 0;
	
	recvCmd.pTabUint8 = NULL;
	
	while( pReceiver->isAlive )
	{	
		if( receiverRead( pReceiver ) > 0)
		{
			//sal_print(PRINT_WARNING,"- read  Receiver: \n");
			
			while( !getCmd( pReceiver, &(recvCmd.pTabUint8) ) )
			{			
				switch (recvCmd.pCmd->type)
				{
					case CMD_TYPE_ACK:
						sal_print(PRINT_DEBUG,"	- TYPE: CMD_TYPE_ACK | SEQ:%d | ID:%d \n",
												recvCmd.pCmd->seq, recvCmd.pCmd->id);
												
						senderAckReceived( pReceiver->pSender,idAckToIdInput(recvCmd.pCmd->id),
											(int) recvCmd.pTabUint8[AR_CMD_INDEX_DATA] );
					break;
					
					case CMD_TYPE_DATA:
						sal_print(PRINT_DEBUG," - TYPE: CMD_TYPE_DATA | SEQ:%d | ID:%d \n",
												recvCmd.pCmd->seq, recvCmd.pCmd->id);
						
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
					
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->numOfOutputBuff,
															recvCmd.pCmd->id);
						if(pOutBufferTemp != NULL)
						{
							// OutBuffer->seqWaitAck used to save the last seq 
							if( recvCmd.pCmd->seq != pOutBufferTemp->seqWaitAck )
							{
								pushError = ringBuffPushBack(	pOutBufferTemp->pBuffer,
														&(recvCmd.pTabUint8[AR_CMD_INDEX_DATA]));
								if( !pushError)
								{
									returnASK(pReceiver, recvCmd.pCmd->id, recvCmd.pCmd->seq);
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

void stopReceiver(network_Receiver_t* pReceiver)
{
	pReceiver->isAlive = 0;
}

int getCmd(network_Receiver_t* pReceiver, uint8_t** ppCmd)
{
	int error = 1;
	UNION_CMD recvCmd ;

	if(*ppCmd == NULL)
	{
		*ppCmd = pReceiver->pRecvBuffer->pStart;//!!!voir
	}
	else
	{
		//point the next command
		(*ppCmd) += *( (uint32_t*) (*ppCmd + AR_CMD_INDEX_SIZE) );
	}
	
	recvCmd.pTabUint8 = *ppCmd;
	
	//check if the buffer stores enough data
	if(*ppCmd <= (uint8_t*) pReceiver->pRecvBuffer->pFront - AR_CMD_HEADER_SIZE)
	{	
		// pass the command to the host endian 
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

void returnASK(network_Receiver_t* pReceiver, int id, int seq)
{
	network_inOutBuffer_t* pBufferASK = inOutBufferWithId(	pReceiver->pptab_outputBuffer,
																pReceiver->numOfOutputBuff,
																idOutputToIdAck(id) );
	if(pBufferASK != NULL)
	{
		ringBuffPushBack(pBufferASK->pBuffer, &seq );
	}
}

int receiverRead(network_Receiver_t* pReceiver)
{
	int readDataSize =  sal_recv(	pReceiver->socket, pReceiver->pRecvBuffer->pStart,
								pReceiver->pRecvBuffer->buffSize, 0);

	pReceiver->pRecvBuffer->pFront =  pReceiver->pRecvBuffer->pStart + readDataSize;
	
	return readDataSize;
}

int receiverBind(network_Receiver_t* pReceiver, unsigned short port, int timeoutSec)
{
	struct timeval timeout;  
	
	SOCKADDR_IN recvSin;
	recvSin.sin_addr.s_addr = htonl(INADDR_ANY);   
	recvSin.sin_family = AF_INET;
	recvSin.sin_port = htons(port);
	
	pReceiver->socket = sal_socket(  AF_INET, SOCK_DGRAM,0);
	
	
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0; 
	setsockopt (pReceiver->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	
	return sal_bind(pReceiver->socket, (SOCKADDR*)&recvSin, sizeof(recvSin));
}
