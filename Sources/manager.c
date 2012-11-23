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
	/** -- Create a new Manager -- */
	
	/** local declarations */
	network_manager_t* pManager = NULL;
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
    
    /** Create the Manager */
    pManager = malloc( sizeof(network_manager_t));
    
	if(pManager == NULL)
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
		pManager->numOfOutputWithoutAck = numberOfOutput;
		pManager->numOfOutput = 2 * numberOfOutput;
		pManager->ppTabOutput = malloc(sizeof(network_ioBuffer_t*) * pManager->numOfOutput );
		
		/** 
		 * Allocate the input buffer list of size of the number of input plus the number of 
		 * buffer of acknowledgement. 
		 * The size of the list is then number of input plus the number of output.
		**/ 
		pManager->numOfInputWithoutAck = numberOfInput;
		pManager->numOfInput = numberOfInput + numberOfOutput;
		pManager->ppTabInput = malloc( sizeof(network_ioBuffer_t*) * pManager->numOfInput );
		
		
		if( pManager->ppTabOutput && pManager->ppTabInput)
		{
			/** Create the output buffers and the buffers of acknowledgement */
			for(ii = 0; ii < numberOfOutput; ++ii)
			{
				pManager->ppTabOutput[ii] = newInOutBuffer( &(ptabParamOutput[ii]) );
				
				/** 
				 * Create the buffer of acknowledgement associated with the output buffer and 
				 * store it at the end of the output and input buffer lists.
				**/
				paramNewACK.id = idOutputToIdAck(ptabParamOutput[ii].id); 
				indexAckOutput = numberOfOutput + ii;
				
				pManager->ppTabOutput[ indexAckOutput ] = newInOutBuffer(&paramNewACK);
				
				pManager->ppTabInput[numberOfInput + ii] = pManager->ppTabOutput[ indexAckOutput ];
			}
			
			/** Create the input buffers */
			for(ii = 0; ii< numberOfInput ; ++ii)
			{
				pManager->ppTabInput[ii] = newInOutBuffer( &(ptabParamInput[ii]) );
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
		pManager->pSender = NETWORK_NewSender(sendBuffSize, pManager->numOfInput, pManager->ppTabInput);
		pManager->pReceiver = NETWORK_NewReceiver(recvBuffSize, pManager->numOfOutput, pManager->ppTabOutput);
		
		if( pManager->ppTabOutput && pManager->ppTabInput )
		{
			pManager->pReceiver->pSender = pManager->pSender;
		}
		else
		{
			error = 1;
		}
	}
    
    /** delete the Manager if an error occurred */
    if(error)
    {
		NETWORK_DeleteManager(&pManager);
	}
    
    return pManager;
}

void NETWORK_DeleteManager(network_manager_t** ppManager)
{	
	/** -- Delete the Manager -- */
	
	/** local declarations */
	network_manager_t* pManager = NULL;
	int ii = 0;
	
	if(ppManager)
	{
		pManager = *ppManager;
		
		if(pManager)
		{
			NETWORK_DeleteSender( &(pManager->pSender) );
			NETWORK_DeleteReceiver( &(pManager->pReceiver) );
			
			/** Delete all output buffers including the the buffers of acknowledgement */
			for(ii = 0; ii< pManager->numOfOutput ; ++ii)
			{
				deleteInOutBuffer( &(pManager->ppTabOutput[ii]) );
			}	
			
			/** Delete the input buffers but not the buffers of acknowledgement already deleted */
			for(ii = 0; ii< pManager->numOfInputWithoutAck ; ++ii)
			{
				deleteInOutBuffer( &(pManager->ppTabInput[ii]) );
			}
			
			free(pManager);
		}
		
		*ppManager = NULL;
	}
}

int NETWORK_ManagerSendData(network_manager_t* pManager, int inputBufferId, const void* pData)
{
	/** -- Add data to send -- */
	
	/** local declarations */
	int error = 1;
	network_ioBuffer_t* pInputBuffer = NULL;
	
	pInputBuffer = inOutBufferWithId( pManager->ppTabInput, pManager->numOfInput, inputBufferId);
	
	if(pInputBuffer != NULL)
	{
		error = ringBuffPushBack(pInputBuffer->pBuffer, pData);
	}
	
	return error;
}

int NETWORK_ManagerReadData(network_manager_t* pManager, int outputBufferId, void* pData)
{
	/** -- read data received -- */
	
	/** local declarations */
	int error = 1;
	network_ioBuffer_t* pOutputBuffer = NULL;
	
	pOutputBuffer = inOutBufferWithId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
	
	if(pOutputBuffer != NULL)
	{
		error = ringBuffPopFront(pOutputBuffer->pBuffer, pData);
	}
	
	return error;
}
