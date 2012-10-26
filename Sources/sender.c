/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <stdio.h>// !!!! sup

#include <libSAL/print.h>
#include <libSAL/mutex.h>//voir ?
#include <libSAL/socket.h>
#include <libNetWork/common.h>// !! modif
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/singleBuffer.h>
#include <libNetWork/sender.h>

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

#define MICRO_SECOND 1000
#define SENDER_SLEEP_TIME_MS 25//1
#define INPUT_PARAM_NUM 7
#define HEAD_SIZE 16

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_Sender_t* newSender(unsigned int sendingBufferSize, unsigned int inputBufferNum, ...)
{
	sal_print(PRINT_WARNING,"newSender \n");//!! sup
	
	va_list ap;
	int vaListSize = inputBufferNum * INPUT_PARAM_NUM;
	
	netWork_Sender_t* pSender =  malloc( sizeof(netWork_Sender_t));
	
	int iiInputBuff = 0;
	netWork_paramNewInOutBuffer_t paramNewInputBuff;
	int error=0;
	
	if(pSender)
	{
		pSender->isAlive = 1;
		pSender->sleepTime = MICRO_SECOND * SENDER_SLEEP_TIME_MS;

		pSender->inputBufferNum = inputBufferNum;

		pSender->pptab_inputBuffer = malloc(sizeof(netWork_inOutBuffer_t) * inputBufferNum );
		
		if(pSender->pptab_inputBuffer)
		{
			va_start( ap, vaListSize );
			for(iiInputBuff = 0 ; iiInputBuff < inputBufferNum ; ++iiInputBuff) // pass it  !!!! ////
			{
				sal_print(PRINT_WARNING," iiInputBuff:%d \n",iiInputBuff);
				//get parameters //!!!!!!!!!!!!!!!!!!!!!!
				paramNewInputBuff.id = va_arg(ap, int);
				paramNewInputBuff.dataType = va_arg(ap, int); //paramNewInputBuff.needAck = va_arg(ap, int);
				paramNewInputBuff.sendingWaitTime = va_arg(ap, int);
				paramNewInputBuff.ackTimeoutMs = va_arg(ap, int);
				paramNewInputBuff.nbOfRetry = va_arg(ap, int);
				
				paramNewInputBuff.buffSize = va_arg(ap, unsigned int);
				paramNewInputBuff.buffCellSize = va_arg(ap, unsigned int);
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			
				pSender->pptab_inputBuffer[iiInputBuff] = newInOutBuffer(&paramNewInputBuff);
				
				if(pSender->pptab_inputBuffer[iiInputBuff] == NULL)
				{
					error = 1;
				}
			}
			va_end(ap);
		}
		else
		{
			error = 1;
		}
		
		if(!error)
		{
			//pSender->sendingBufferSize = sendingBufferSize;
			//pSender->pSendingBuffer = malloc( pSender->sendingBufferSize );
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
			
			if(pSender->pptab_inputBuffer)
			{
				for(iiInputBuff = 0 ; iiInputBuff < pSender->inputBufferNum ; ++iiInputBuff) // pass it  !!!! ////
				{
					deleteInOutBuffer( &(pSender->pptab_inputBuffer[ iiInputBuff ]) );
				}
				free(pSender->pptab_inputBuffer);
			}
			
			//free(pSender->pSendingBuffer);
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
		sal_print(PRINT_WARNING," senderTic \n");
		usleep(pSender->sleepTime);
		
		for(indexInput = 0 ; indexInput < pSender->inputBufferNum ; ++indexInput  )
		{
			pInputTemp = pSender->pptab_inputBuffer[indexInput];
			sal_print( PRINT_WARNING," Buff ID :%d \n",pInputTemp->id );
			
			if(pInputTemp->waitTimeCount > 0)
			{
				--(pInputTemp->waitTimeCount);
			}
			

			if(pInputTemp->isWaitAck) //mutex ?
			{
				sal_print(PRINT_WARNING,"  - WaitAck \n");
				--(pInputTemp->ackWaitTimeCount);
				if(pInputTemp->ackWaitTimeCount == 0)
				{
					--(pInputTemp->retryCount);
					if(pInputTemp->retryCount == 0)
					{
						//callBackReturn = pInputTemp->timeoutCallback();
						if(callBackReturn)
						{
							pInputTemp->retryCount = pInputTemp->nbOfRetry;
						}
					}
					
					if(pInputTemp->retryCount != 0)
					{
						senderAddToBuffer(pSender, pInputTemp, pInputTemp->seqWaitAck);
						pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
					}
				}
			}
			else if( !ringBuffIsEmpty(pInputTemp->pBuffer) && !pInputTemp->waitTimeCount)
			{
				sal_print(PRINT_WARNING,"  - not Empty and wait count = 0 \n");
				if( !senderAddToBuffer(pSender, pInputTemp, seq) )
				{
					sal_print(PRINT_WARNING,"  copy ok \n");
					pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
					
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
						
						default:

						break;
					}
					
					++seq;
				}
			}
		}
		senderSend(pSender);
	}
        
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
	// !!! temp
	
	sal_print(PRINT_WARNING," senderSend \n");
	
	int nbCharCopy = 0;
	
	FILE* pFichier = NULL; //sup
	
	if( !bufferIsEmpty(pSender->pSendingBuffer) )
	{
	
		pFichier = fopen("wifi.txt", "rb+");
		if (pFichier != NULL)
		{
			fseek(pFichier, 0, SEEK_END);
			
			nbCharCopy = pSender->pSendingBuffer->pFront - pSender->pSendingBuffer->pBack;

			fwrite(pSender->pSendingBuffer->pBack,nbCharCopy,1,pFichier);
			
			pSender->pSendingBuffer->pFront = pSender->pSendingBuffer->pBack;
			
			fclose(pFichier);
		}
		else
		{
			// On affiche un message d'erreur si on veut
			sal_print(PRINT_WARNING," no wifi.txt \n");
		}
	}
}

int senderAddToBuffer(	netWork_Sender_t* pSender,const netWork_inOutBuffer_t* pinputBuff,
						int seqNum)
{
	sal_print(PRINT_WARNING," senderAddToBuffer \n");
	
	char* tabC = 0;//sup
	
	int error = 1;
	int sizeNeed = HEAD_SIZE + pinputBuff->pBuffer->buffCellSize;
	
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
		
		tabC = pSender->pSendingBuffer->pFront;
		sal_print(PRINT_WARNING,"last char %c \n",*tabC);
		
		if(!error)
		{
			pSender->pSendingBuffer->pFront += pinputBuff->pBuffer->buffCellSize;
		}
	}
	
	return error;
}

void senderAckReceived(netWork_Sender_t* pSender, int id, int seqNum)
{
	sal_print(PRINT_WARNING," senderTransmitAck \n");
	netWork_inOutBuffer_t* pInputBuff = inOutBufferWithId( pSender->pptab_inputBuffer,
															pSender->inputBufferNum, id );
	if(pInputBuff != NULL)
	{
		sal_print(PRINT_WARNING," E \n");
		inOutBufferAckReceived(pInputBuff, seqNum);
		sal_print(PRINT_WARNING," F \n");
		ringBuffPopFront(pInputBuff->pBuffer, NULL);// !! pass in inOutBufferAckReceived ???
		sal_print(PRINT_WARNING," G \n");
	}
}