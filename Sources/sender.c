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

#include <libSAL/print.h>
#include <libSAL/mutex.h>//voir ?
#include <libSAL/socket.h>
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/singleBuffer.h>
#include <libNetWork/common.h>// !! modif
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


#define SENDER_SLEEP_TIME_MS 500000//1000
#define INPUT_PARAM_NUM 5

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
		pSender->sleepTime = SENDER_SLEEP_TIME_MS;

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
				paramNewInputBuff.needAck = va_arg(ap, int);
				paramNewInputBuff.sendingWaitTime = va_arg(ap, int);
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
	
	netWork_inOutBuffer_t* pInputTemp;
	
	while( pSender->isAlive )
	{		
		usleep(pSender->sleepTime);
		
		for(indexInput = 0 ; indexInput < pSender->inputBufferNum ; ++indexInput  )
		{
			sal_print(PRINT_WARNING," Buff ID :%d \n",pSender->pptab_inputBuffer[indexInput]->id);
			pInputTemp = pSender->pptab_inputBuffer[indexInput];
			
			sal_print(PRINT_WARNING," A empty: %d \n", ringBuffIsEmpty(pInputTemp->pBuffer));
			
			if(	!ringBuffIsEmpty(pInputTemp->pBuffer) &&
				!pInputTemp->isWaitAck && 					// !!! mutex ???? 
				!pInputTemp->waitTimeCount)
			{
				
				/// voir pour simplifier ????
				
				
				if(	!senderAddToBuffer(pSender, pInputTemp, seq) )
				{
					sal_print(PRINT_WARNING," B \n");
					
					if(pInputTemp->needAck)
					{
						sal_print(PRINT_WARNING," C NEED ACK \n");
						
						pInputTemp->isWaitAck = 1;
						pInputTemp->seqWaitAck = seq;
					}
					else
					{
						sal_print(PRINT_WARNING," D \n");
						ringBuffPopFront(pInputTemp->pBuffer, NULL);
					}
					
					++seq;
				}
			}
			else if(pInputTemp->waitTimeCount)
			{
				--(pInputTemp->waitTimeCount);
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
	
}

int senderAddToBuffer(	netWork_Sender_t* pSender,const netWork_inOutBuffer_t* pinputBuff,
						int seqNum)
{
	sal_print(PRINT_WARNING," senderAddToBuffer \n");
	
	int error = 1;
	if( bufferGetFreeCellNb(pSender->pSendingBuffer) >= pinputBuff->pBuffer->buffCellSize )
	{	
		sal_print(PRINT_WARNING," bufferGetFreeCellNb(pSender->pSendingBuffer) :%d | pinputBuff->pBuffer->buffCellSize : %d \n" ,
									bufferGetFreeCellNb(pSender->pSendingBuffer),  pinputBuff->pBuffer->buffCellSize);
		error = ringBuffFront(pinputBuff->pBuffer, pSender->pSendingBuffer);
		
		if(!error)
		{
			pSender->pSendingBuffer->pFront += pinputBuff->pBuffer->buffCellSize;
		}
	}
	
	return error;
}

void senderTransmitAck(netWork_Sender_t* pSender, int id, int seqNum)
{
	sal_print(PRINT_WARNING," senderTransmitAck \n");
	netWork_inOutBuffer_t* pInputBuff = inOutBufferWithId( pSender->pptab_inputBuffer,
															pSender->inputBufferNum, id );
	if(pInputBuff != NULL)
	{
		sal_print(PRINT_WARNING," E \n");
		inOutBufferTransmitAck(pInputBuff, seqNum);
		sal_print(PRINT_WARNING," F \n");
		ringBuffPopFront(pInputBuff->pBuffer, NULL);// !! pass in inOutBufferTransmitAck ???
		sal_print(PRINT_WARNING," G \n");
	}
}
