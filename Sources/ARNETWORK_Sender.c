/**
 *  @file ARNETWORK_Sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <stdlib.h>

#include <stddef.h>

#include <errno.h>

#include <unistd.h>
#include <string.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Socket.h>
#include <libARSAL/ARSAL_Endianness.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_Frame.h>
#include "ARNETWORK_Buffer.h"
#include "ARNETWORK_VariableSizeData.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_IOBuffer.h"

#include "ARNETWORK_Sender.h"

/*****************************************
 * 
 *             define :
 *
******************************************/

#define ARNETWORK_SENDER_TAG "ARNETWORK_Sender"
#define ARNETWORK_SENDER_MILLISECOND 1000
#define ARNETWORK_SENDER_FIRST_SEQ 1 /**< first sequence number sent */

/*****************************************
 * 
 *             private header:
 *
******************************************/

/**
 *  @brief send the data
 *  @param senderPtr the pointer on the Sender
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
**/
void ARNETWORK_Sender_Send(ARNETWORK_Sender_t *senderPtr);

/**
 *  @brief add data to the sender buffer
 *  @param senderPtr the pointer on the Sender
 *  @param inputBufferPtr Pointer on the input buffer
 *  @param seqNum sequence number
 *  @return error eARNETWORK_ERROR
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
**/
eARNETWORK_ERROR ARNETWORK_Sender_AddToBuffer(ARNETWORK_Sender_t *senderPtr, const ARNETWORK_IOBuffer_t *inputBufferPtr, int seqNum);

/**
 *  @brief call the Callback this timeout status
 *  @param senderPtr the pointer on the Sender
 *  @param inputBufferPtr Pointer on the input buffer
 *  @return eARNETWORK_MANAGER_CALLBACK_RETURN
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
**/
eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_Sender_TimeOutCallback(ARNETWORK_Sender_t *senderPtr, const ARNETWORK_IOBuffer_t *inputBufferPtr);

/**
 *  @brief manage the return of the callback
 *  @param senderPtr the pointer on the Sender
 *  @param[in] inputBufferPtr Pointer on the input buffer
 *  @param[in] callbackReturn return of the callback
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
**/
void ARNETWORK_Sender_ManageTimeOut(ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *inputBufferPtr, eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn);

/*****************************************
 * 
 *             implementation :
 *
******************************************/

ARNETWORK_Sender_t* ARNETWORK_Sender_New(unsigned int sendingBufferSize, unsigned int numberOfInputBuffer, ARNETWORK_IOBuffer_t **inputBufferPtrArr, ARNETWORK_IOBuffer_t **inputBufferPtrMap)
{    
    /** -- Create a new sender -- */
    
    /** local declarations */
    ARNETWORK_Sender_t* senderPtr =  NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    /** Create the sender */
    senderPtr =  malloc(sizeof(ARNETWORK_Sender_t));
    
    if(senderPtr)
    {
        senderPtr->isAlive = 1;
        senderPtr->numberOfInputBuff = numberOfInputBuffer;
        senderPtr->inputBufferPtrArr = inputBufferPtrArr;    
        senderPtr->inputBufferPtrMap = inputBufferPtrMap;
        
        /** Create the Sending buffer */
        senderPtr->sendingBufferPtr = ARNETWORK_Buffer_New(sendingBufferSize, 1);
            
        if(senderPtr->sendingBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_NEW_BUFFER;
        }
        
        /** delete the sender if an error occurred */
        if(error != ARNETWORK_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_SENDER_TAG,"error: %d occurred \n", error );
            ARNETWORK_Sender_Delete(&senderPtr);
        }
        
        senderPtr->seq = ARNETWORK_SENDER_FIRST_SEQ;
    }
    
    return senderPtr;
}

void ARNETWORK_Sender_Delete(ARNETWORK_Sender_t **senderPtrAddr)
{
    /** -- Delete the sender -- */
    
    /** local declarations */
    ARNETWORK_Sender_t *senderPtr = NULL;
    
    if(senderPtrAddr != NULL)
    {
        senderPtr = *senderPtrAddr;
        
        if(senderPtr != NULL)
        {
            ARNETWORK_Buffer_Delete( &(senderPtr->sendingBufferPtr) );
            
            free(senderPtr);
            senderPtr = NULL;
        }
        *senderPtrAddr = NULL;
    }
}

void* ARNETWORK_Sender_ThreadRun(void* data)
{
    /** -- Manage the sending of the data on the sender' socket -- */
    
    /** local declarations */
    ARNETWORK_Sender_t *senderPtr = data;
    int inputBufferIndex = 0;
    ARNETWORK_IOBuffer_t *inputBufferPtrTemp = NULL; /**< pointer of the input buffer in processing */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    while( senderPtr->isAlive )
    {
        usleep( ARNETWORK_SENDER_MILLISECOND );
        
        /** for each input buffer try to send the data if necessary */
        for(inputBufferIndex = 0 ; inputBufferIndex < senderPtr->numberOfInputBuff ; ++inputBufferIndex  )
        {
            inputBufferPtrTemp = senderPtr->inputBufferPtrArr[inputBufferIndex];
            
            /** decrement the time to wait */
            if(inputBufferPtrTemp->waitTimeCount > 0)
            {
                --(inputBufferPtrTemp->waitTimeCount);
            }

            if( ARNETWORK_IOBuffer_IsWaitAck(inputBufferPtrTemp)  ) 
            {
                if(inputBufferPtrTemp->ackWaitTimeCount == 0)
                {
                    if(inputBufferPtrTemp->retryCount == 0)
                    {
                        /** if there are timeout and too sending retry ... */
                        
                        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_SENDER_TAG," !!! too retry !!! \n");
                        
                        callbackReturn = ARNETWORK_Sender_TimeOutCallback(senderPtr, inputBufferPtrTemp);
                        
                        ARNETWORK_Sender_ManageTimeOut(senderPtr, inputBufferPtrTemp, callbackReturn);
                        
                    }
                    else
                    {
                        /** if there is a timeout, retry to send the data */
                       
                        error = ARNETWORK_Sender_AddToBuffer(senderPtr, inputBufferPtrTemp, inputBufferPtrTemp->seqWaitAck);
                        
                        if( error == ARNETWORK_OK)
                        {
                            /** reset the timeout counter*/
                            inputBufferPtrTemp->ackWaitTimeCount = inputBufferPtrTemp->ackTimeoutMs;
                            
                            /** decrement the number of retry still possible is retryCount isn't -1 */
                            if(inputBufferPtrTemp->retryCount > 0)
                            {
                                --(inputBufferPtrTemp->retryCount);
                            }
                        }
                    }
                }
                
                /** decrement the time to wait before considering as a timeout */
                if(inputBufferPtrTemp->ackWaitTimeCount > 0 )
                {
                    --(inputBufferPtrTemp->ackWaitTimeCount);
                }
            }
            else if(ARNETWORK_RingBuffer_IsEmpty(inputBufferPtrTemp->bufferPtr) == ARNETWORK_OK && inputBufferPtrTemp->waitTimeCount == 0)
            {
                /** try to add the latest data of the input buffer in the sending buffer*/
                if( !ARNETWORK_Sender_AddToBuffer(senderPtr, inputBufferPtrTemp, senderPtr->seq) )
                {
                    inputBufferPtrTemp->waitTimeCount = inputBufferPtrTemp->sendingWaitTimeMs;
                    
                    switch(inputBufferPtrTemp->dataType)
                    {
                        case ARNETWORK_FRAME_TYPE_DATA_WITH_ACK:
                            /** 
                             * reinitialize the input buffer parameters,
                             * save the sequence wait for the acknowledgement,
                             * and pass on waiting acknowledgement.
                            **/
                            inputBufferPtrTemp->isWaitAck = 1;
                            inputBufferPtrTemp->seqWaitAck = senderPtr->seq;
                            inputBufferPtrTemp->ackWaitTimeCount = inputBufferPtrTemp->ackTimeoutMs;
                            inputBufferPtrTemp->retryCount = inputBufferPtrTemp->numberOfRetry;    
                            break;
                        
                        case ARNETWORK_FRAME_TYPE_DATA:
                            /** delete the data sent*/
                            ARNETWORK_IOBuffer_DeleteData(inputBufferPtrTemp, ARNETWORK_MANAGER_CALLBACK_STATUS_SENT);
                            break;
                        
                        case ARNETWORK_FRAME_TYPE_ACK:
                            /** pop the acknowledgement sent*/
                            ARNETWORK_RingBuffer_PopFront(inputBufferPtrTemp->bufferPtr, NULL);
                            break;
                        
                        case ARNETWORK_FRAME_TYPE_KEEP_ALIVE:
                            
                            break;
                        
                        default:
                            
                            break;
                    }
                    
                    /** increment the sequence number */
                    ++(senderPtr->seq);
                }
            }
        }
        ARNETWORK_Sender_Send(senderPtr);
    }
    
    ARSAL_Socket_Close(senderPtr->socket);
    
    return NULL;
}

void ARNETWORK_Sender_Stop(ARNETWORK_Sender_t *senderPtr)
{
    /** -- Stop the sending -- */
    senderPtr->isAlive = 0;
}

eARNETWORK_ERROR ARNETWORK_Sender_AckReceived(ARNETWORK_Sender_t *senderPtr, int ID, int seqNumber)
{
    /** -- Receive an acknowledgment fo a data -- */
    
    /** local declarations */
    ARNETWORK_IOBuffer_t *inputBufferPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    inputBufferPtr = senderPtr->inputBufferPtrMap[ID]; 
    
    if(inputBufferPtr != NULL)
    {
        /**
         *  Transmit the acknowledgment to the input buffer. 
         *     if the acknowledgment is suiarray the waiting data is popped
        **/
        error = ARNETWORK_IOBuffer_AckReceived(inputBufferPtr, seqNumber);
    }
    else
    {
        error = ARNETWORK_ERROR_ID_UNKNOWN;
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_Sender_Connect(ARNETWORK_Sender_t *senderPtr, const char *addr, int port)
{
    /** -- Connect the socket in UDP to a port of an address -- */
    
    /** local declarations */
    struct sockaddr_in sendSin;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int connectError;
    
    sendSin.sin_addr.s_addr = inet_addr(addr);
    sendSin.sin_family = AF_INET;
    sendSin.sin_port = htons(port);
    
    senderPtr->socket = ARSAL_Socket_Create(AF_INET, SOCK_DGRAM, 0);

    connectError = ARSAL_Socket_Connect( senderPtr->socket, (struct sockaddr*) &sendSin, sizeof(sendSin) );
    
    if(connectError != 0)
    {
        switch( errno )
        {
            case EACCES:
                error = ARNETWORK_ERROR_SOCKET_PERMISSION_DENIED;
                break;
            
            default:
                error = ARNETWORK_ERROR_SOCKET;
                break;
        }
    }
    
    return error;
}

eARNETWORK_ERROR ARNETWORK_Sender_Flush(ARNETWORK_Sender_t *senderPtr)
{
    /** -- Flush all IoBuffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int inputIndex;
    ARNETWORK_IOBuffer_t *inputBufferTemp = NULL;
    
    /** for each input buffer */
    for(inputIndex = 0; inputIndex < senderPtr->numberOfInputBuff && error == ARNETWORK_OK; ++inputIndex)
    {
        inputBufferTemp = senderPtr->inputBufferPtrArr[inputIndex];
        
        /**  flush the IoBuffer */
        error = ARNETWORK_IOBuffer_Flush(inputBufferTemp);
    }
    
    return error;
}

void ARNETWORK_Sender_Reset(ARNETWORK_Sender_t *senderPtr)
{
    /** -- Reset the Sender -- */
    
    /** local declarations */
    
    /** flush all IoBuffer */
    ARNETWORK_Sender_Flush(senderPtr);
    
    /** reset */
    senderPtr->seq = ARNETWORK_SENDER_FIRST_SEQ;
}

/*****************************************
 * 
 *             private implementation:
 *
******************************************/

void ARNETWORK_Sender_Send(ARNETWORK_Sender_t *senderPtr)
{
    /**  -- send the data -- */
    
    /** local declarations */
    int numberOfByteToCopy = 0;
    
    if( !ARNETWORK_Buffer_IsEmpty(senderPtr->sendingBufferPtr) )
    {
        numberOfByteToCopy = senderPtr->sendingBufferPtr->frontPtr - senderPtr->sendingBufferPtr->startPtr;
        
        ARSAL_Socket_Send(senderPtr->socket, senderPtr->sendingBufferPtr->startPtr, numberOfByteToCopy, 0);
        
        senderPtr->sendingBufferPtr->frontPtr = senderPtr->sendingBufferPtr->startPtr;
    }
}

eARNETWORK_ERROR ARNETWORK_Sender_AddToBuffer(ARNETWORK_Sender_t *senderPtr, const ARNETWORK_IOBuffer_t *inputBufferPtr, int seqNum)
{
    /** -- add data to the sender buffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint32_t droneEndianInt32 = 0;
    int sizeNeed = 0;
    int dataSize = 0;
    ARNETWORK_VariableSizeData_t variableSizeData;
    
    /** get the data size */
    if( inputBufferPtr->isUsingVariableSizeData )
    {
        /** IOBuffer using variable size data */
        
        /** get the size of the date pointed*/
        error = ARNETWORK_RingBuffer_Front(inputBufferPtr->bufferPtr, (uint8_t*) &variableSizeData);
        if( error == ARNETWORK_OK )
        {
            dataSize = variableSizeData.dataSize;
        }
    }
    else
    {
        /** IOBuffer using fixed size data */
        
        dataSize = inputBufferPtr->bufferPtr->cellSize;
    }
    
    if( error == ARNETWORK_OK )
    {
        /** calculate the size needed */
        sizeNeed = offsetof(ARNETWORK_Frame_t, dataPtr) + dataSize;
        
        /** check the size free */
        if( ARNETWORK_Buffer_GetFreeCellNumber(senderPtr->sendingBufferPtr) < sizeNeed)
        {
            error = ARNETWORK_ERROR_BUFFER_SIZE;
        }
    }
    
    if( error == ARNETWORK_OK )
    {
        /** add type */
        memcpy( senderPtr->sendingBufferPtr->frontPtr, &(inputBufferPtr->dataType), sizeof(uint8_t) );
        senderPtr->sendingBufferPtr->frontPtr +=  sizeof(uint8_t) ;
        
        /** add id */
        memcpy( senderPtr->sendingBufferPtr->frontPtr, &(inputBufferPtr->ID), sizeof(uint8_t) );
        senderPtr->sendingBufferPtr->frontPtr +=  sizeof(uint8_t);
        
        /** add seq */
        droneEndianInt32 =  htodl(seqNum);
        memcpy( senderPtr->sendingBufferPtr->frontPtr, &(droneEndianInt32), sizeof(uint32_t));
        senderPtr->sendingBufferPtr->frontPtr +=  sizeof(uint32_t);
        
        /** add size */
        droneEndianInt32 =  htodl(sizeNeed); 
        memcpy( senderPtr->sendingBufferPtr->frontPtr, &(droneEndianInt32), sizeof(uint32_t));
        senderPtr->sendingBufferPtr->frontPtr +=  sizeof(uint32_t);
        
        /** add data */
        if(inputBufferPtr->isUsingVariableSizeData)
        {
            /** IOBuffer using variable size data */
            
            /** copy the data pointed by the variableSizeData */
            memcpy(senderPtr->sendingBufferPtr->frontPtr, variableSizeData.dataPtr, dataSize);
        }
        else
        {
            /** IOBuffer using fixed size data */
            
            /** copy the data on the ring buffer */            
            error = ARNETWORK_RingBuffer_Front(inputBufferPtr->bufferPtr, senderPtr->sendingBufferPtr->frontPtr);
        }
        
        if( error == ARNETWORK_OK )
        {
            /** if the adding of the data is successful, increment the front of the Sending Buffer */
            senderPtr->sendingBufferPtr->frontPtr += dataSize;
        }
        else
        {
            /** if the adding of the data is failed, decrement the front of the SendingBuffer */
            senderPtr->sendingBufferPtr->frontPtr -= offsetof(ARNETWORK_Frame_t, dataPtr);
        }
    }
    
    return error;
}

eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_Sender_TimeOutCallback(ARNETWORK_Sender_t *senderPtr, const ARNETWORK_IOBuffer_t *inputBufferPtr)
{
    /** -- callback -- */
    
    /** local declarations */
    ARNETWORK_VariableSizeData_t variableSizeDataTemp;
    
    int ret = 0;
    
    if(inputBufferPtr->isUsingVariableSizeData)
    {
        /** IOBuffer using variable size data */
        
        ARNETWORK_RingBuffer_Front( inputBufferPtr->bufferPtr, (uint8_t*) &variableSizeDataTemp);
        
        ret = variableSizeDataTemp.callback(inputBufferPtr->ID, variableSizeDataTemp.dataPtr, variableSizeDataTemp.customData, ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT);
    }
    else
    {
        /** IOBuffer using fixed size data */
        
        //pCallback = senderPtr->callback; // !!! todo !!!
    }
    
    return ret;
}

void ARNETWORK_Sender_ManageTimeOut(ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *inputBufferPtr, eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn)
{
    /**  -- Manager the return of the callback -- */
    
    /** local declarations */

    switch(callbackReturn)
    {
        case ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY :
            /** reset the retry counter */
            inputBufferPtr->retryCount = inputBufferPtr->numberOfRetry;
            break;
        
        case ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP :
            /** pop the data*/
            ARNETWORK_IOBuffer_DeleteData( inputBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_FREE );
            
            /** force the waiting acknowledge at 0 */
            inputBufferPtr->isWaitAck = 0;
            break;
        
        case ARNETWORK_MANAGER_CALLBACK_RETURN_FLUSH :
            /** fluch all IoBuffer */
            ARNETWORK_Sender_Flush(senderPtr);
            break;
        
        default:
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_SENDER_TAG," Bad CallBack return :%d \n", callbackReturn);
            break;
    }
    
}
