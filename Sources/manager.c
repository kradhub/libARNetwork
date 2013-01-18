/**
 *  @file manager.c
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <stdlib.h>

#include <inttypes.h>
#include <stddef.h>

#include <string.h>

#include <errno.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>

#include <libNetwork/status.h>
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
 *             define :
 *
******************************************/
#define TAG "Manager"

/*****************************************
 * 
 *             private header:
 *
******************************************/

/**
 *  @brief create manager's IoBuffers.
 *  @warning only call by NETWORK_NewManager()
 *  @pre pManager->ppTabOutput and pManager->ppTabInput must be allocated and set to NULL.
 *  @pre pManager->numOfOutputWithoutAck, pManager->numOfOutput, pManager->numOfInputWithoutAck and pManager->numOfInput must be accurate
 *  @param pManager pointer on the Manager
 *  @param[in] ptabParamInput Table of the parameters of creation of the inputs. The table must contain as many parameters as the number of input buffer.
 *  @param[in] ptabParamOutput Table of the parameters of creation of the outputs. The table must contain as many parameters as the number of output buffer.
 *  @param[in] sendBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 *  @return error equal to NETWORK_OK if the IoBuffer are correctly created otherwise see eNETWORK_Manager_Error.
 *  @see NETWORK_NewManager()
**/
eNETWORK_Error NETWORK_ManagerCreateIoBuffer( network_manager_t* pManager,
                                                network_paramNewIoBuffer_t* ptabParamInput, 
                                                network_paramNewIoBuffer_t* ptabParamOutput,
                                                unsigned int sendBufferSize );

/*****************************************
 * 
 *             implementation :
 *
******************************************/

network_manager_t* NETWORK_NewManager( unsigned int recvBufferSize,unsigned int sendBufferSize,
                unsigned int numberOfInput, network_paramNewIoBuffer_t* ptabParamInput,
                unsigned int numberOfOutput, network_paramNewIoBuffer_t* ptabParamOutput,
                eNETWORK_Error* pError)
{
    /** -- Create a new Manager -- */
    
    /** local declarations */
    network_manager_t* pManager = NULL;
    eNETWORK_Error error = NETWORK_OK;
    
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
        error = NETWORK_ManagerCreateIoBuffer(pManager, ptabParamInput, ptabParamOutput, sendBufferSize);
    }
    
    if( error == NETWORK_OK )
    {
        /** Create the Sender */
        pManager->pSender = NETWORK_NewSender(sendBufferSize, pManager->numOfInput, pManager->ppTabInput);
        if( pManager->pSender == NULL)
        {
            error = NETWORK_MANAGER_ERROR_NEW_SENDER;
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** Create the Receiver */
        pManager->pReceiver = NETWORK_NewReceiver(recvBufferSize, pManager->numOfOutput, pManager->ppTabOutput);
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
        SAL_PRINT(PRINT_ERROR, TAG, "error: %d occurred \n", error );
        NETWORK_DeleteManager(&pManager);
    }

    /** return the error */
    if(pError != NULL)
    {
        *pError = error;
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
                NETWORK_DeleteIoBuffer( &(pManager->ppTabOutput[ii]) );
            }
            free(pManager->ppTabOutput);
            pManager->ppTabOutput = NULL;
            
            /** Delete the input buffers but not the buffers of acknowledgement already deleted */
            for(ii = 0; ii< pManager->numOfInputWithoutAck ; ++ii)
            {
                NETWORK_DeleteIoBuffer( &(pManager->ppTabInput[ii]) );
            }
            free(pManager->ppTabInput);
            pManager->ppTabInput = NULL;
            
            free(pManager);
            pManager = NULL;
        }
        
        *ppManager = NULL;
    }
}

eNETWORK_Error NETWORK_ManagerSocketsInit(network_manager_t* pManager,const char* addr, 
                                            int sendingPort,int recvPort, int recvTimeoutSec)
{
    /** -- initialize UDP sockets of sending and receiving the data. -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    
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
    
    SAL_PRINT(PRINT_DEBUG, TAG," error: %d occurred \n", error );
    
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
        SAL_PRINT(PRINT_ERROR, TAG, "error: %d occurred \n", NETWORK_ERROR_BAD_PARAMETER );
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
        SAL_PRINT(PRINT_ERROR, TAG,"error: %d occurred \n", NETWORK_ERROR_BAD_PARAMETER );
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

eNETWORK_Error NETWORK_ManagerSendData(network_manager_t* pManager, int inputBufferId, const uint8_t* pData)
{
    /** -- Add data to send -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
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

eNETWORK_Error NETWORK_ManagerSendDeportedData( network_manager_t* pManager, int inputBufferId,
                                                  uint8_t* pData, int dataSize, void* pCustomData, 
                                                  network_deportDatacallback callback )
{
    /** -- Add data deported to send -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_ioBuffer_t* pInputBuffer = NULL;
    network_DeportedData_t deportedDataTemp;
    network_DeportedData_t deportedDataOverwritten;
    
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
        if( pInputBuffer->deportedData && 
            pData != NULL && 
            callback != NULL &&  
            dataSize < ( pManager->pSender->pSendingBuffer->numberOfCell - offsetof(network_frame_t, data) ) )
        {
            /** initialize deportedDataTemp and push it in the InputBuffer */
            deportedDataTemp.pData = pData;
            deportedDataTemp.dataSize = dataSize;
            deportedDataTemp.pCustomData = pCustomData;
            deportedDataTemp.callback = callback;
            
            /** if the buffer is overwriting and it is full */
            if( pInputBuffer->pBuffer->isOverwriting == 1 &&
                NETWORK_RingBuffGetFreeCellNb(pInputBuffer->pBuffer) == 0 )
            {
                /** get the deported Data Overwritten */
                error = NETWORK_RingBuffPopFront( pInputBuffer->pBuffer, (uint8_t*) &deportedDataOverwritten );
                /** free the deported Data Overwritten */
                deportedDataOverwritten.callback( pInputBuffer->id, 
                                                  deportedDataOverwritten.pData,
                                                  deportedDataOverwritten.pCustomData, 
                                                  NETWORK_CALLBACK_STATUS_FREE);
            }
            
            if(error == NETWORK_OK)
            {
                error = NETWORK_RingBuffPushBack(pInputBuffer->pBuffer, (uint8_t*) &deportedDataTemp);
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
    
    return error;
}

eNETWORK_Error NETWORK_ManagerReadData(network_manager_t* pManager, int outputBufferId, uint8_t* pData)
{
    /** -- read data received -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_ioBuffer_t* pOutputBuffer = NULL;
    int semError = 0;

    /** check paratemters */
    if(pManager != NULL)
    {
        pOutputBuffer = NETWORK_IoBufferFromId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
        
        /** check pOutputBuffer */
        if(pOutputBuffer == NULL)
        {
            error = NETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    if( error == NETWORK_OK )
    {
        /** try to take the semaphore */
        semError = sal_sem_trywait( &(pOutputBuffer->outputSem) );
        
        if(semError)
        {
            switch( errno )
            {
                case EAGAIN : /** no semaphore */ 
                    error = NETWORK_ERROR_BUFFER_EMPTY;
                break;
                
                default:
                    error = NETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** check paratemters */
        if( !pOutputBuffer->deportedData )
        {
            /** pop the data in the OutputBuffer */
            error = NETWORK_RingBuffPopFront(pOutputBuffer->pBuffer, pData);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
    }
    
    return error;
}

eNETWORK_Error NETWORK_ManagerReadDeportedData( network_manager_t* pManager, int outputBufferId,
                                     uint8_t* pData, int dataLimitSize, int* pReadSize)
{
    /** -- read data deported received -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_ioBuffer_t* pOutputBuffer = NULL;
    network_DeportedData_t deportedDataTemp;
    int readSize = 0;
    int semError = 0;
    
    /** check paratemters */
    if(pManager != NULL)
    {
        pOutputBuffer = NETWORK_IoBufferFromId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
        
        /** check pOutputBuffer */
        if(pOutputBuffer == NULL)
        {
            error = NETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    if( error == NETWORK_OK )
    {
        /** try to take the semaphore */
        semError = sal_sem_trywait( &(pOutputBuffer->outputSem) );
        
        if(semError)
        {
            switch( errno )
            {
                case EAGAIN : /** no semaphore */ 
                    error = NETWORK_ERROR_BUFFER_EMPTY;
                break;
                
                default:
                    error = NETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** check paratemters */
        if( pOutputBuffer->deportedData )
        {
            /** get data deported */
            error = NETWORK_RingBuffPopFront(pOutputBuffer->pBuffer, (uint8_t*) &deportedDataTemp);
            
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
                    deportedDataTemp.callback( pOutputBuffer->id, 
                                               deportedDataTemp.pData,
                                               deportedDataTemp.pCustomData, 
                                               NETWORK_CALLBACK_STATUS_FREE);
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
    
    /** return the size of the data read */
    if(pReadSize != NULL)
    {
        *pReadSize = readSize;
    }
    
    return error;
}

eNETWORK_Error NETWORK_ManagerReadDataWithTimeout( network_manager_t* pManager, 
                                                     int outputBufferId, 
                                                     uint8_t* pData,
                                                     int timeoutMs )
{
    /** -- read data received with timeout -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_ioBuffer_t* pOutputBuffer = NULL;
    int semError = 0;
    struct timespec semTimeout;
    
    /** check paratemters */
    if(pManager != NULL)
    {
        pOutputBuffer = NETWORK_IoBufferFromId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
        
        /** check pOutputBuffer */
        if(pOutputBuffer == NULL)
        {
            error = NETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    if( error == NETWORK_OK )
    {
        /** convert timeoutMs in timespec */
        semTimeout.tv_sec = timeoutMs / 1000;
        semTimeout.tv_nsec = (timeoutMs % 1000) * 1000000;
        
        /** try to take the semaphore with timeout*/
        semError = sal_sem_timedwait( &(pOutputBuffer->outputSem), &semTimeout);
        
        if(semError)
        {
            switch( errno )
            {
                case ETIMEDOUT : /** semaphore time out */ 
                    error = NETWORK_ERROR_BUFFER_EMPTY;
                break;
                
                default:
                    error = NETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** check paratemters */
        if( !pOutputBuffer->deportedData )
        {
            /** pop the data in the OutputBuffer */
            error = NETWORK_RingBuffPopFront(pOutputBuffer->pBuffer, pData);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
    }

    
    return error;
}

eNETWORK_Error NETWORK_ManagerReadDeportedDataWithTimeout( network_manager_t* pManager, 
                                                             int outputBufferId,
                                                             uint8_t* pData, 
                                                             int dataLimitSize, 
                                                             int* pReadSize,
                                                             int timeoutMs )
{
    /** -- read data deported received with timeout -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_ioBuffer_t* pOutputBuffer = NULL;
    network_DeportedData_t deportedDataTemp;
    int readSize = 0;
    int semError = 0;
    struct timespec semTimeout;
    
    /** check paratemters */
    if(pManager != NULL)
    {
        pOutputBuffer = NETWORK_IoBufferFromId( pManager->ppTabOutput, pManager->numOfOutput, outputBufferId);
        
        /** check pOutputBuffer */
        if(pOutputBuffer == NULL)
        {
            error = NETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    if( error == NETWORK_OK )
    {
        /** convert timeoutMs in timespec */
        semTimeout.tv_sec = timeoutMs / 1000;
        semTimeout.tv_nsec = (timeoutMs % 1000) * 1000000;
        
        /** try to take the semaphore with timeout*/
        semError = sal_sem_timedwait( &(pOutputBuffer->outputSem), &semTimeout);
        
        if(semError)
        {
            switch( errno )
            {
                case ETIMEDOUT : /** semaphore time out */ 
                    error = NETWORK_ERROR_BUFFER_EMPTY;
                break;
                
                default:
                    error = NETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** check paratemters */
        if( pOutputBuffer->deportedData )
        {
            /** get data deported */
            error = NETWORK_RingBuffPopFront(pOutputBuffer->pBuffer, (uint8_t*) &deportedDataTemp);
            
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
                    deportedDataTemp.callback( pOutputBuffer->id, 
                                               deportedDataTemp.pData,
                                               deportedDataTemp.pCustomData,
                                               NETWORK_CALLBACK_STATUS_FREE );
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

    
    /** return the size of the data read */
    if(pReadSize != NULL)
    {
        *pReadSize = readSize;
    }
    
    return error;
}


/*****************************************
 * 
 *             private implementation:
 *
******************************************/

eNETWORK_Error NETWORK_ManagerCreateIoBuffer( network_manager_t* pManager,
                                                network_paramNewIoBuffer_t* ptabParamInput, 
                                                network_paramNewIoBuffer_t* ptabParamOutput,
                                                unsigned int sendBufferSize )
{
    /** -- Create manager's IoBuffers --*/
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    int ii = 0;
    int indexAckOutput = 0;
    network_paramNewIoBuffer_t paramNewACK;
    
    /** Initialize the default parameters for the buffers of acknowledgement. */
    NETWORK_ParamNewIoBufferDefaultInit(&paramNewACK); 
    paramNewACK.dataType = NETWORK_FRAME_TYPE_ACK;
    paramNewACK.numberOfCell = 1;
    paramNewACK.cellSize = sizeof(int);
    paramNewACK.isOverwriting = 0;
    
    /**
     *  For each output buffer a buffer of acknowledgement is add and referenced
     *  in the output buffer list and the input buffer list.
    **/
    
    /** Create the output buffers and the buffers of acknowledgement */
    for(ii = 0; ii < pManager->numOfOutputWithoutAck; ++ii)
    {
        /** check parameters */
        /**     id is smaller than the id acknowledge offset */
        if( ptabParamOutput[ii].id < NETWORK_ID_ACK_OFFSET )
        {
            /** set cellSize if deported data is enabled */
            if(ptabParamOutput[ii].deportedData == 1)
            {
                ptabParamOutput[ii].cellSize = sizeof(network_DeportedData_t);
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
        /** check parameters */
        /**     id is smaller than the id acknowledge offset */
        /**     dataSize isn't too big */
        if( ptabParamInput[ii].id < NETWORK_ID_ACK_OFFSET && 
            ptabParamInput[ii].cellSize < ( sendBufferSize - offsetof(network_frame_t, data) ))
        {
            /** set cellSize if deported data is enabled */
            if(ptabParamInput[ii].deportedData == 1)
            {
                ptabParamInput[ii].cellSize = sizeof(network_DeportedData_t);
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
