/**
 *  @file receiver.c
 *  @brief manage the data received, used by libNetwork/network
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

#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/sem.h>
#include <libSAL/socket.h>
#include <libSAL/endianness.h>

#include <libNetwork/status.h>
#include <libNetwork/frame.h>
#include "buffer.h"
#include <libNetwork/deportedData.h>
#include "ioBuffer.h"
#include "sender.h"

#include "receiver.h"

/*****************************************
 * 
 *             define :
 *
******************************************/
#define TAG "Receiver"

/*****************************************
 * 
 *             private header:
 *
******************************************/

/**
 *  @brief get the next Frame of the receiving buffer
 *  @param pReceiver the pointer on the receiver
 *  @param[out] pNextFrame pointer where store the frame
 *  @return eNETWORK_Error
 *  @pre only call by NETWORK_RunSendingThread()
 *  @see NETWORK_RunSendingThread()
**/
eNETWORK_Error NETWORK_ReceiverGetFrame( network_receiver_t* pReceiver, network_frame_t* pFrame );

/**
 *  @brief get a Frame from an address
 *  @param[in] orgPointer address where get the frame
 *  @param[out] pFrame pointer where store the frame
 *  @return eNETWORK_Error
**/
void NETWORK_getFrameFromAddr( uint8_t* orgPointer, network_frame_t* pFrame );

/**
 *  @brief copy the data received to the output buffer
 *  @param pReceiver the pointer on the receiver
 *  @param pOutBuffer[in] pointer on the output buffer
 *  @param pFrame[in] pointer on the frame received
 *  @return error equal to NETWORK_OK if the Bind if successful otherwise equal to error of eNETWORK_Error.
 *  @pre only call by NETWORK_RunSendingThread()
 *  @see NETWORK_RunSendingThread()
**/
eNETWORK_Error NETWORK_ReceiverCopyDataRecv( network_receiver_t* pReceiver,
                                               network_ioBuffer_t* pOutBuffer,
                                               network_frame_t* pFrame );
                                   
/**
 *  @brief call back use to free deported data 
 *  @param[in] OutBufferId IoBuffer identifier of the IoBuffer is calling back
 *  @param[in] pData pointer on the data
 *  @param[in] status status indicating the reason of the callback. eNETWORK_CALLBACK_STATUS_Status type
 *  @return  eNETWORK_CALLBACK_RETURN 
 *  @see eNETWORK_CALLBACK_STATUS
**/
eNETWORK_CALLBACK_RETURN NETWORK_freeDeportedData(int OutBufferId, 
                                                      uint8_t* pData, 
                                                      void* pCustomData, 
                                                      eNETWORK_CALLBACK_STATUS status);

/*****************************************
 * 
 *             implementation :
 *
******************************************/


network_receiver_t* NETWORK_NewReceiver( unsigned int recvBufferSize, 
                                            unsigned int numOfOutputBuff,
                                            network_ioBuffer_t** pptab_output,
                                            network_ioBuffer_t** ppTabOutputMap)
{    
    /** -- Create a new receiver -- */
    
    /** local declarations */
    network_receiver_t* pReceiver = NULL;
    eNETWORK_Error error = NETWORK_OK;
    
    /** Create the receiver */
    pReceiver =  malloc( sizeof(network_receiver_t) );
    
    if(pReceiver)
    {
        pReceiver->isAlive = 1;
        pReceiver->pSender = NULL;
        
        pReceiver->numOfOutputBuff = numOfOutputBuff;
        pReceiver->pptab_outputBuffer = pptab_output;
        
        pReceiver->ppTabOutputMap = ppTabOutputMap;
        
        pReceiver->pRecvBuffer = NETWORK_NewBuffer(recvBufferSize,1);
        if(pReceiver->pRecvBuffer == NULL)
        {
            error = NETWORK_ERROR_NEW_BUFFER;
        }
    
        /** initialize the reading pointer to the start of the buffer  */
        pReceiver->readingPointer = pReceiver->pRecvBuffer->pStart;
    
        /** delete the receiver if an error occurred */
        if(error != NETWORK_OK)
        {
            SAL_PRINT(PRINT_ERROR, TAG,"error: %d occurred \n", error );
            NETWORK_DeleteReceiver(&pReceiver);
        }
    }
    
    return pReceiver;
}

void NETWORK_DeleteReceiver(network_receiver_t** ppReceiver)
{
    /** -- Delete the Receiver -- */
    
    /** local declarations */
    network_receiver_t* pReceiver = NULL;
    
    if(ppReceiver)
    {
        pReceiver = *ppReceiver;
        
        if(pReceiver)
        {
            NETWORK_DeleteBuffer( &(pReceiver->pRecvBuffer) );
    
            free(pReceiver);
            pReceiver = NULL;
        }
        *ppReceiver = NULL;
    }
}

void* NETWORK_RunReceivingThread(void* data)
{    
    /** -- Manage the reception of the data on the Receiver' socket. -- */
    
    /** local declarations */
    network_receiver_t* pReceiver = data;
    network_frame_t frame ;
    network_ioBuffer_t* pOutBufferTemp = NULL;
    eNETWORK_Error error = NETWORK_OK;
    int ackSeqNumDAta = 0;
    
    while( pReceiver->isAlive )
    {    
        /** wait a receipt */
        if( NETWORK_ReceiverRead( pReceiver ) > 0 )
        {    
            /** for each frame present in the receiver buffer */ 
            error = NETWORK_ReceiverGetFrame(pReceiver, &frame);
            while( error == NETWORK_OK ) 
            {    
                /** management by the command type */            
                switch (/*pFrame->type*/ frame.type)
                {
                    case NETWORK_FRAME_TYPE_ACK:
                        SAL_PRINT(PRINT_DEBUG, TAG," - TYPE: NETWORK_FRAME_TYPE_ACK | SEQ:%d | ID:%d \n",
                                                    frame.seq, frame.id);
                                                
                        /** get the acknowledge sequence number from the data */
                        memcpy( &ackSeqNumDAta, (uint32_t*) frame.pData, sizeof(uint32_t) );
                        
                        /** transmit the acknowledgement to the sender */
                        error = NETWORK_SenderAckReceived( pReceiver->pSender,
                                                           idAckToIdInput(frame.id),
                                                           ackSeqNumDAta );
                        if( error != NETWORK_OK )
                        {
                            switch(error)
                            {
                                case NETWORK_IOBUFFER_ERROR_BAD_ACK:
                                    SAL_PRINT(PRINT_DEBUG, TAG,"Bad acknowledge, error: %d occurred \n", error);
                                break;
                                
                                default:
                                    SAL_PRINT(PRINT_ERROR, TAG,"acknowledge received, error: %d occurred \n", error);
                                break;
                            }
                        }
                        
                    break;
                    
                    case NETWORK_FRAME_TYPE_DATA:
                        SAL_PRINT(PRINT_DEBUG, TAG," - TYPE: NETWORK_FRAME_TYPE_DATA | SEQ:%d | ID:%d \n",
                                                   frame.seq, frame.id);
                        
                        /** push the data received in the output buffer targeted */
                        pOutBufferTemp = pReceiver->ppTabOutputMap[ frame.id ]; 
                        
                        if(pOutBufferTemp != NULL)
                        {
                            error = NETWORK_ReceiverCopyDataRecv( pReceiver, pOutBufferTemp, &frame );
                            
                            if( error != NETWORK_OK )
                            {
                                SAL_PRINT(PRINT_ERROR, TAG,"data received, error: %d occurred \n", error);
                            }
                            
                        }                            
                    break;
                    
                    case NETWORK_FRAME_TYPE_DATA_WITH_ACK:
                        SAL_PRINT(PRINT_DEBUG, TAG," - TYPE: NETWORK_FRAME_TYPE_DATA_WITH_ACK | SEQ:%d | ID:%d \n", 
                                                    frame.seq, frame.id);
                        
                        /** 
                         * push the data received in the output buffer targeted, 
                         * save the sequence of the command and return an acknowledgement
                        **/
                        pOutBufferTemp = pReceiver->ppTabOutputMap[ frame.id ];

                        if(pOutBufferTemp != NULL)
                        {
                            /** OutBuffer->seqWaitAck used to save the last seq */
                            if( frame.seq != pOutBufferTemp->seqWaitAck )
                            {        
                                error = NETWORK_ReceiverCopyDataRecv( pReceiver, 
                                                                      pOutBufferTemp,
                                                                      &frame);
                                if( error == NETWORK_OK)
                                {
                                    pOutBufferTemp->seqWaitAck = frame.seq;
                                }
                                else
                                {
                                    SAL_PRINT(PRINT_ERROR, TAG,"data acknowledgeed received, error: %d occurred \n", error);
                                }
                            }
                            
                            /** sending ack even if the seq is not correct */
                            NETWORK_ReturnACK(pReceiver, frame.id, frame.seq);
                            
                        }    
                                                
                    break;
                    
                    default:
                        SAL_PRINT(PRINT_WARNING, TAG," !!! command type: %d not known  !!! \n", frame.type); //!!!!
                    break;
                }
                /** get the next frame*/
                error = NETWORK_ReceiverGetFrame(pReceiver, &frame);
            }
            NETWORK_BufferClean(pReceiver->pRecvBuffer);
        }
    }

    sal_close(pReceiver->socket);
       
    return NULL;
}

void NETWORK_StopReceiver(network_receiver_t* pReceiver)
{
    /** -- stop the reception -- */
    pReceiver->isAlive = 0;
}


void NETWORK_ReturnACK(network_receiver_t* pReceiver, int id, int seq)
{
    /** -- return an acknowledgement -- */
    
    /** local declarations */
    network_ioBuffer_t* pBufferASK = pReceiver->ppTabOutputMap[ idOutputToIdAck(id) ];
                                                                
    if(pBufferASK != NULL)
    {
        NETWORK_RingBuffPushBack( pBufferASK->pBuffer, (uint8_t*) &seq );
    }
}

int NETWORK_ReceiverRead(network_receiver_t* pReceiver)
{
    /** -- receiving data present on the socket -- */
    
    /** local declarations */
    int readDataSize =  sal_recv( pReceiver->socket, pReceiver->pRecvBuffer->pStart,
                                  pReceiver->pRecvBuffer->numberOfCell, 0);
    if(readDataSize > 0)
    {
        pReceiver->pRecvBuffer->pFront = pReceiver->pRecvBuffer->pStart + readDataSize;
    }
    
    return readDataSize;
}

eNETWORK_Error NETWORK_ReceiverBind( network_receiver_t* pReceiver, unsigned short port, int timeoutSec )
{
    /** -- receiving data present on the socket -- */
    
    /** local declarations */
    struct timeval timeout;  
    struct sockaddr_in recvSin;
    eNETWORK_Error error = NETWORK_OK;
    int errorBind = 0;
    
    /** socket initialization */
    recvSin.sin_addr.s_addr = htonl(INADDR_ANY);   
    recvSin.sin_family = AF_INET;
    recvSin.sin_port = htons(port);
    
    pReceiver->socket = sal_socket( AF_INET, SOCK_DGRAM, 0 );
    
    /** set the socket timeout */
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0; 
    sal_setsockopt(pReceiver->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    
    errorBind = sal_bind(pReceiver->socket, (struct sockaddr*)&recvSin, sizeof(recvSin));
    
    if( errorBind !=0 )
    {
        switch( errno )
        {
            case EACCES:
                error = NETWORK_SCOCKET_ERROR_PERMISSION_DENIED;
            break;
            
            default:
                error = NETWORK_SCOCKET_ERROR;
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

eNETWORK_Error NETWORK_ReceiverGetFrame( network_receiver_t* pReceiver, network_frame_t* pFrame )
{
    /** -- get a Frame of the receiving buffer -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    
    /** if the receiving buffer not contain enough data for the frame head*/
    if( pReceiver->readingPointer > pReceiver->pRecvBuffer->pFront - offsetof(network_frame_t, pData ) )
    {
        if( pReceiver->readingPointer == pReceiver->pRecvBuffer->pFront)
        {
            error = NETWORK_RECEIVER_ERROR_BUFFER_END;
        }
        else
        {
            error = NETWORK_RECEIVER_ERROR_BAD_FRAME;
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** get the frame from the reading address */
        NETWORK_getFrameFromAddr( pReceiver->readingPointer, pFrame );
        
        /** if the receiving buffer not contain enough data for the full frame */
        if ( pReceiver->readingPointer > pReceiver->pRecvBuffer->pFront - pFrame->size )
        {
            error = NETWORK_RECEIVER_ERROR_BAD_FRAME;
        }
    }
    
    if( error == NETWORK_OK )
    {
        /** offset the readingPointer on the next frame */
        pReceiver->readingPointer = ( pReceiver->readingPointer + pFrame->size );
    }
    else
    {
        /** reset the reading pointer to the start of the buffer */
        pReceiver->readingPointer = pReceiver->pRecvBuffer->pStart;
        
        /** reset pFrame */
        pFrame->type = NETWORK_FRAME_TYPE_UNINITIALIZED;
        pFrame->id = 0;
        pFrame->seq = 0;
        pFrame->size = 0;
        pFrame->pData = NULL;
    }
                
    return error;
}

void NETWORK_getFrameFromAddr( uint8_t* orgPointer, network_frame_t* pFrame )
{
    /** local declarations */
    uint8_t* tempPointer = orgPointer;
    
    /** get type */
    memcpy( &(pFrame->type), tempPointer, sizeof(uint8_t) );
    tempPointer +=  sizeof(uint8_t) ;
    
    /** get id */
    memcpy( &(pFrame->id), tempPointer, sizeof(uint8_t) );
    tempPointer +=  sizeof(uint8_t);
    
    /** get seq */
    memcpy( &(pFrame->seq), tempPointer, sizeof(uint32_t));
    tempPointer +=  sizeof(uint32_t);
    /** convert the endianness */
    pFrame->seq = dtohl( pFrame->seq );
    
    /** get size */
    memcpy( &(pFrame->size), tempPointer, sizeof(uint32_t));
    tempPointer +=  sizeof(uint32_t);
    /** convert the endianness */
    pFrame->size = dtohl( pFrame->size );
    
    /** data address */
    pFrame->pData = tempPointer;
}

eNETWORK_Error NETWORK_ReceiverCopyDataRecv( network_receiver_t* pReceiver,
                                               network_ioBuffer_t* pOutBuffer,
                                               network_frame_t* pFrame )
{
    /** -- copy the data received to the output buffer -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    network_DeportedData_t deportedDataTemp;
    network_DeportedData_t deportedDataOverwritten;
    int semError = 0;
    int bufferFreeCellNb = 0;
    
    /** if the ioBuffer used deportedData */
    if( pOutBuffer->deportedData )
    {
        bufferFreeCellNb = NETWORK_RingBuffGetFreeCellNb(pOutBuffer->pBuffer);
        
        /** if the buffer is not full or it is overwriting */
        if( pOutBuffer->pBuffer->isOverwriting == 1 ||
            bufferFreeCellNb > 0 )
        {
            /** alloc data deported */
            deportedDataTemp.dataSize = pFrame->size - offsetof( network_frame_t, pData );
            deportedDataTemp.callback = &(NETWORK_freeDeportedData);
            
            deportedDataTemp.pData = malloc( deportedDataTemp.dataSize );
            if(deportedDataTemp.pData == NULL)
            {
                error = NETWORK_ERROR_ALLOC;
            }
            
            if(error == NETWORK_OK)
            {
                /** copy the data received in the space allocated */
                memcpy(deportedDataTemp.pData, pFrame->pData, deportedDataTemp.dataSize);
                
                if( bufferFreeCellNb == 0)
                {
                    /** if the buffer is full, free the data deported lost by the overwriting */
                    
                    /** get the deported Data Overwritten */
                    error = NETWORK_RingBuffPopFront( pOutBuffer->pBuffer, (uint8_t*) &deportedDataOverwritten );
                    /** free the deported Data Overwritten */
                    deportedDataOverwritten.callback( pOutBuffer->id, 
                                                      deportedDataOverwritten.pData,
                                                      deportedDataOverwritten.pCustomData, 
                                                      NETWORK_CALLBACK_STATUS_FREE);
                }
            }
            
            if(error == NETWORK_OK)
            {
                /** push the deportedDataTemp in the output buffer targeted */
                error = NETWORK_RingBuffPushBack( pOutBuffer->pBuffer, (uint8_t*) &deportedDataTemp );
            }
        }
        else
        {
            error = NETWORK_ERROR_BUFFER_SIZE;
        }
    }
    else
    {
        /** push the data received in the output buffer targeted */
        error = NETWORK_RingBuffPushBack( pOutBuffer->pBuffer, pFrame->pData );
    }
    
    if(error == NETWORK_OK)
    {
        /** post a semaphore to indicate data ready to be read */
        semError = sal_sem_post( &(pOutBuffer->outputSem) );
        
        if( semError )
        {
            error = NETWORK_ERROR_SEMAPHORE;
        }
    }
    
    return error;
}

eNETWORK_CALLBACK_RETURN NETWORK_freeDeportedData(int OutBufferId,
                                                      uint8_t* pData, 
                                                      void* pCustomData, 
                                                      eNETWORK_CALLBACK_STATUS status)
{
    /** call back use to free deported data */
    
    free(pData);
    
    return NETWORK_CALLBACK_RETURN_DEFAULT;
}
