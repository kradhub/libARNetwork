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

#include <string.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>

#include <libNetwork/error.h>
#include <libNetwork/frame.h>
#include "ringBuffer.h"
#include <libNetwork/deportedData.h>
#include <libNetwork/paramNewIoBuffer.h>
#include "ioBuffer.h"
#include "sender.h"
#include "receiver.h"

#include "manager.h"
#include <libNetwork/manager.h>


/*****************************************
 * 
 * 			private header:
 *
******************************************/

/**
 *  @brief create manager's IoBuffers.
 *  @warning only call by NETWORK_NewManager()
 *  @pre pManager->ppTabOutput and pManager->ppTabInput must be allocated and set to NULL.
 *  @pre pManager->numOfOutputWithoutAck, pManager->numOfOutput, pManager->numOfInputWithoutAck and pManager->numOfInput must be accurate
 *  @param pManager pointer on the Manager
 *  @param[in] ptabParamInput Table of the parameters of creation of the inputs. The table must contain as many parameters as the number of input buffer.
 * 	@param[in] ptabParamOutput Table of the parameters of creation of the outputs. The table must contain as many parameters as the number of output buffer.
 *  @return error equal to NETWORK_OK if the IoBuffer are correctly created otherwise see eNETWORK_Manager_Error.
 *  @see NETWORK_NewManager()
**/
int NETWORK_ManagerCreateIoBuffer(network_manager_t* pManager,
                                    network_paramNewIoBuffer_t* ptabParamInput, 
                                    network_paramNewIoBuffer_t* ptabParamOutput);

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

network_manager_t* NETWORK_NewManager(	unsigned int recvBuffSize,unsigned int sendBuffSize,
				unsigned int numberOfInput, network_paramNewIoBuffer_t* ptabParamInput,
				unsigned int numberOfOutput, network_paramNewIoBuffer_t* ptabParamOutput)
{
	/** -- Create a new Manager -- */
	
	/** local declarations */
	network_manager_t* pManager = NULL;
	int error = NETWORK_OK;
    
    /** Create the Manager */
    pManager = malloc( sizeof(network_manager_t));
	if(pManager != NULL)
	{
        /** Initialize to default values */
        pManager->pSender = NULL;
        pManager->pReceiver = NULL;
        pManager->ppTabInput = NULL;
        pManager->ppTabOutput = NULL;
        pManager->numOfOutput = 0;
        pManager->numOfOutputWithoutAck = 0;
        pManager->numOfInput = 0;
        pManager->numOfInputWithoutAck = 0;
    }
    else
    {
		error = NETWORK_ERROR_ALLOC;
	}
    
    /**
     *  For each output buffer a buffer of acknowledgement is add and referenced
     *  in the output buffer list and the input buffer list.
    **/

	if( error == NETWORK_OK )
	{
		/** 
		 * Allocate the output buffer list of size of the number of output plus the number of 
		 * buffer of acknowledgement. The size of the list is then two times the number of output.
		**/
		pManager->numOfOutputWithoutAck = numberOfOutput;
		pManager->numOfOutput = 2 * numberOfOutput;
		pManager->ppTabOutput = calloc( pManager->numOfOutput, sizeof(network_ioBuffer_t*) );
        if( pManager->ppTabOutput == NULL )
        {
            error = NETWORK_ERROR_ALLOC;
            pManager->numOfOutput = 0;
            pManager->numOfOutputWithoutAck = 0;
        }
    }
    
    if( error == NETWORK_OK )
	{	
		/** 
		 * Allocate the input buffer list of size of the number of input plus the number of 
		 * buffer of acknowledgement. 
		 * The size of the list is then number of input plus the number of output.
		**/ 
		pManager->numOfInputWithoutAck = numberOfInput;
		pManager->numOfInput = numberOfInput + numberOfOutput;
		pManager->ppTabInput = calloc( pManager->numOfInput, sizeof(network_ioBuffer_t*) );
        if( pManager->ppTabInput == NULL )
        {
            error = NETWORK_ERROR_ALLOC;
            pManager->numOfInput = 0;
            pManager->numOfInputWithoutAck = numberOfOutput;
        }
    }
    
    if( error == NETWORK_OK )
	{	
        /** Create manager's IoBuffers */
        error = NETWORK_ManagerCreateIoBuffer(pManager, ptabParamInput, ptabParamOutput);
	}
    
	if( error == NETWORK_OK )
	{
		/** Create the Sender */
		pManager->pSender = NETWORK_NewSender(sendBuffSize, pManager->numOfInput, pManager->ppTabInput);
        if( pManager->pSender == NULL)
        {
            error = NETWORK_MANAGER_ERROR_NEW_SENDER;
        }
    }
    
    if( error == NETWORK_OK )
	{
        /** Create the Receiver */
		pManager->pReceiver = NETWORK_NewReceiver(recvBuffSize, pManager->numOfOutput, pManager->ppTabOutput);
		if( pManager->pReceiver != NULL)
		{
			pManager->pReceiver->pSender = pManager->pSender;
		}
		else
		{
			error = NETWORK_MANAGER_ERROR_NEW_RECEIVER;
		}
	}
    
    /** delete the Manager if an error occurred */
    if( error != NETWORK_OK)
    {
        sal_print(PRINT_ERROR,"error: %d occurred \n", error );
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
				NETWORK_DeleteIotBuffer( &(pManager->ppTabOutput[ii]) );
			}
            free(pManager->ppTabOutput);
            pManager->ppTabOutput = NULL;
			
			/** Delete the input buffers but not the buffers of acknowledgement already deleted */
			for(ii = 0; ii< pManager->numOfInputWithoutAck ; ++ii)
			{
				NETWORK_DeleteIotBuffer( &(pManager->ppTabInput[ii]) );
			}
            free(pManager->ppTabInput);
            pManager->ppTabInput = NULL;
			
			free(pManager);
            pManager = NULL;
		}
		
		*ppManager = NULL;
	}
}

int NETWORK_ManagerSocketsInit(network_manager_t* pManager,const char* addr, int sendingPort,
                                    int recvPort, int recvTimeoutSec)
{
    /** -- initialize UDP sockets of sending and receiving the data. -- */
	
	/** local declarations */
    int error = NETWORK_OK;
    
    /** check paratemters*/
    if(pManager == NULL || addr== NULL)
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    if( error == NETWORK_OK )
    {
        error = NETWORK_SenderConnection( pManager->pSender,addr, sendingPort );
    }
			
    if( error == NETWORK_OK )
    {
        error = NETWORK_ReceiverBind( pManager->pReceiver, recvPort, recvTimeoutSec );
    }
    
    return error;
}

void* NETWORK_ManagerRunSendingThread(void* data)
{
    /** -- Manage the sending of the data -- */
    
    /** local declarations */
    network_manager_t* pManager = data;
    void* ret = NULL;
    
    /** check paratemters */
    if(pManager != NULL)
    {
        ret = NETWORK_RunSendingThread( pManager->pSender );
    }
    else
    {
        sal_print(PRINT_ERROR,"error: %d occurred \n", NETWORK_ERROR_BAD_PARAMETER );
    }
    
    return ret;
}

void* NETWORK_ManagerRunReceivingThread(void* data)
{
    /** -- Manage the reception of the data -- */
    
    /** local declarations */
    network_manager_t* pManager = data;
    void* ret = NULL;
    
    /** check paratemters */
    if(pManager != NULL)
    {
        ret = NETWORK_RunReceivingThread( pManager->pReceiver );
    }
    else
    {
        sal_print(PRINT_ERROR,"error: %d occurred \n", NETWORK_ERROR_BAD_PARAMETER );
    }
    
    return ret;
}

void NETWORK_ManagerStop(network_manager_t* pManager)
{
    /** -- stop the threads of sending and reception -- */
    
    /** check paratemters */
    if(pManager != NULL)
    {
        NETWORK_StopSender( pManager->pSender );
        NETWORK_StopReceiver( pManager->pReceiver );
    }
}

int NETWORK_ManagerSendData(network_manager_t* pManager, int inputBufferId, const void* pData)
{
	/** -- Add data to send -- */
	
	/** local declarations */
	int error = NETWORK_OK;
	network_ioBuffer_t* pInputBuffer = NULL;
	
    /** check paratemters */
    if(pManager != NULL  )
    {
	    pInputBuffer = NETWORK_IoBufferFromId( pManager->ppTabInput, pManager->numOfInput, inputBufferId);
    }
    else
    {
       error =  NETWORK_ERROR_BAD_PARAMETER;
    }
	
	if(pInputBuffer != NULL)
	{
        /** check paratemters */
		if( !pInputBuffer->deportedData && pData != NULL )
        {
            error = NETWORK_RingBuffPushBack(pInputBuffer->pBuffer, pData);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
	}
    else
    {
        error = NETWORK_ERROR_ID_UNKNOWN;
    }
	
	return error;
}

int NETWORK_ManagerSendDeportedData( network_manager_t* pManager, int inputBufferId,
                                     void* pData, int dataSize,
                                     int (*callBack)(int, void*, int) )
{
	/** -- Add data deported to send -- */
	
	/** local declarations */
	int error = NETWORK_OK;
	network_ioBuffer_t* pInputBuffer = NULL;
    network_DeportedData_t deportedDataTemp;
    
    /** check paratemters */
    if(pManager != NULL)
    {
	    pInputBuffer = NETWORK_IoBufferFromId( pManager->ppTabInput, pManager->numOfInput, inputBufferId);
	}
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
	if(pInputBuffer != NULL)
	{
        /** check paratemters */
        if( pInputBuffer->deportedData && pData != NULL && callBack != NULL)
        {
            /** initialize deportedDataTemp and push it in the InputBuffer */
            deportedDataTemp.pData = pData;
            deportedDataTemp.dataSize = dataSize;
            deportedDataTemp.callBack = callBack;
            
            error = NETWORK_RingBuffPushBack(pInputBuffer->pBuffer, &deportedDataTemp);
        }
        else
        {
		    error = NETWORK_ERROR_BAD_PARAMETER;
        }
	}
    else
    {
        error = NETWORK_ERROR_ID_UNKNOWN;
    }
	
	return error;
}

int NETWORK_ManagerReadData(network_manager_t* pManager, int outputBufferId, void* pData)
{
	/** -- read data received -- */
	
	/** local declarations */
	int error = NETWORK_OK;
	network_ioBuffer_t* pOutputBuffer = NULL;
    
    /** check paratemters */
    if(pManager != NULL)
    {
	    pOutputBuffer = NETWORK_IoBufferFromId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
	
	if(pOutputBuffer != NULL)
	{
        /** check paratemters */
        if( !pOutputBuffer->deportedData )
        {
            /** push the data in the InputBuffer */
		    error = NETWORK_RingBuffPopFront(pOutputBuffer->pBuffer, pData);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
	}
    else
    {
        error = NETWORK_ERROR_ID_UNKNOWN;
    }
	
	return error;
}

int NETWORK_ManagerReaddeportedData( network_manager_t* pManager, int outputBufferId,
                                     void* pData, int dataLimitSize, int* pReadSize)
{
    /** -- read data deported received -- */
	
	/** local declarations */
	int error = NETWORK_OK;
	network_ioBuffer_t* pOutputBuffer = NULL;
    network_DeportedData_t deportedDataTemp;
    int readSize = 0;
	
    /** check paratemters */
    if(pManager != NULL)
    {
	    pOutputBuffer = NETWORK_IoBufferFromId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
    }
	
	if(pOutputBuffer != NULL)
	{
        /** check paratemters */
        if( pOutputBuffer->deportedData )
        {
            /** get data deported */
            error = NETWORK_RingBuffPopFront(pOutputBuffer->pBuffer, &deportedDataTemp);
            
            if( error == NETWORK_OK )
            {
                /** data size check*/
                if(deportedDataTemp.dataSize <= dataLimitSize)
                {
                    /** data copy */
                    memcpy(pData, deportedDataTemp.pData, deportedDataTemp.dataSize);
                    
                    /** set data read */
                    readSize = deportedDataTemp.dataSize;
                    
                    /** free the data deported*/
                    deportedDataTemp.callBack( pOutputBuffer->id, deportedDataTemp.pData, 
                                               NETWORK_DEPORTEDDATA_CALLBACK_FREE);
                }
                else
                {
                    error = NETWORK_ERROR_BUFFER_SIZE;
                }
            }
        }
        else
        {
		    error = NETWORK_ERROR_BAD_PARAMETER;
        }
	}
    else
    {
        error = NETWORK_ERROR_ID_UNKNOWN;
    }
    
    /** return the size of the data read */
    if(pReadSize != NULL)
    {
        *pReadSize = readSize;
    }
	
	return error;
}

/*****************************************
 * 
 * 			private implementation:
 *
******************************************/

int NETWORK_ManagerCreateIoBuffer( network_manager_t* pManager,
                                    network_paramNewIoBuffer_t* ptabParamInput, 
                                    network_paramNewIoBuffer_t* ptabParamOutput )
{
    /** -- Create manager's IoBuffers --*/
    
    /** local declarations */
	int error = NETWORK_OK;
    int ii = 0;
	int indexAckOutput = 0;
    network_paramNewIoBuffer_t paramNewACK;
	
	/** Initialize the default parameters for the buffers of acknowledgement. */
	NETWORK_ParamNewIoBufferDefaultInit(&paramNewACK); 
    paramNewACK.dataType = network_frame_t_TYPE_ACK;
    paramNewACK.buffSize = 1;
	paramNewACK.buffCellSize = sizeof(int);
	paramNewACK.overwriting = 0;
    
    /**
     *  For each output buffer a buffer of acknowledgement is add and referenced
     *  in the output buffer list and the input buffer list.
    **/
    
    /** Create the output buffers and the buffers of acknowledgement */
    for(ii = 0; ii < pManager->numOfOutputWithoutAck; ++ii)
    {
        /** check the IoBuffer identifier */
        if( ptabParamOutput[ii].id < NETWORK_ID_ACK_OFFSET )
        {
            /** set cellSize if deported data is enabled */
            if(ptabParamOutput[ii].deportedData == 1)
            {
                ptabParamOutput[ii].buffCellSize = sizeof(network_DeportedData_t);
            }
            
            pManager->ppTabOutput[ii] = NETWORK_NewIoBuffer( &(ptabParamOutput[ii]) );
            if(pManager->ppTabOutput[ii] == NULL)
            {
                error = NETWORK_MANAGER_ERROR_NEW_IOBUFFER;
            }
            
            /** 
             * Create the buffer of acknowledgement associated with the output buffer and 
             * store it at the end of the output and input buffer lists.
            **/
            paramNewACK.id = idOutputToIdAck(ptabParamOutput[ii].id); 
            indexAckOutput = pManager->numOfOutputWithoutAck + ii;
            
            pManager->ppTabOutput[ indexAckOutput ] = NETWORK_NewIoBuffer(&paramNewACK);
            if(pManager->ppTabOutput[indexAckOutput] == NULL)
            {
                error = NETWORK_MANAGER_ERROR_NEW_IOBUFFER;
            }
            
            pManager->ppTabInput[pManager->numOfInputWithoutAck + ii] = 
                                                            pManager->ppTabOutput[ indexAckOutput ];
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
    }
    
    /** Create the input buffers */
    for(ii = 0; ii< pManager->numOfInputWithoutAck; ++ii)
    {
        /** check the IoBuffer identifier */
        if( ptabParamInput[ii].id < NETWORK_ID_ACK_OFFSET )
        {
            /** set cellSize if deported data is enabled */
            if(ptabParamInput[ii].deportedData == 1)
            {
                ptabParamInput[ii].buffCellSize = sizeof(network_DeportedData_t);
            }
            
            pManager->ppTabInput[ii] = NETWORK_NewIoBuffer( &(ptabParamInput[ii]) );
            if(pManager->ppTabInput[ii] == NULL)
            {
                error = NETWORK_MANAGER_ERROR_NEW_IOBUFFER;
            }
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
    }
    
    return error;
}
