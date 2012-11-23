/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

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
 *	@note only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
void senderSend(network_sender_t* pSender);

/**
 *  @brief add data to the sender buffer
 * 	@param pSender the pointer on the Sender
 *	@param pinputBuff
 * 	@param seqNum
 * 	@note only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
int senderAddToBuffer(	network_sender_t* pSender,const network_ioBuffer_t* pinputBuff,
						int seqNum);

/*****************************************
 * 
 * 			define :
 *
******************************************/
					
#define MILLISECOND 1000

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_sender_t* NETWORK_NewSender(	unsigned int sendingBufferSize, unsigned int numOfInputBuff,
								network_ioBuffer_t** ppTab_input)
{	
	/** -- Create a new sender -- */
	
	/** local declarations */
	network_sender_t* pSender =  NULL;
	int error=0;
	
	/** Create the sender */
	pSender =  malloc( sizeof(network_sender_t));
	
	if(pSender)
	{
		pSender->isAlive = 1;
		pSender->numOfInputBuff = numOfInputBuff;
		pSender->pptab_inputBuffer = ppTab_input;
		
		/** Create the Sending buffer */
		pSender->pSendingBuffer = newBuffer(sendingBufferSize, 1);
			
		if(pSender->pSendingBuffer == NULL)
		{
			error = 1;
		}
		
		/** delete the sender if an error occurred */
		if(error)
		{
			NETWORK_DeleteSender(&pSender);
		}
	}
	
	return pSender;
}

void NETWORK_DeleteSender(network_sender_t** ppSender)
{
	/** -- Delete the sender -- */
	
	/** local declarations */
	network_sender_t* pSender = NULL;
	
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

void* NETWORK_RunSendingThread(void* data)
{
	/** -- Manage the sending of the data on the sender' socket -- */
	
	/** local declarations */
	network_sender_t* pSender = data;
	int seq = 1;
	int indexInput = 0;
	int callBackReturn = 0;
	network_ioBuffer_t* pInputTemp = NULL;
	
	
	while( pSender->isAlive )
	{		
		usleep(MILLISECOND);
		
		/** for each input buffer try to send the data if necessary */
		for(indexInput = 0 ; indexInput < pSender->numOfInputBuff ; ++indexInput  )
		{
			pInputTemp = pSender->pptab_inputBuffer[indexInput];
			
			/** decrement the time to wait */
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
						/** if there are timeout and too sending retry ... */
						
						//callBackReturn = pInputTemp->timeoutCallback();
						sal_print(PRINT_WARNING," !!! too retry !!! \n");
						
						if(callBackReturn)
						{
							pInputTemp->retryCount = pInputTemp->nbOfRetry;
						}
					}
					else
					{
						/**
						 *  if there is a timeout, retry to send the data 
						 *  and decrement the number of retry still possible
						**/
						senderAddToBuffer(pSender, pInputTemp, pInputTemp->seqWaitAck);
						pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
						
						if(pInputTemp->retryCount > 0 )
						{
							--(pInputTemp->retryCount);
						}
					}
				}
				
				/** decrement the time to wait before considering as a timeout */
				if(pInputTemp->ackWaitTimeCount > 0 )
				{
					--(pInputTemp->ackWaitTimeCount);
				}
			}
			else if( !ringBuffIsEmpty(pInputTemp->pBuffer) && !pInputTemp->waitTimeCount)
			{
				/** try to add the latest data of the input buffer in the sending buffer*/
				if( !senderAddToBuffer(pSender, pInputTemp, seq) )
				{				
					pInputTemp->waitTimeCount = pInputTemp->sendingWaitTime;
					
					switch(pInputTemp->dataType)
					{
						case CMD_TYPE_DATA_WITH_ACK:
							/** 
							 * reinitialize the input buffer parameters,
							 * save the sequence wait for the acknowledgement,
							 * and pass on waiting acknowledgement.
							**/
							pInputTemp->isWaitAck = 1;
							pInputTemp->seqWaitAck = seq;
							pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
							pInputTemp->retryCount = pInputTemp->nbOfRetry;	
						break;
						
						case CMD_TYPE_DATA:
							/** pop the data sent*/
							ringBuffPopFront(pInputTemp->pBuffer, NULL);
						break;
						
						case CMD_TYPE_ACK:
							/** pop the acknowledgement sent*/
							ringBuffPopFront(pInputTemp->pBuffer, NULL);
						break;
						
						case CMD_TYPE_KEEP_ALIVE:
						
						break;
						
						default:

						break;
					}
					
					/** increment the sequence number */
					++seq;
				}
			}
		}
		senderSend(pSender);
	}

	sal_close(pSender->socket);
        
    return NULL;
}

void NETWORK_StopSender(network_sender_t* pSender)
{
	/** -- Stop the sending -- */
	pSender->isAlive = 0;
}

int NETWORK_SenderAckReceived(network_sender_t* pSender, int id, int seqNum)
{
	/** -- Receive an acknowledgment fo a data -- */
	
	/** local declarations */
	network_ioBuffer_t* pInputBuff = NULL;
	int error = 1;
	
	pInputBuff = inOutBufferWithId( pSender->pptab_inputBuffer, pSender->numOfInputBuff, id );
	
	if(pInputBuff != NULL)
	{
		/**
		 *  transmit the acknowledgment to the input buffer. 
		 * 	if the acknowledgment is suitable the waiting data is popped
		**/
		error = inOutBufferAckReceived( pInputBuff, seqNum );
	}
	
	return error;
}

int NETWORK_SenderConnection(network_sender_t* pSender,const char* addr, int port)
{
	/** -- Connect the socket in UDP to a port of an address -- */
	
	/** local declarations */
	SOCKADDR_IN sendSin;
	
	sendSin.sin_addr.s_addr = inet_addr(addr);
	sendSin.sin_family = AF_INET;
	sendSin.sin_port = htons(port);
	
	pSender->socket = sal_socket(  AF_INET, SOCK_DGRAM, 0);

	return sal_connect( pSender->socket, (SOCKADDR*)&sendSin, sizeof(sendSin) );
}

/*****************************************
 * 
 * 			private implementation:
 *
******************************************/

void senderSend(network_sender_t* pSender)
{	
	/**  -- send the data -- */
	
	/** local declarations */
	int nbCharCopy = 0;
	
	if( !bufferIsEmpty(pSender->pSendingBuffer) )
	{	
		nbCharCopy = pSender->pSendingBuffer->pFront - pSender->pSendingBuffer->pStart;
			
		sal_send(pSender->socket, pSender->pSendingBuffer->pStart, nbCharCopy, 0);
			
		pSender->pSendingBuffer->pFront = pSender->pSendingBuffer->pStart;
	}
}

int senderAddToBuffer( network_sender_t* pSender,const network_ioBuffer_t* pinputBuff,
						int seqNum)
{
	/** -- add data to the sender buffer -- */
	
	/** local declarations */
	int error = 1;
	int sizeNeed = AR_CMD_HEADER_SIZE + pinputBuff->pBuffer->buffCellSize;
	uint32_t droneEndianInt32 = 0;
	
	if( bufferGetFreeCellNb(pSender->pSendingBuffer) >= sizeNeed )
	{	
		/** add type */
		droneEndianInt32 =  htodl( (uint32_t) pinputBuff->dataType );
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add id */
		droneEndianInt32 =  htodl(pinputBuff->id);
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add seq */
		droneEndianInt32 =  htodl(seqNum);
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add size */
		droneEndianInt32 =  htodl(sizeNeed); 
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add data */						
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

