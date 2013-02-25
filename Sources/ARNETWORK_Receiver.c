/**
 *  @file ARNETWORK_Receiver.c
 *  @brief manage the data received
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

#include <string.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Socket.h>
#include <libARSAL/ARSAL_Endianness.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_Frame.h>
#include "ARNETWORK_Buffer.h"
#include "ARNETWORK_VariableSizeData.h"
#include "ARNETWORK_Manager.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_IOBuffer.h"
#include "ARNETWORK_Sender.h"

#include "ARNETWORK_Receiver.h"

/*****************************************
 * 
 *             define :
 *
******************************************/

#define ARNETWORK_RECEIVER_TAG "ARNETWORK_Receiver"

/*****************************************
 * 
 *             private header:
 *
******************************************/

/**
 *  @brief get the next Frame of the receiving buffer
 *  @param receiverPtr the pointer on the receiver
 *  @param[out] framePtr pointer where store the frame
 *  @return eARNETWORK_ERROR
 *  @pre only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
**/
eARNETWORK_ERROR ARNETWORK_Receiver_GetFrame(ARNETWORK_Receiver_t *receiverPtr, ARNETWORK_Frame_t *framePtr);

/**
 *  @brief get a Frame from an address
 *  @param[in] orgFrameAddr address where get the frame
 *  @param[out] framePtr pointer where store the frame
 *  @return eARNETWORK_ERROR
**/
void ARNETWORK_Receiver_GetFrameFromAddr(uint8_t *orgFrameAddr, ARNETWORK_Frame_t *framePtr);

/**
 *  @brief copy the data received to the output buffer
 *  @param receiverPtr the pointer on the receiver
 *  @param outputBufferPtr[in] pointer on the output buffer
 *  @param framePtr[in] pointer on the frame received
 *  @return eARNETWORK_ERROR.
 *  @pre only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
**/
eARNETWORK_ERROR ARNETWORK_Receiver_CopyDataRecv(ARNETWORK_Receiver_t *receiverPtr, ARNETWORK_IOBuffer_t *outputBufferPtr, ARNETWORK_Frame_t *framePtr);

/**
 *  @brief call back used to free variable size data 
 *  @param[in] outputBufferID IOBuffer identifier of the IOBuffer is calling back
 *  @param[in] dataPtr pointer on the data
 *  @param[in] customData custom data
 *  @param[in] status status indicating the reason of the callback. eARNETWORK_MANAGER_CALLBACK_STATUS_Status type
 *  @return  eARNETWORK_MANAGER_CALLBACK_RETURN 
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
**/
eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_Receiver_FreeVariableSizeDataCallback(int outputBufferID, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

/*****************************************
 * 
 *             implementation :
 *
******************************************/


ARNETWORK_Receiver_t* ARNETWORK_Receiver_New(unsigned int recvBufferSize, unsigned int numberOfOutputBuff, ARNETWORK_IOBuffer_t **outputBufferPtrArr, ARNETWORK_IOBuffer_t **outputBufferPtrMap)
{    
    /** -- Create a new receiver -- */
    
    /** local declarations */
    ARNETWORK_Receiver_t *receiverPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    /** Create the receiver */
    receiverPtr =  malloc( sizeof(ARNETWORK_Receiver_t) );
    
    if(receiverPtr)
    {
        receiverPtr->isAlive = 1;
        receiverPtr->senderPtr = NULL;
        
        receiverPtr->numberOfOutputBuff = numberOfOutputBuff;
        receiverPtr->outpuBufferPtrArr = outputBufferPtrArr;
        
        receiverPtr->outputBufferPtrMap = outputBufferPtrMap;
        
        receiverPtr->receivingBufferPtr = ARNETWORK_Buffer_New(recvBufferSize,1);
        if(receiverPtr->receivingBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_NEW_BUFFER;
        }
    
        /** initialize the reading pointer to the start of the buffer  */
        receiverPtr->readingPointer = receiverPtr->receivingBufferPtr->startPtr;
    
        /** delete the receiver if an error occurred */
        if(error != ARNETWORK_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG,"error: %d occurred \n", error );
            ARNETWORK_Receiver_Delete(&receiverPtr);
        }
    }
    
    return receiverPtr;
}

void ARNETWORK_Receiver_Delete(ARNETWORK_Receiver_t **receiverPtrAddr)
{
    /** -- Delete the Receiver -- */
    
    /** local declarations */
    ARNETWORK_Receiver_t *receiverPtr = NULL;
    
    if(receiverPtrAddr)
    {
        receiverPtr = *receiverPtrAddr;
        
        if(receiverPtr)
        {
            ARNETWORK_Buffer_Delete( &(receiverPtr->receivingBufferPtr) );
    
            free(receiverPtr);
            receiverPtr = NULL;
        }
        *receiverPtrAddr = NULL;
    }
}

void* ARNETWORK_Receiver_ThreadRun(void *data)
{    
    /** -- Manage the reception of the data on the Receiver' socket. -- */
    
    /** local declarations */
    ARNETWORK_Receiver_t *receiverPtr = data;
    ARNETWORK_Frame_t frame ;
    ARNETWORK_IOBuffer_t* outBufferPtrTemp = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int ackSeqNumDAta = 0;
    
    while( receiverPtr->isAlive )
    {    
        /** wait a receipt */
        if( ARNETWORK_Receiver_Read( receiverPtr ) > 0 )
        {    
            /** for each frame present in the receiver buffer */ 
            error = ARNETWORK_Receiver_GetFrame(receiverPtr, &frame);
            while( error == ARNETWORK_OK ) 
            {    
                /** management by the command type */            
                switch (frame.type)
                {
                    case ARNETWORK_FRAME_TYPE_ACK:
                        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG," - TYPE: ARNETWORK_FRAME_TYPE_ACK | SEQ:%d | ID:%d \n", frame.seq, frame.ID);
                        
                        /** get the acknowledge sequence number from the data */
                        memcpy( &ackSeqNumDAta, (uint32_t*) frame.dataPtr, sizeof(uint32_t) );
                        
                        /** transmit the acknowledgement to the sender */
                        error = ARNETWORK_Sender_AckReceived(receiverPtr->senderPtr, ARNETWORK_Manager_IDAckToIDInput(frame.ID), ackSeqNumDAta);
                        if( error != ARNETWORK_OK )
                        {
                            switch(error)
                            {
                                case ARNETWORK_ERROR_IOBUFFER_BAD_ACK:
                                    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG,"Bad acknowledge, error: %d occurred \n", error);
                                    break;
                                
                                default:
                                    ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG,"acknowledge received, error: %d occurred \n", error);
                                    break;
                            }
                        }
                        
                        break;
                    
                    case ARNETWORK_FRAME_TYPE_DATA:
                        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG," - TYPE: ARNETWORK_FRAME_TYPE_DATA | SEQ:%d | ID:%d \n", frame.seq, frame.ID);
                        
                        /** push the data received in the output buffer targeted */
                        outBufferPtrTemp = receiverPtr->outputBufferPtrMap[frame.ID]; 
                        
                        if(outBufferPtrTemp != NULL)
                        {
                            error = ARNETWORK_Receiver_CopyDataRecv(receiverPtr, outBufferPtrTemp, &frame);
                            
                            if(error != ARNETWORK_OK)
                            {
                                ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG,"data received, error: %d occurred \n", error);
                            }
                            
                        }
                        
                        break;
                    
                    case ARNETWORK_FRAME_TYPE_DATA_WITH_ACK:
                        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG," - TYPE: ARNETWORK_FRAME_TYPE_DATA_WITH_ACK | SEQ:%d | ID:%d \n", frame.seq, frame.ID);
                        
                        /** 
                         * push the data received in the output buffer targeted, 
                         * save the sequence of the command and return an acknowledgement
                        **/
                        outBufferPtrTemp = receiverPtr->outputBufferPtrMap[frame.ID];

                        if(outBufferPtrTemp != NULL)
                        {
                            /** OutBuffer->seqWaitAck used to save the last seq */
                            if(frame.seq != outBufferPtrTemp->seqWaitAck)
                            {        
                                error = ARNETWORK_Receiver_CopyDataRecv( receiverPtr, outBufferPtrTemp, &frame);
                                if( error == ARNETWORK_OK)
                                {
                                    outBufferPtrTemp->seqWaitAck = frame.seq;
                                }
                                else
                                {
                                    ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG,"data acknowledgeed received, error: %d occurred \n", error);
                                }
                            }
                            
                            /** sending ack even if the seq is not correct */
                            ARNETWORK_Receiver_ReturnACK(receiverPtr, frame.ID, frame.seq);
                            
                        }    
                                                
                        break;
                    
                    default:
                        ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORK_RECEIVER_TAG," !!! command type: %d not known  !!! \n", frame.type);
                        break;
                }
                /** get the next frame*/
                error = ARNETWORK_Receiver_GetFrame(receiverPtr, &frame);
            }
            ARNETWORK_Buffer_Clean(receiverPtr->receivingBufferPtr);
        }
    }

    ARSAL_Socket_Close(receiverPtr->socket);
       
    return NULL;
}

void ARNETWORK_Receiver_Stop(ARNETWORK_Receiver_t *receiverPtr)
{
    /** -- stop the reception -- */
    receiverPtr->isAlive = 0;
}


void ARNETWORK_Receiver_ReturnACK(ARNETWORK_Receiver_t *receiverPtr, int ID, int seq)
{
    /** -- return an acknowledgement -- */
    
    /** local declarations */
    ARNETWORK_IOBuffer_t* pBufferASK = receiverPtr->outputBufferPtrMap[ ARNETWORK_Manager_IDOutputToIDAck(ID) ];
                                                                
    if(pBufferASK != NULL)
    {
        ARNETWORK_RingBuffer_PushBack( pBufferASK->bufferPtr, (uint8_t*) &seq );
    }
}

int ARNETWORK_Receiver_Read(ARNETWORK_Receiver_t *receiverPtr)
{
    /** -- receiving data present on the socket -- */
    
    /** local declarations */
    int readDataSize =  ARSAL_Socket_Recv(receiverPtr->socket, receiverPtr->receivingBufferPtr->startPtr, receiverPtr->receivingBufferPtr->numberOfCell, 0);
    
    if(readDataSize > 0)
    {
        receiverPtr->receivingBufferPtr->frontPtr = receiverPtr->receivingBufferPtr->startPtr + readDataSize;
    }
    
    return readDataSize;
}

eARNETWORK_ERROR ARNETWORK_Receiver_Bind(ARNETWORK_Receiver_t *receiverPtr, unsigned short port, int timeoutSec)
{
    /** -- receiving data present on the socket -- */
    
    /** local declarations */
    struct timeval timeout;  
    struct sockaddr_in recvSin;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int errorBind = 0;
    
    /** socket initialization */
    recvSin.sin_addr.s_addr = htonl(INADDR_ANY);   
    recvSin.sin_family = AF_INET;
    recvSin.sin_port = htons(port);
    
    receiverPtr->socket = ARSAL_Socket_Create( AF_INET, SOCK_DGRAM, 0 );
    
    /** set the socket timeout */
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0; 
    ARSAL_Socket_Setsockopt(receiverPtr->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    
    errorBind = ARSAL_Socket_Bind(receiverPtr->socket, (struct sockaddr*)&recvSin, sizeof(recvSin));
    
    if( errorBind !=0 )
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

/*****************************************
 * 
 *             private implementation:
 *
******************************************/

eARNETWORK_ERROR ARNETWORK_Receiver_GetFrame( ARNETWORK_Receiver_t *receiverPtr, ARNETWORK_Frame_t *framePtr )
{
    /** -- get a Frame of the receiving buffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    /** if the receiving buffer not contain enough data for the frame head*/
    if( receiverPtr->readingPointer > receiverPtr->receivingBufferPtr->frontPtr - offsetof(ARNETWORK_Frame_t, dataPtr ) )
    {
        if( receiverPtr->readingPointer == receiverPtr->receivingBufferPtr->frontPtr)
        {
            error = ARNETWORK_ERROR_RECEIVER_BUFFER_END;
        }
        else
        {
            error = ARNETWORK_ERROR_RECEIVER_BAD_FRAME;
        }
    }
    
    if( error == ARNETWORK_OK )
    {
        /** get the frame from the reading address */
        ARNETWORK_Receiver_GetFrameFromAddr( receiverPtr->readingPointer, framePtr );
        
        /** if the receiving buffer not contain enough data for the full frame */
        if ( receiverPtr->readingPointer > receiverPtr->receivingBufferPtr->frontPtr - framePtr->size )
        {
            error = ARNETWORK_ERROR_RECEIVER_BAD_FRAME;
        }
    }
    
    if( error == ARNETWORK_OK )
    {
        /** offset the readingPointer on the next frame */
        receiverPtr->readingPointer = ( receiverPtr->readingPointer + framePtr->size );
    }
    else
    {
        /** reset the reading pointer to the start of the buffer */
        receiverPtr->readingPointer = receiverPtr->receivingBufferPtr->startPtr;
        
        /** reset pFrame */
        framePtr->type = ARNETWORK_FRAME_TYPE_UNINITIALIZED;
        framePtr->ID = 0;
        framePtr->seq = 0;
        framePtr->size = 0;
        framePtr->dataPtr = NULL;
    }
    
    return error;
}

void ARNETWORK_Receiver_GetFrameFromAddr(uint8_t *orgFrameAddr, ARNETWORK_Frame_t *framePtr)
{
    /** local declarations */
    uint8_t *tempPtr = orgFrameAddr; /**< temporary pointer */
    
    /** get type */
    memcpy( &(framePtr->type), tempPtr, sizeof(uint8_t) );
    tempPtr +=  sizeof(uint8_t) ;
    
    /** get id */
    memcpy( &(framePtr->ID), tempPtr, sizeof(uint8_t) );
    tempPtr +=  sizeof(uint8_t);
    
    /** get seq */
    memcpy( &(framePtr->seq), tempPtr, sizeof(uint32_t));
    tempPtr +=  sizeof(uint32_t);
    /** convert the endianness */
    framePtr->seq = dtohl( framePtr->seq );
    
    /** get size */
    memcpy( &(framePtr->size), tempPtr, sizeof(uint32_t));
    tempPtr +=  sizeof(uint32_t);
    /** convert the endianness */
    framePtr->size = dtohl( framePtr->size );
    
    /** get data address */
    framePtr->dataPtr = tempPtr;
}

eARNETWORK_ERROR ARNETWORK_Receiver_CopyDataRecv(ARNETWORK_Receiver_t *receiverPtr, ARNETWORK_IOBuffer_t *outputBufferPtr, ARNETWORK_Frame_t *framePtr)
{
    /** -- copy the data received to the output buffer -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_VariableSizeData_t variableSizeDataTemp;
    ARNETWORK_VariableSizeData_t variableSizeDataOverwritten;
    int semError = 0;
    int bufferFreeCellNb = 0;
    
    /** if the IOBuffer using variable size data */
    if( outputBufferPtr->isUsingVariableSizeData )
    {
        bufferFreeCellNb = ARNETWORK_RingBuffer_GetFreeCellNumber(outputBufferPtr->bufferPtr);
        
        /** if the buffer is not full or it is overwriting */
        if( outputBufferPtr->bufferPtr->isOverwriting == 1 ||
            bufferFreeCellNb > 0 )
        {
            /** allocate variable size data */
            variableSizeDataTemp.dataSize = framePtr->size - offsetof(ARNETWORK_Frame_t, dataPtr);
            variableSizeDataTemp.callback = &(ARNETWORK_Receiver_FreeVariableSizeDataCallback);
            
            variableSizeDataTemp.dataPtr = malloc(variableSizeDataTemp.dataSize);
            if(variableSizeDataTemp.dataPtr == NULL)
            {
                error = ARNETWORK_ERROR_ALLOC;
            }
            
            if(error == ARNETWORK_OK)
            {
                /** copy the data received in the space allocated */
                memcpy(variableSizeDataTemp.dataPtr, framePtr->dataPtr, variableSizeDataTemp.dataSize);
                
                if( bufferFreeCellNb == 0)
                {
                    /** if the buffer is full, free the variable size data lost by the overwriting */
                    
                    /** get the variable size data Overwritten */
                    error = ARNETWORK_RingBuffer_PopFront( outputBufferPtr->bufferPtr, (uint8_t*) &variableSizeDataOverwritten );
                    /** free the variable size data Overwritten */
                    variableSizeDataOverwritten.callback( outputBufferPtr->ID, variableSizeDataOverwritten.dataPtr, variableSizeDataOverwritten.customData, ARNETWORK_MANAGER_CALLBACK_STATUS_FREE);
                }
            }
            
            if(error == ARNETWORK_OK)
            {
                /** push the variableSizeDataTemp in the output buffer targeted */
                error = ARNETWORK_RingBuffer_PushBack( outputBufferPtr->bufferPtr, (uint8_t*) &variableSizeDataTemp );
            }
        }
        else
        {
            error = ARNETWORK_ERROR_BUFFER_SIZE;
        }
    }
    else
    {
        /** push the data received in the output buffer targeted */
        error = ARNETWORK_RingBuffer_PushBack( outputBufferPtr->bufferPtr, framePtr->dataPtr );
    }
    
    if(error == ARNETWORK_OK)
    {
        /** post a semaphore to indicate data ready to be read */
        semError = ARSAL_Sem_Post( &(outputBufferPtr->outputSem) );
        
        if( semError )
        {
            error = ARNETWORK_ERROR_SEMAPHORE;
        }
    }
    
    return error;
}

eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_Receiver_FreeVariableSizeDataCallback(int outputBufferID, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status)
{
    /** call back used to free variable size data */
    
    free(dataPtr);
    
    return ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
}
