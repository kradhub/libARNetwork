/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>

#include <unistd.h>
#include <string.h>

#include <libSAL/print.h>
#include <libSAL/socket.h>
#include <libSAL/endianness.h>

#include <libNetwork/common.h>
#include <libNetwork/buffer.h>
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>

#include <arpa/inet.h> // !!!!!!!!!!!!!!!!!!!!!!!!!!!pass in libsal

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/*****************************************
 * 
 * 			private header:
 *
******************************************/
/**
 *  @brief send the data
 * 	@param pSender the pointer on the Sender
 *	@pre only call by runSendingThread()
 * 	@see runSendingThread()
**/
void senderSend(network_Sender_t* pSender);

/**
 *  @brief add data to the sender buffer
 * 	@param pSender the pointer on the Sender
 *	@param pData the pointer on the data
 * 
 * 
 * 
 * 
 *	@pre the thread calling runSendingThread() must be created
 * 	@see runSendingThread()
**/
int senderAddToBuffer(	network_Sender_t* pSender,const network_inOutBuffer_t* pinputBuff,
						int seqNum);
						
#define MILLISECOND 1000
#define SENDER_SLEEP_TIME_MS 1
#define INPUT_PARAM_NUM 7

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_Sender_t* newSender(	unsigned int sendingBufferSize, unsigned int numOfInputBuff,
								network_inOutBuffer_t** ppTab_input)
{	
	network_Sender_t* pSender =  malloc( sizeof(network_Sender_t));
	
	int error=0;
	
	if(pSender)
	{
		pSender->isAlive = 1;
		//pSender->sleepTime = MILLISECOND * SENDER_SLEEP_TIME_MS;

		pSender->numOfInputBuff = numOfInputBuff;

		pSender->pptab_inputBuffer = ppTab_input;
		
		if(!error)
		{
			pSender->pSendingBuffer = newBuffer(sendingBufferSize, 1);
			
			if(pSender->pSendingBuffer == NULL)
			{
				error = 1;
			}
		}
		
		if(error)
		{
			deleteSender(&pSender);
		}
	}
	
	return pSender;
}

void deleteSender(network_Sender_t** ppSender)
{
	network_Sender_t* pSender = NULL;
	
	if(ppSender)
	{
		pSender = *ppSender;
		
		if(pSender)
		{
			deleteBuffer( &(pSender->pSendingBuffer) );
		
			free(pSender);
		}
		*ppSender = NULL;
	}
}

void* runSendingThread(void* data)
{
	network_Sender_t* pSender = data;
	int seq = 1;
	int indexInput = 0;
	int callBackReturn = 0;
	
	network_inOutBuffer_t* pInputTemp;
	
	while( pSender->isAlive )
	{		
		usleep(MILLISECOND /*pSender->sleepTime*/);
		
		for(indexInput = 0 ; indexInput < pSender->numOfInputBuff ; ++indexInput  )
		{
			pInputTemp = pSender->pptab_inputBuffer[indexInput];
			
			if(pInputTemp->waitTimeCount > 0)
			{
				--(pInputTemp->waitTimeCount);
			}

			if( inOutBuffeIsWaitAck(pInputTemp)  ) 
			{
				if(pInputTemp->ackWaitTimeCount == 0)
				{
					if(pInputTemp->retryCount == 0)
					{
						//callBackReturn = pInputTemp->timeoutCallback();
						sal_print(PRINT_WARNING," !!! too retry !!! \n");
						
						if(callBackReturn)
						{
							pInputTemp->retryCount = pInputTemp->nbOfRetry;
						}
					}
					else
					{
						senderAddToBuffer(pSender, pInputTemp, pInputTemp->seqWaitAck);
						pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
					}
					
					if(pInputTemp->retryCount > 0 )
					{
						--(pInputTemp->retryCount);
					}
				}
				
				if(pInputTemp->ackWaitTimeCount > 0 )
				{
					--(pInputTemp->ackWaitTimeCount);
				}
			}
			else if( !ringBuffIsEmpty(pInputTemp->pBuffer) && !pInputTemp->waitTimeCount)
			{
				if( !senderAddToBuffer(pSender, pInputTemp, seq) )
				{
					pInputTemp->waitTimeCount = pInputTemp->sendingWaitTime;
					
					switch(pInputTemp->dataType)
					{
						case CMD_TYPE_DATA_WITH_ACK:
							pInputTemp->isWaitAck = 1;
							pInputTemp->seqWaitAck = seq;
							pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
							pInputTemp->retryCount = pInputTemp->nbOfRetry;	
						break;
						
						case CMD_TYPE_DATA:
							ringBuffPopFront(pInputTemp->pBuffer, NULL);
						break;
						
						case CMD_TYPE_ACK:
							ringBuffPopFront(pInputTemp->pBuffer, NULL);
						break;
						
						case CMD_TYPE_KEEP_ALIVE:
						
						break;
						
						default:

						break;
					}
					
					++seq;
				}
			}
		}
		senderSend(pSender);
	}

	sal_close(pSender->socket);
        
    return NULL;
}

void stopSender(network_Sender_t* pSender)
{
	pSender->isAlive = 0;
}

void senderSend(network_Sender_t* pSender)
{	
	int nbCharCopy = 0;
	
	if( !bufferIsEmpty(pSender->pSendingBuffer) )
	{
		//bufferPrint(pSender->pSendingBuffer); //!!! debug
		
		nbCharCopy = pSender->pSendingBuffer->pFront - pSender->pSendingBuffer->pStart;
			
		sal_send(pSender->socket, pSender->pSendingBuffer->pStart, nbCharCopy, 0);
			
		pSender->pSendingBuffer->pFront = pSender->pSendingBuffer->pStart;
	}
}

int senderAddToBuffer(	network_Sender_t* pSender,const network_inOutBuffer_t* pinputBuff,
						int seqNum)
{
	int error = 1;
	int sizeNeed = AR_CMD_HEADER_SIZE + pinputBuff->pBuffer->buffCellSize;
	uint32_t droneEndianInt32 = 0;
	
	if( bufferGetFreeCellNb(pSender->pSendingBuffer) >= sizeNeed )
	{	
		//add type
		droneEndianInt32 =  htodl( (uint32_t) pinputBuff->dataType );
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		//add id 
		droneEndianInt32 =  htodl(pinputBuff->id);
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		//add seq 
		droneEndianInt32 =  htodl(seqNum);
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		//add size
		droneEndianInt32 =  htodl(sizeNeed); 
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		//add data						
		error = ringBuffFront(pinputBuff->pBuffer, pSender->pSendingBuffer->pFront);
		
		if(!error)
		{
			pSender->pSendingBuffer->pFront += pinputBuff->pBuffer->buffCellSize;
		}
		else
		{
			pSender->pSendingBuffer->pFront -= AR_CMD_HEADER_SIZE;
		}
		
	}
	
	return error;
}

int senderAckReceived(network_Sender_t* pSender, int id, int seqNum)
{
	int error = 1;
	network_inOutBuffer_t* pInputBuff = inOutBufferWithId( pSender->pptab_inputBuffer,
															pSender->numOfInputBuff, id );
	if(pInputBuff != NULL)
	{
		error = inOutBufferAckReceived(pInputBuff, seqNum);
	}
	
	return error;
}

int senderConnection(network_Sender_t* pSender,const char* addr, int port)
{
	SOCKADDR_IN sendSin;
	sendSin.sin_addr.s_addr = inet_addr(addr);
	sendSin.sin_family = AF_INET;
	sendSin.sin_port = htons(port);
	
	pSender->socket = sal_socket(  AF_INET, SOCK_DGRAM, 0);

	return sal_connect(pSender->socket, (SOCKADDR*)&sendSin, sizeof(sendSin));
}
