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
#include <libNetWork/common.h>// !! modif
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/sender.h>
#include <libNetWork/receiver.h>

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
int getCmd(netWork_Receiver_t* pReceiver, uint8_t** ppCmd);


int initRecvBuffer(netWork_Receiver_t* pReceiver);

#define OUTPUT_PARAM_NUM 4
#define MICRO_SECOND 1000

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_Receiver_t* newReceiver(	unsigned int recvBufferSize, unsigned int outputBufferNum,
									netWork_inOutBuffer_t** pptab_output)
{
	sal_print(PRINT_WARNING,"newReceiver \n");//!! sup
	
	
	netWork_Receiver_t* pReceiver =  malloc( sizeof(netWork_Receiver_t));

	int error = 0;
	
	if(pReceiver)
	{
		pReceiver->isAlive = 1;
		pReceiver->sleepTime = 25 * MICRO_SECOND;
		
		pReceiver->pSender = NULL; // !!!!!!
		
		pReceiver->outputBufferNum = outputBufferNum;
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
			deleteBuffer( &(pReceiver->pRecvBuffer) );
	
			free(pReceiver);
		}
		*ppReceiver = NULL;
	}
}

void* runReceivingThread(void* data)
{
	//sal_print(PRINT_WARNING,"- runReceivingThread -\n");
	
	netWork_Receiver_t* pReceiver = data;
	
	UNION_CMD recvCmd;
	netWork_inOutBuffer_t* pOutBufferTemp = NULL;
	
	int pushError = 0;
	
	recvCmd.pTabUint8 = NULL;
	
	while( pReceiver->isAlive )
	{
		//sal_print(PRINT_WARNING,"- read  tic  -\n");
		//usleep(pReceiver->sleepTime);//sup ?
		
		if( receiverRead( pReceiver ) > 0 /*pReceiver->readDataSize > 0*/)
		{
			sal_print(PRINT_WARNING,"--- read  Receiver: \n");
			bufferPrint(pReceiver->pRecvBuffer);
			
			while( !getCmd( pReceiver, &(recvCmd.pTabUint8) ) )
			{
				
				switch (recvCmd.pCmd->type)
				{
					case CMD_TYPE_ACK:
						sal_print(PRINT_WARNING," -CMD_TYPE_ACK \n");
						senderAckReceived(pReceiver->pSender,idAckToIdInput(recvCmd.pCmd->id),recvCmd.pCmd->seq);
					break;
					
					case CMD_TYPE_DATA:
					
						sal_print(PRINT_WARNING," -CMD_TYPE_DATA \n");
						
						pOutBufferTemp = inOutBufferWithId(	pReceiver->pptab_outputBuffer, 
															pReceiver->outputBufferNum,
															recvCmd.pCmd->id);
						
						if(pOutBufferTemp != NULL)
						{
							//sup pushError
							pushError = ringBuffPushBack(	pOutBufferTemp->pBuffer,
												( recvCmd.pTabUint8 + AR_CMD_INDEX_DATA ) );
												
							sal_print(PRINT_WARNING," pushError :%d \n",pushError);
							//ringPrint(pOutBufferTemp->pBuffer);
						}							
					break;
					
					case CMD_TYPE_DATA_WITH_ACK:
					
						sal_print(PRINT_WARNING," -CMD_TYPE_DATA_WITH_ACK \n");
					
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

    sal_close(pReceiver->socket);
       
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
		*ppCmd = pReceiver->pRecvBuffer->pStart;
	}
	else
	{
		//point the next command
		(*ppCmd) +=  (int) ( (*ppCmd)[AR_CMD_INDEX_SIZE] ) ;
	}
	
	//check if the buffer stores enough data
	if(*ppCmd <= (uint8_t*) pReceiver->pRecvBuffer->pFront - AR_CMD_HEADER_SIZE)
	{	
		cmdSize = *( (int*) (*ppCmd + AR_CMD_INDEX_SIZE) ) ;
		
		{
		if(*ppCmd <= (uint8_t*) pReceiver->pRecvBuffer->pFront - cmdSize)
			error = 0;
		}
	}
	
	if( error != 0)
	{
		*ppCmd = NULL;
	}
	
	return error;
}

void returnASK(netWork_Receiver_t* pReceiver, int id, int seq)
{
	netWork_inOutBuffer_t* pBufferASK = inOutBufferWithId(	pReceiver->pptab_outputBuffer,
																pReceiver->outputBufferNum,
																idOutputToIdAck(id) );

	if(pBufferASK != NULL)
	{
		ringBuffPushBack(pBufferASK->pBuffer, &seq ); ///!!!
	}
}

int receiverRead(netWork_Receiver_t* pReceiver)
{
	int readDataSize =  sal_recv(	pReceiver->socket, pReceiver->pRecvBuffer->pStart,
								pReceiver->pRecvBuffer->buffSize, 0);

	pReceiver->pRecvBuffer->pFront =  pReceiver->pRecvBuffer->pStart + readDataSize;
	
	return readDataSize;
}

int idOutputToIdAck( int id)
{
	return id + 1000; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

int idAckToIdInput( int id)
{
	return id - 1000; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

int receiverBind(netWork_Receiver_t* pReceiver, unsigned short port)
{
	SOCKADDR_IN recvSin;
	recvSin.sin_addr.s_addr = htonl(INADDR_ANY);   
	recvSin.sin_family = AF_INET;
	recvSin.sin_port = htons(port);
	
	pReceiver->socket = sal_socket(  AF_INET, SOCK_DGRAM,0);
	
	return sal_bind(pReceiver->socket, (SOCKADDR*)&recvSin, sizeof(recvSin));
}
