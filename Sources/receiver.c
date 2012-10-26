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
 * 	@param pCmd the pointer on the command
 * 	@return error 1 if buffer is empty
 *	@pre only call by runSendingThread()
 * 	@see runSendingThread()
**/
int getCmd(netWork_Receiver_t* pReceiver, uint8_t* pCmd);

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
		
		pReceiver->readDataSize = 0;
		
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
			pReceiver->recvBufferSize = recvBufferSize;
			pReceiver->pRecvBuffer = malloc( recvBufferSize ); //voir si c bon !!!!!!!!!
			
			
			sal_print(PRINT_WARNING," new pReceiver->pRecvBuffer : %d \n", pReceiver->pRecvBuffer);//!! sup
			
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
			
			free(pReceiver->pRecvBuffer);
	
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
	
	int fd;
	fd = open("wifi", O_RDONLY);
	
	while( pReceiver->isAlive )
	{
		//usleep(pReceiver->sleepTime);//sup ?
		
		//receiverRead( pReceiver, &(pReceiver->readDataSize) );
		pReceiver->readDataSize = read(fd, pReceiver->pRecvBuffer, pReceiver->recvBufferSize);
		
		sal_print(PRINT_WARNING,"-waaaaaaaaaaaaaaaaaaaaa-\n");
		
		if( pReceiver->readDataSize > 0 /*pReceiver->pRecvBuffer != NULL*/)
		{
			sal_print(PRINT_WARNING,"--- read  Receiver ---\n");
			while( getCmd(pReceiver, recvCmd.pTabUint8) )
			{
				switch (recvCmd.pCmd->type)
				{
					case CMD_TYPE_ACK:
						senderAckReceived(pReceiver->pSender,recvCmd.pCmd->id,recvCmd.pCmd->seq);
					break;
					
					case CMD_TYPE_DATA:
					
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->outputBufferNum,
															recvCmd.pCmd->id);
						if(pOutBufferTemp != NULL)
						{
							ringBuffPushBack(pOutBufferTemp->pBuffer, recvCmd.pCmd->data);
						}							
					break;
					
					case CMD_TYPE_DATA_WITH_ACK:
					
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->outputBufferNum,
															recvCmd.pCmd->id);
						if(pOutBufferTemp != NULL)
						{
							pushError = ringBuffPushBack(pOutBufferTemp->pBuffer,recvCmd.pCmd->data);
							
							if( !pushError)
							{
								returnASK(pReceiver, recvCmd.pCmd->id, recvCmd.pCmd->seq);
							}
						}							
					break;
					
					default:
					
					break;
				}
			}
		}
	}
     
    close(fd);
       
    return NULL;
}

void stopReceiver(netWork_Receiver_t* pReceiver)
{
	pReceiver->isAlive = 0;
}

int getCmd(netWork_Receiver_t* pReceiver, uint8_t* pCmd)
{
	//!!!!!
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

void receiverRead(netWork_Receiver_t* pReceiver, int* readDataSize)
{
	int fd;
	*readDataSize = 0;
	
	fd = open("wifi", O_RDONLY);
	
	if (fd != 0)
	{
		//*readDataSize = fread( pReceiver->pRecvBuffer, 1,pReceiver->recvBufferSize,pFichier );
		*readDataSize = read(fd, pReceiver->pRecvBuffer, pReceiver->recvBufferSize);
		
		
		sal_print(PRINT_WARNING," readDataSize : %d \n", *readDataSize);
		
		close(fd);
	}
	else
	{
		// On affiche un message d'erreur si on veut
		sal_print(PRINT_WARNING," no wifiCopy.txt\n");
	}
}
