/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>

#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>//voir ?
#include <libSAL/socket.h>
#include <libNetWork/common.h>
#include <libNetWork/buffer.h>
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/sender.h>

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
void senderSend(netWork_Sender_t* pSender);

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
int senderAddToBuffer(	netWork_Sender_t* pSender,const netWork_inOutBuffer_t* pinputBuff,
						int seqNum);
						
#define MILLISECOND 1000
#define SENDER_SLEEP_TIME_MS 1
#define INPUT_PARAM_NUM 7

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_Sender_t* newSender(	unsigned int sendingBufferSize, unsigned int inputBufferNum,
								netWork_inOutBuffer_t** ppTab_input)
{	
	netWork_Sender_t* pSender =  malloc( sizeof(netWork_Sender_t));
	
	int iiInputBuff = 0;
	int error=0;
	
	if(pSender)
	{
		pSender->isAlive = 1;
		pSender->sleepTime = MILLISECOND * SENDER_SLEEP_TIME_MS;

		pSender->inputBufferNum = inputBufferNum;

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

void deleteSender(netWork_Sender_t** ppSender)
{
	netWork_Sender_t* pSender = NULL;
	int iiInputBuff = 0;
	
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
	netWork_Sender_t* pSender = data;
	int seq = 1;
	int indexInput = 0;
	int callBackReturn = 0;
	
	netWork_inOutBuffer_t* pInputTemp;
	
	while( pSender->isAlive )
	{		
		usleep(pSender->sleepTime);
		
		for(indexInput = 0 ; indexInput < pSender->inputBufferNum ; ++indexInput  )
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

/*
void startSender(netWork_Sender_t* pSender)
{
	pSender->isAlive = 1;
}
*/

void stopSender(netWork_Sender_t* pSender)
{
	pSender->isAlive = 0;
}

void senderSend(netWork_Sender_t* pSender)
{	
	int nbCharCopy = 0;
	
	if( !bufferIsEmpty(pSender->pSendingBuffer) )
	{
		//bufferPrint(pSender->pSendingBuffer); //!!! debug
		
		nbCharCopy = pSender->pSendingBuffer->pFront - pSender->pSendingBuffer->pStart;
		
		sal_print(PRINT_WARNING," nbCharCopy :%d \n",nbCharCopy);
			
		sal_send(pSender->socket, pSender->pSendingBuffer->pStart, nbCharCopy, 0);
			
		pSender->pSendingBuffer->pFront = pSender->pSendingBuffer->pStart;
	}
}

int senderAddToBuffer(	netWork_Sender_t* pSender,const netWork_inOutBuffer_t* pinputBuff,
						int seqNum)
{
	int error = 1;
	int sizeNeed = AR_CMD_HEADER_SIZE + pinputBuff->pBuffer->buffCellSize;
	
	if( bufferGetFreeCellNb(pSender->pSendingBuffer) >= sizeNeed )
	{	
		//add type 
		memcpy( pSender->pSendingBuffer->pFront, &(pinputBuff->dataType), sizeof(int));
		pSender->pSendingBuffer->pFront +=  sizeof(int) ;
		
		//add id 
		memcpy( pSender->pSendingBuffer->pFront, &(pinputBuff->id), sizeof(int));
		pSender->pSendingBuffer->pFront +=  sizeof(int) ;
		
		//add seq 
		memcpy( pSender->pSendingBuffer->pFront, &(seqNum), sizeof(int));
		pSender->pSendingBuffer->pFront +=  sizeof(int) ;
		
		//add size 
		memcpy( pSender->pSendingBuffer->pFront, &(sizeNeed), sizeof(int));
		pSender->pSendingBuffer->pFront +=  sizeof(int) ;
		
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

void senderAckReceived(netWork_Sender_t* pSender, int id, int seqNum)
{
	netWork_inOutBuffer_t* pInputBuff = inOutBufferWithId( pSender->pptab_inputBuffer,
															pSender->inputBufferNum, id );
	if(pInputBuff != NULL)
	{
		inOutBufferAckReceived(pInputBuff, seqNum);
	}
}

int senderConnection(netWork_Sender_t* pSender,const char* addr, int port)
{
	SOCKADDR_IN sendSin;
	sendSin.sin_addr.s_addr = inet_addr(addr);
	sendSin.sin_family = AF_INET;
	sendSin.sin_port = htons(port);
	
	pSender->socket = sal_socket(  AF_INET, SOCK_DGRAM, 0);

	return sal_connect(pSender->socket, (SOCKADDR*)&sendSin, sizeof(sendSin));
}
