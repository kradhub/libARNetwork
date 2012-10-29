/**
 *	@file receiver.c
 *  @brief manage the data receiving
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#include <stdio.h>// !!!! sup
#include <sys/stat.h> //!!!sup
#include <sys/types.h> // !! sup
#include <fcntl.h> //!!!

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libNetWork/common.h>// !! modif
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/sender.h>
#include <libNetWork/receiver.h>

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
int getCmd(netWork_Receiver_t* pReceiver, uint8_t** ppCmd);


int initRecvBuffer(netWork_Receiver_t* pReceiver);

#define OUTPUT_PARAM_NUM 4
#define MICRO_SECOND 1000

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_Receiver_t* newReceiver(unsigned int recvBufferSize, unsigned int outputBufferNum, ...)
{
	sal_print(PRINT_WARNING,"newReceiver \n");//!! sup
	
	va_list ap;
	//int vaListSize = outputBufferNum * OUTPUT_PARAM_NUM;
	
	netWork_Receiver_t* pReceiver =  malloc( sizeof(netWork_Receiver_t));
	
	int outputNum = 2 ; // !!!!!sup calc with param
	int iiOutputBuff = 0;
	netWork_paramNewInOutBuffer_t paramNewOutputBuff;
	int error = 0;
	
	if(pReceiver)
	{
		pReceiver->isAlive = 1;
		pReceiver->sleepTime = 25 * MICRO_SECOND;
		
		pReceiver->pSender = NULL; // !!! 
		
		pReceiver->outputBufferNum = outputBufferNum;
		pReceiver->pptab_outputBuffer = malloc(sizeof(netWork_inOutBuffer_t) * outputBufferNum );
		
		if(pReceiver->pptab_outputBuffer)
		{
			va_start(ap, outputBufferNum );
			for(iiOutputBuff = outputBufferNum ; iiOutputBuff != 0 ; --iiOutputBuff) // pass it  !!!! ////
			{
				//get parameters //!!!!!!!!!!!!!!!!!!!!!!
				paramNewOutputBuff.id = va_arg(ap, int);
				paramNewOutputBuff.dataType = va_arg(ap, int); //paramNewOutputBuff.needAck = va_arg(ap, int);
				paramNewOutputBuff.buffSize = va_arg(ap, unsigned int);
				paramNewOutputBuff.buffCellSize = va_arg(ap, unsigned int);
				
				paramNewOutputBuff.sendingWaitTime = 0; //not used
				paramNewOutputBuff.ackTimeoutMs = 1;//not used
				paramNewOutputBuff.nbOfRetry = 1;//not used
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			
				pReceiver->pptab_outputBuffer[iiOutputBuff -1] = newInOutBuffer(&paramNewOutputBuff);
			}
			va_end(ap);
		}
		else
		{
			error = 1;
		}
		
		if(!error)
		{
			//pReceiver->recvBufferSize = recvBufferSize;
			pReceiver->pRecvBuffer = newBuffer(recvBufferSize,1);	//malloc( recvBufferSize ); //voir si c bon !!!!!!!!!
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

void deleteReceiver(netWork_Receiver_t** ppReceiver)
{
	netWork_Receiver_t* pReceiver = NULL;
	
	sal_print(PRINT_WARNING," deleteReceiver \n");//!! sup
	
	int iiOutputBuff = 0;
	
	if(ppReceiver)
	{
		pReceiver = *ppReceiver;
		
		if(pReceiver)
		{

			if(pReceiver->pptab_outputBuffer)
			{
				for(iiOutputBuff = pReceiver->outputBufferNum; iiOutputBuff != 0; --iiOutputBuff) // pass it  !!!! ////
				{
					deleteInOutBuffer( &(pReceiver->pptab_outputBuffer[iiOutputBuff - 1]) );
				}
				free(pReceiver->pptab_outputBuffer);
			}
			
			deleteBuffer( &(pReceiver->pRecvBuffer) ); //free(pReceiver->pRecvBuffer);
	
			free(pReceiver);
		}
		*ppReceiver = NULL;
	}
}

void* runReceivingThread(void* data)
{
	netWork_Receiver_t* pReceiver = data;
	
	UNION_CMD recvCmd;
	netWork_inOutBuffer_t* pOutBufferTemp = NULL;
	
	int pushError = 0;
	
	recvCmd.pTabUint8 = NULL;
	
	pReceiver->fd = open("wifi", O_RDONLY);
	
	while( pReceiver->isAlive )
	{
		//usleep(pReceiver->sleepTime);//sup ?
		
		//receiverRead( pReceiver, &(pReceiver->readDataSize) );
		//pReceiver->readDataSize = read(fd, pReceiver->pRecvBuffer, pReceiver->pRecvBuffer->buffSize);
		
		if( receiverRead( pReceiver ) > 0 /*pReceiver->readDataSize > 0*/)
		{
			sal_print(PRINT_WARNING,"--- read  Receiver ---\n");

			bufferPrint(pReceiver->pRecvBuffer);
			
			while( !getCmd( pReceiver, &(recvCmd.pTabUint8) ) )
			{
				sal_print(PRINT_WARNING," totoooooooooooooooo recvCmd.pTabUint8 %d \n", recvCmd.pTabUint8);
				sal_print(PRINT_WARNING," +++++++ ++ +recvCmd.pTabUint8[AR_CMD_INDEX_TYPE] type : %d \n",(int) recvCmd.pTabUint8[AR_CMD_INDEX_TYPE]);
				sal_print(PRINT_WARNING," +++++++ ++ +cmd type : %d \n",recvCmd.pCmd->type);
				
				switch (recvCmd.pCmd->type)
				{
					case CMD_TYPE_ACK:
						sal_print(PRINT_WARNING," CMD_TYPE_ACK \n");
						senderAckReceived(pReceiver->pSender,recvCmd.pCmd->id,recvCmd.pCmd->seq);
					break;
					
					case CMD_TYPE_DATA:
					
						sal_print(PRINT_WARNING," CMD_TYPE_DATA \n");
						
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->outputBufferNum,
															recvCmd.pCmd->id);
						if(pOutBufferTemp != NULL)
						{
							//sup pushError
							pushError=ringBuffPushBack(	pOutBufferTemp->pBuffer,
												( recvCmd.pTabUint8 + AR_CMD_INDEX_DATA ) );
												
							sal_print(PRINT_WARNING," pushError :%d \n",pushError);
						}							
					break;
					
					case CMD_TYPE_DATA_WITH_ACK:
					
						sal_print(PRINT_WARNING," CMD_TYPE_DATA_WITH_ACK \n");
					
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->outputBufferNum,
															recvCmd.pCmd->id);
						if(pOutBufferTemp != NULL)
						{
							pushError = ringBuffPushBack(	pOutBufferTemp->pBuffer,
															&(recvCmd.pTabUint8[AR_CMD_INDEX_DATA]));
							
							if( !pushError)
							{
								returnASK(pReceiver, recvCmd.pCmd->id, recvCmd.pCmd->seq);
							}
						}							
					break;
					
					default:
						sal_print(PRINT_WARNING," default \n");
					break;
				}
			}
		}
	}
     
    close(pReceiver->fd);
       
    return NULL;
}

void stopReceiver(netWork_Receiver_t* pReceiver)
{
	pReceiver->isAlive = 0;
}

int getCmd(netWork_Receiver_t* pReceiver, uint8_t** ppCmd)
{
	sal_print(PRINT_WARNING," getCmd \n");
	int error = 1;

	int cmdSize = 0 ;

	if(*ppCmd == NULL)
	{
		sal_print(PRINT_WARNING," = NULL \n");
		*ppCmd = pReceiver->pRecvBuffer->pStart;
	}
	else
	{
		sal_print(PRINT_WARNING," NEXT \n");
		//point the next command
		(*ppCmd) +=  (int) ( (*ppCmd)[AR_CMD_INDEX_SIZE] ) ;
	}
	
	//check if the buffer stores enough data
	if(*ppCmd <= pReceiver->pRecvBuffer->pFront - AR_CMD_HEADER_SIZE)
	{
		sal_print(PRINT_WARNING," head size ok \n");
		
		cmdSize = *( (int*) (*ppCmd + AR_CMD_INDEX_SIZE) ) ;
		
		if(*ppCmd <= pReceiver->pRecvBuffer->pFront - cmdSize)
		{
			sal_print(PRINT_WARNING," total size ok :%d  \n", cmdSize);
			sal_print(PRINT_WARNING," pCmd: %d \n", *ppCmd);
			sal_print(PRINT_WARNING,"pReceiver->pRecvBuffer->pFront : %d \n",pReceiver->pRecvBuffer->pFront);

			error = 0;
		}
	}
	
	if( error != 0)
	{
		sal_print(PRINT_WARNING," lastttttttttttttttttt cmd \n");
		*ppCmd = NULL;
	}
	
	return error;
}

void returnASK(netWork_Receiver_t* pReceiver, int id, int seq)
{
	netWork_inOutBuffer_t* pBufferASK = inOutBufferWithId(	pReceiver->pptab_outputBuffer,
																pReceiver->outputBufferNum, -id);

	if(pBufferASK != NULL)
	{
		ringBuffPushBack(pBufferASK->pBuffer, &seq);
	}
}

int receiverRead(netWork_Receiver_t* pReceiver)
{
	int readDataSize = read(pReceiver->fd, pReceiver->pRecvBuffer->pStart, pReceiver->pRecvBuffer->buffSize );
	
	//pReceiver->pRecvBuffer->pCursor =  pReceiver->pRecvBuffer->pStart;
	pReceiver->pRecvBuffer->pFront =  pReceiver->pRecvBuffer->pStart + readDataSize;
	
	return readDataSize;
}
