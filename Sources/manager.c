/**
 *	@file manager.c
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>

#include <inttypes.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>

#include <libNetwork/common.h>
#include <libNetwork/ringBuffer.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/manager.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

network_manager_t* NETWORK_NewManager(	unsigned int recvBuffSize,unsigned int sendBuffSize,
				unsigned int numberOfInput, network_paramNewInOutBuffer_t* ptabParamInput,
				unsigned int numberOfOutput, network_paramNewInOutBuffer_t* ptabParamOutput)
{
	/** -- Create a new Network -- */
	
	/** local declarations */
	network_manager_t* pNetwork = NULL;
	int error = NETWORK_MANAGER_OK;
	
	int ii = 0;
	int indexAckOutput = 0;
	network_paramNewInOutBuffer_t paramNewACK;
	
	/** Initialize the default parameters for the buffers of acknowledgement. */
	// call paramDefault !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    paramNewACK.dataType = CMD_TYPE_ACK;
    paramNewACK.buffSize = 1;
	paramNewACK.buffCellSize = sizeof(int);
	paramNewACK.overwriting = 0;
    paramNewACK.sendingWaitTime = 0; /* not used */
    paramNewACK.ackTimeoutMs = 0; /* not used */
    paramNewACK.nbOfRetry = 0 ; /* not used */
    
    /** Create the Network */
    pNetwork = malloc( sizeof(network_manager_t));
    
	if(pNetwork == NULL)
	{
		error = 1;
	}
	
	if( !error )
	{
		/**
		 *  For each output buffer a buffer of acknowledgement is add and referenced
		 *  in the output buffer list and the input buffer list.
		**/
		
		/** 
		 * Allocate the output buffer list of size of the number of output plus the number of 
		 * buffer of acknowledgement. The size of the list is then two times the number of output.
		**/
		pNetwork->numOfOutputWithoutAck = numberOfOutput;
		pNetwork->numOfOutput = 2 * numberOfOutput;
		pNetwork->ppTabOutput = malloc(sizeof(network_ioBuffer_t*) * pNetwork->numOfOutput );
		
		/** 
		 * Allocate the input buffer list of size of the number of input plus the number of 
		 * buffer of acknowledgement. 
		 * The size of the list is then number of input plus the number of output.
		**/ 
		pNetwork->numOfInputWithoutAck = numberOfInput;
		pNetwork->numOfInput = numberOfInput + numberOfOutput;
		pNetwork->ppTabInput = malloc( sizeof(network_ioBuffer_t*) * pNetwork->numOfInput );
		
		
		if( pNetwork->ppTabOutput && pNetwork->ppTabInput)
		{
			/** Create the output buffers and the buffers of acknowledgement */
			for(ii = 0; ii < numberOfOutput; ++ii)
			{
				pNetwork->ppTabOutput[ii] = newInOutBuffer( &(ptabParamOutput[ii]) );
				
				/** 
				 * Create the buffer of acknowledgement associated with the output buffer and 
				 * store it at the end of the output and input buffer lists.
				**/
				paramNewACK.id = idOutputToIdAck(ptabParamOutput[ii].id); 
				indexAckOutput = numberOfOutput + ii;
				
				pNetwork->ppTabOutput[ indexAckOutput ] = newInOutBuffer(&paramNewACK);
				
				pNetwork->ppTabInput[numberOfInput + ii] = pNetwork->ppTabOutput[ indexAckOutput ];
			}
			
			/** Create the input buffers */
			for(ii = 0; ii< numberOfInput ; ++ii)
			{
				pNetwork->ppTabInput[ii] = newInOutBuffer( &(ptabParamInput[ii]) );
			}
		}
		else
		{
			error = 1;
		}
	}
	
	if( !error )
	{
		/** Create the Sender and the Receiver */
		pNetwork->pSender = NETWORK_NewSender(sendBuffSize, pNetwork->numOfInput, pNetwork->ppTabInput);
		pNetwork->pReceiver = NETWORK_NewReceiver(recvBuffSize, pNetwork->numOfOutput, pNetwork->ppTabOutput);
		
		if( pNetwork->ppTabOutput && pNetwork->ppTabInput )
		{
			pNetwork->pReceiver->pSender = pNetwork->pSender;
		}
		else
		{
			error = 1;
		}
	}
    
    /** delete the network if an error occurred */
    if(error)
    {
		NETWORK_DeleteManager(&pNetwork);
	}
    
    return pNetwork;
}

void NETWORK_DeleteManager(network_manager_t** ppNetwork)
{	
	/** -- Delete the Network -- */
	
	/** local declarations */
	network_manager_t* pNetwork = NULL;
	int ii = 0;
	
	if(ppNetwork)
	{
		pNetwork = *ppNetwork;
		
		if(pNetwork)
		{
			NETWORK_DeleteSender( &(pNetwork->pSender) );
			NETWORK_DeleteReceiver( &(pNetwork->pReceiver) );
			
			/** Delete all output buffers including the the buffers of acknowledgement */
			for(ii = 0; ii< pNetwork->numOfOutput ; ++ii)
			{
				deleteInOutBuffer( &(pNetwork->ppTabOutput[ii]) );
			}	
			
			/** Delete the input buffers but not the buffers of acknowledgement already deleted */
			for(ii = 0; ii< pNetwork->numOfInputWithoutAck ; ++ii)
			{
				deleteInOutBuffer( &(pNetwork->ppTabInput[ii]) );
			}
			
			free(pNetwork);
		}
		
		*ppNetwork = NULL;
	}
}

int NETWORK_ManagerSendData(network_manager_t* pNetwork, int inputBufferId, const void* pData)
{
	/** -- Add data to send -- */
	
	/** local declarations */
	int error = 1;
	network_ioBuffer_t* pInputBuffer = NULL;
	
	pInputBuffer = inOutBufferWithId( pNetwork->ppTabInput, pNetwork->numOfInput, inputBufferId);
	
	if(pInputBuffer != NULL)
	{
		error = ringBuffPushBack(pInputBuffer->pBuffer, pData);
	}
	
	return error;
}

int NETWORK_ManagerReadData(network_manager_t* pNetwork, int outputBufferId, void* pData)
{
	/** -- read data received -- */
	
	/** local declarations */
	int error = 1;
	network_ioBuffer_t* pOutputBuffer = NULL;
	
	pOutputBuffer = inOutBufferWithId( pNetwork->ppTabOutput, pNetwork->numOfOutput, outputBufferId);
	
	if(pOutputBuffer != NULL)
	{
		error = ringBuffPopFront(pOutputBuffer->pBuffer, pData);
	}
	
	return error;
}
