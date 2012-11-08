/**
 *	@file network.c
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
#include <libNetwork/common.h>//!! modif
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/network.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

network_t* newNetwork(	unsigned int recvBuffSize,unsigned int sendBuffSize,
						unsigned int numberOfOutput, unsigned int numberOfInput, ...)
{
	network_t* pNetwork = malloc( sizeof(network_t));
	
	int error = 0;
	
	va_list ap;
	
	int ii = 0;
	int indexAckOutput = 0;
	network_paramNewInOutBuffer_t paramNewInOutput;
	network_paramNewInOutBuffer_t paramNewACK;
	
    paramNewACK.dataType = CMD_TYPE_ACK;
    paramNewACK.buffSize = 1;
	paramNewACK.buffCellSize = sizeof(int);
	paramNewACK.overwriting = 0;
    paramNewACK.sendingWaitTime = 0; //not used
    paramNewACK.ackTimeoutMs = 0; //not used
    paramNewACK.nbOfRetry = 0 ; //not used
    
    
	if(pNetwork == NULL)
	{
		error = 1;
	}
	
	va_start(ap, numberOfInput );
	
	if( !error )
	{
		pNetwork->numOfOutput = 2 * numberOfOutput;
		pNetwork->ppTabOutput = malloc(sizeof(network_inOutBuffer_t*) * pNetwork->numOfOutput );
		
		pNetwork->numOfInput = numberOfInput + numberOfOutput;
		pNetwork->ppTabInput = malloc( sizeof(network_inOutBuffer_t*) * pNetwork->numOfInput );
		
		if( pNetwork->ppTabOutput && pNetwork->ppTabInput)
		{
			for(ii = 0; ii< numberOfOutput ; ++ii)
			{
				paramNewInOutput = va_arg(ap, network_paramNewInOutBuffer_t);
				pNetwork->ppTabOutput[ii] = newInOutBuffer(&paramNewInOutput);
				
				paramNewACK.id = idOutputToIdAck(paramNewInOutput.id); 
				
				indexAckOutput = numberOfOutput + ii;
				pNetwork->ppTabOutput[ indexAckOutput ] = newInOutBuffer(&paramNewACK);
				
				pNetwork->ppTabInput[numberOfInput + ii] = pNetwork->ppTabOutput[ indexAckOutput ];
			}
			
			for(ii = 0; ii< numberOfInput ; ++ii)
			{
				paramNewInOutput = va_arg(ap, network_paramNewInOutBuffer_t);
				pNetwork->ppTabInput[ii] = newInOutBuffer(&paramNewInOutput);
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
		pNetwork->pSender = newSender(sendBuffSize, pNetwork->numOfInput, pNetwork->ppTabInput);
		pNetwork->pReceiver = newReceiver(recvBuffSize, pNetwork->numOfOutput, pNetwork->ppTabOutput);
		
		if( pNetwork->ppTabOutput && pNetwork->ppTabInput )
		{
			pNetwork->pReceiver->pSender = pNetwork->pSender;
		}
		else
		{
			error = 1;
		}
	}
    
    if(error)
    {
		deleteNetwork(&pNetwork);
	}
    
    return pNetwork;
}

void deleteNetwork(network_t** ppNetwork)
{	
	
	network_t* pNetwork = NULL;
	int ii = 0;
	
	if(ppNetwork)
	{
		pNetwork = *ppNetwork;
		
		if(pNetwork)
		{
			deleteSender( &(pNetwork->pSender) );
			deleteReceiver( &(pNetwork->pReceiver) );
			
			for(ii = 0; ii< pNetwork->numOfInput ; ++ii)
			{
				deleteInOutBuffer( &(pNetwork->ppTabInput[ii]) );
			}
			
			for(ii = 0; ii< pNetwork->numOfOutput ; ++ii)
			{
				deleteInOutBuffer( &(pNetwork->ppTabOutput[ii]) );
			}	
			
			free(pNetwork);
		}
		*ppNetwork = NULL;
	}
}


