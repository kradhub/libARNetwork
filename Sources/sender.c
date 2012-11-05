/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>

//#include <stdarg.h>
#include <string.h>

/*
#include <sys/types.h>
#include <unistd.h> 
*/

/*
#include <stdio.h>// !!!! sup
#include <sys/stat.h> //!!!sup
#include <sys/types.h> // !! sup
#include <fcntl.h> //!!!
*/

#include <libSAL/print.h>
#include <libSAL/mutex.h>//voir ?
#include <libSAL/socket.h>
#include <libNetWork/common.h>// !! modif
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/singleBuffer.h>
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
#define SENDER_SLEEP_TIME_MS 25//1
#define INPUT_PARAM_NUM 7

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_Sender_t* newSender(	unsigned int sendingBufferSize, unsigned int inputBufferNum,
								netWork_inOutBuffer_t** ppTab_input)
{
	sal_print(PRINT_WARNING,"newSender \n");//!! sup
	
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
			sal_print(PRINT_WARNING,"deleteSender \n");//!! sup
			
			deleteBuffer( &(pSender->pSendingBuffer) );
		
			free(pSender);
		}
		*ppSender = NULL;
	}
}

void* runSendingThread(void* data)
{
	netWork_Sender_t* pSender = data;
	int seq = 0;
	int indexInput = 0;
	int callBackReturn = 0;
	
	netWork_inOutBuffer_t* pInputTemp;
	
	while( pSender->isAlive )
	{		
		//sal_print(PRINT_WARNING," senderTic \n");
		usleep(pSender->sleepTime);
		
		for(indexInput = 0 ; indexInput < pSender->inputBufferNum ; ++indexInput  )
		{
			pInputTemp = pSender->pptab_inputBuffer[indexInput];
			/*
			sal_print( PRINT_WARNING," Buff ID :%d \n",pInputTemp->id );
			sal_print( PRINT_WARNING," pInputTemp->waitTimeCount :%d \n",pInputTemp->waitTimeCount );
			sal_print( PRINT_WARNING," empty? :%d",ringBuffIsEmpty(pInputTemp->pBuffer));
			*/
			
			if(pInputTemp->waitTimeCount > 0)
			{
				--(pInputTemp->waitTimeCount);
			}

			if( inOutBuffeIsWaitAck(pInputTemp)  /*pInputTemp->isWaitAck*/) //mutex ?
			{
				
				/*sal_print(PRINT_WARNING,"  - WaitAck ackWaitTimeCount: %d pInputTemp->retryCount :%d \n",
											pInputTemp->ackWaitTimeCount , pInputTemp->retryCount); */
				if(pInputTemp->ackWaitTimeCount == 0)
				{
					if(pInputTemp->retryCount == 0)
					{
						//callBackReturn = pInputTemp->timeoutCallback();
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
				sal_print(PRINT_WARNING,"  - not Empty and wait count = 0 \n");
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
		
			
		nbCharCopy = pSender->pSendingBuffer->pFront - pSender->pSendingBuffer->pStart;
		
		sal_print(PRINT_WARNING," nbCharCopy :%d \n",nbCharCopy);
			
		sal_send(pSender->socket, pSender->pSendingBuffer->pStart, nbCharCopy, 0);
			
		pSender->pSendingBuffer->pFront = pSender->pSendingBuffer->pStart;
		
	}
}

int senderAddToBuffer(	netWork_Sender_t* pSender,const netWork_inOutBuffer_t* pinputBuff,
						int seqNum)
{
	sal_print(PRINT_WARNING," senderAddToBuffer \n");
	
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
		
		sal_print(PRINT_WARNING," AddToBuffer : \n");
		bufferPrint(pSender->pSendingBuffer);

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
	sendSin.sin_addr.s_addr = inet_addr(addr);//inet_addr("127.0.0.1");
	sendSin.sin_family = AF_INET /*AF_UNIX*/;
	sendSin.sin_port = htons(port);
	
	pSender->socket = sal_socket(  AF_INET /*AF_UNIX*/, SOCK_DGRAM /*SOCK_STREAM*/, 0);

	return sal_connect(pSender->socket, (SOCKADDR*)&sendSin, sizeof(sendSin));
}
