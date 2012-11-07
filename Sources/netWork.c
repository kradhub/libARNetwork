/**
 *	@file netWork.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>

#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libNetWork/common.h>//!! modif
#include <libNetWork/sender.h>
#include <libNetWork/receiver.h>
#include <libNetWork/netWork.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

netWork_t* newNetWork(	unsigned int recvBuffSize,unsigned int sendBuffSize,
						unsigned int numberOfOutput, unsigned int numberOfInput, ...)
{
	netWork_t* pNetWork = malloc( sizeof(netWork_t));
	
	int error = 0;
	
	va_list ap;
	
	int ii = 0;
	int indexAckOutput = 0;
	netWork_paramNewInOutBuffer_t paramNewInOutput;
	netWork_paramNewInOutBuffer_t paramNewACK;
	
    paramNewACK.dataType = CMD_TYPE_ACK;
    paramNewACK.buffSize = 1;
	paramNewACK.buffCellSize = sizeof(int);
	paramNewACK.overwriting = 0;
    paramNewACK.sendingWaitTime = 0; //not used
    paramNewACK.ackTimeoutMs = 0; //not used
    paramNewACK.nbOfRetry = 0 ; //not used
    
    
	if(pNetWork == NULL)
	{
		error = 1;
	}
	
	va_start(ap, numberOfInput );
	
	if( !error )
	{
		pNetWork->numOfOutput = 2 * numberOfOutput;
		pNetWork->ppTabOutput = malloc(sizeof(netWork_inOutBuffer_t*) * pNetWork->numOfOutput );
		
		pNetWork->numOfInput = numberOfInput + numberOfOutput;
		pNetWork->ppTabInput = malloc( sizeof(netWork_inOutBuffer_t*) * pNetWork->numOfInput );
		
		if( pNetWork->ppTabOutput && pNetWork->ppTabInput)
		{
			for(ii = 0; ii< numberOfOutput ; ++ii)
			{
				paramNewInOutput = va_arg(ap, netWork_paramNewInOutBuffer_t);
				pNetWork->ppTabOutput[ii] = newInOutBuffer(&paramNewInOutput);
				
				paramNewACK.id = idOutputToIdAck(paramNewInOutput.id); 
				
				indexAckOutput = numberOfOutput + ii;
				pNetWork->ppTabOutput[ indexAckOutput ] = newInOutBuffer(&paramNewACK);
				
				pNetWork->ppTabInput[numberOfInput + ii] = pNetWork->ppTabOutput[ indexAckOutput ];
			}
			
			for(ii = 0; ii< numberOfInput ; ++ii)
			{
				paramNewInOutput = va_arg(ap, netWork_paramNewInOutBuffer_t);
				pNetWork->ppTabInput[ii] = newInOutBuffer(&paramNewInOutput);
			}
		}
		else
		{
			error = 1;
		}
	}
	
	va_end(ap);
	
	if( !error )
	{		
		pNetWork->pSender = newSender(sendBuffSize, pNetWork->numOfInput, pNetWork->ppTabInput);
		pNetWork->pReceiver = newReceiver(recvBuffSize, pNetWork->numOfOutput, pNetWork->ppTabOutput);
		
		if( pNetWork->ppTabOutput && pNetWork->ppTabInput )
		{
			pNetWork->pReceiver->pSender = pNetWork->pSender;
		}
		else
		{
			error = 1;
		}
	}
    
    if(error)
    {
		deleteNetWork(&pNetWork);
	}
    
    return pNetWork;
}

void deleteNetWork(netWork_t** ppNetWork)
{	
	
	netWork_t* pNetWork = NULL;
	int ii = 0;
	
	if(ppNetWork)
	{
		pNetWork = *ppNetWork;
		
		if(pNetWork)
		{
			deleteSender( &(pNetWork->pSender) );
			deleteReceiver( &(pNetWork->pReceiver) );
			
			for(ii = 0; ii< pNetWork->numOfInput ; ++ii)
			{
				deleteInOutBuffer( &(pNetWork->ppTabInput[ii]) );
			}
			
			for(ii = 0; ii< pNetWork->numOfOutput ; ++ii)
			{
				deleteInOutBuffer( &(pNetWork->ppTabOutput[ii]) );
			}	
			
			free(pNetWork);
		}
		*ppNetWork = NULL;
	}
}


