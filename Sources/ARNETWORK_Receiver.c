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
 *****************************************/

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
#include <libARNetworkAL/ARNETWORKAL_Frame.h>
#include "ARNETWORK_Buffer.h"
#include "ARNETWORK_DataDescriptor.h"
#include "ARNETWORK_Manager.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_IOBuffer.h"
#include "ARNETWORK_Sender.h"

#include "ARNETWORK_Receiver.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORK_RECEIVER_TAG "ARNETWORK_Receiver"

/*****************************************
 *
 *             private header:
 *
 *****************************************/

/**
 *  @brief copy the data received to the output buffer
 *  @param receiverPtr the pointer on the receiver
 *  @param outputBufferPtr[in] pointer on the output buffer
 *  @param framePtr[in] pointer on the frame received
 *  @return eARNETWORK_ERROR.
 *  @pre only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
 */
eARNETWORK_ERROR ARNETWORK_Receiver_CopyDataRecv (ARNETWORK_Receiver_t *receiverPtr, ARNETWORK_IOBuffer_t *outputBufferPtr, ARNETWORKAL_Frame_t *framePtr);

/*****************************************
 *
 *             implementation :
 *
 *****************************************/


ARNETWORK_Receiver_t* ARNETWORK_Receiver_New (ARNETWORKAL_Manager_t *networkALManager, unsigned int numberOfOutputBuff, ARNETWORK_IOBuffer_t **outputBufferPtrArr, ARNETWORK_IOBuffer_t **outputBufferPtrMap)
{
    /** -- Create a new receiver -- */

    /** local declarations */
    ARNETWORK_Receiver_t *receiverPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;

    /** Create the receiver */
    receiverPtr =  malloc (sizeof (ARNETWORK_Receiver_t));

    if (receiverPtr)
    {
        if(networkALManager != NULL)
        {
            receiverPtr->networkALManager = networkALManager;
        }
        else
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }

        if(error == ARNETWORK_OK)
        {
            receiverPtr->isAlive = 1;
            receiverPtr->senderPtr = NULL;

            receiverPtr->numberOfOutputBuff = numberOfOutputBuff;
            receiverPtr->outputBufferPtrArr = outputBufferPtrArr;

            receiverPtr->outputBufferPtrMap = outputBufferPtrMap;
        }

        /** delete the receiver if an error occurred */
        if (error != ARNETWORK_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG,"error: %s", ARNETWORK_Error_ToString (error));
            ARNETWORK_Receiver_Delete (&receiverPtr);
        }
    }

    return receiverPtr;
}

void ARNETWORK_Receiver_Delete (ARNETWORK_Receiver_t **receiverPtrAddr)
{
    /** -- Delete the Receiver -- */

    /** local declarations */
    ARNETWORK_Receiver_t *receiverPtr = NULL;

    if (receiverPtrAddr)
    {
        receiverPtr = *receiverPtrAddr;

        if (receiverPtr)
        {
            free (receiverPtr);
            receiverPtr = NULL;
        }
        *receiverPtrAddr = NULL;
    }
}

void* ARNETWORK_Receiver_ThreadRun (void *data)
{
    /** -- Manage the reception of the data on the Receiver' socket. -- */

    /** local declarations */
    ARNETWORK_Receiver_t *receiverPtr = data;
    ARNETWORKAL_Frame_t frame ;
    ARNETWORK_IOBuffer_t* outBufferPtrTemp = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    uint8_t ackSeqNumData = 0;
    struct timeval now;

    while (receiverPtr->isAlive)
    {
        /** wait a receipt */
        if (receiverPtr->networkALManager->receivingCallback(receiverPtr->networkALManager) == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
        {
            /** for each frame present in the receiver buffer */
            result = receiverPtr->networkALManager->popNextFrameCallback(receiverPtr->networkALManager, &frame);
            while ((result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT) && (error == ARNETWORK_OK))
            {
                /* Special handling of internal frames */
                if (frame.id < ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_MAX)
                {
                    switch (frame.id)
                    {
                    case ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING:
                        /* Ping, send the corresponding pong */
                    {
                        struct timeval dataTime;
                        memcpy (&dataTime, frame.dataPtr, sizeof (struct timeval));
                        ARNETWORK_Sender_SendPong (receiverPtr->senderPtr, &dataTime);
                    }
                    break;
                    case ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG:
                        /* Pong, tells the sender that we got a response */
                    {
                        struct timeval dataTime;
                        memcpy (&dataTime, frame.dataPtr, sizeof (struct timeval));
                        gettimeofday (&now, NULL);
                        ARNETWORK_Sender_GotPingAck (receiverPtr->senderPtr, &dataTime, &now);
                    }
                    break;
                    default:
                        /* Do nothing as we don't know how to handle it */
                        break;
                    }
                }

                /** management by the command type */
                switch (frame.type)
                {
                case ARNETWORKAL_FRAME_TYPE_ACK:

                    /** get the acknowledge sequence number from the data */
                    memcpy (&ackSeqNumData, frame.dataPtr, sizeof(uint8_t));
                    ARSAL_PRINT (ARSAL_PRINT_WARNING, ARNETWORK_RECEIVER_TAG, "- TYPE: ARNETWORKAL_FRAME_TYPE_ACK | SEQ:%d | ID:%d | SEQ ACK : %d", frame.seq, frame.id, ackSeqNumData);
                    /** transmit the acknowledgement to the sender */
                    error = ARNETWORK_Sender_AckReceived (receiverPtr->senderPtr, ARNETWORK_Manager_IDAckToIDInput (receiverPtr->networkALManager, frame.id), ackSeqNumData);
                    if (error != ARNETWORK_OK)
                    {
                        switch (error)
                        {
                        case ARNETWORK_ERROR_IOBUFFER_BAD_ACK:
                            ARSAL_PRINT (ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG, "Bad acknowledge, error: %s", ARNETWORK_Error_ToString (error));
                            break;

                        default:
                            ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG, "Acknowledge received, error: %s", ARNETWORK_Error_ToString (error));
                            break;
                        }
                    }
                    break;

                case ARNETWORKAL_FRAME_TYPE_DATA:
                    ARSAL_PRINT (ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG, "- TYPE: ARNETWORKAL_FRAME_TYPE_DATA | SEQ:%d | ID:%d", frame.seq, frame.id);

                    /** push the data received in the output buffer targeted */
                    outBufferPtrTemp = receiverPtr->outputBufferPtrMap[frame.id];

                    if (outBufferPtrTemp != NULL)
                    {
                        /** lock the IOBuffer */
                        error = ARNETWORK_IOBuffer_Lock(outBufferPtrTemp);
                        if(error == ARNETWORK_OK)
                        {
                            error = ARNETWORK_Receiver_CopyDataRecv(receiverPtr, outBufferPtrTemp, &frame);

                            /** unlock the IOBuffer */
                            ARNETWORK_IOBuffer_Unlock(outBufferPtrTemp);

                            if(error != ARNETWORK_OK)
                            {
                                ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG, "data received, error: %s", ARNETWORK_Error_ToString (error));
                            }
                        }
                    }
                    break;

                case ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY:
                    ARSAL_PRINT (ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG, "- TYPE: ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY | SEQ:%d | ID:%d", frame.seq, frame.id);

                    /** push the data received in the output buffer targeted */
                    outBufferPtrTemp = receiverPtr->outputBufferPtrMap[frame.id];

                    if (outBufferPtrTemp != NULL)
                    {
                        /** lock the IOBuffer */
                        error = ARNETWORK_IOBuffer_Lock(outBufferPtrTemp);
                        if(error == ARNETWORK_OK)
                        {
                            error = ARNETWORK_Receiver_CopyDataRecv(receiverPtr, outBufferPtrTemp, &frame);

                            /** unlock the IOBuffer */
                            ARNETWORK_IOBuffer_Unlock(outBufferPtrTemp);

                            if(error != ARNETWORK_OK)
                            {
                                ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG, "data received, error: %s", ARNETWORK_Error_ToString (error));
                            }
                        }
                    }
                    break;

                case ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK:
                    ARSAL_PRINT (ARSAL_PRINT_DEBUG, ARNETWORK_RECEIVER_TAG, "- TYPE: ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK | SEQ:%d | ID:%d", frame.seq, frame.id);

                    /**
                     * push the data received in the output buffer targeted,
                     * save the sequence of the command and return an acknowledgement
                     */
                    outBufferPtrTemp = receiverPtr->outputBufferPtrMap[frame.id];

                    if (outBufferPtrTemp != NULL)
                    {
                        /** lock the IOBuffer */
                        error = ARNETWORK_IOBuffer_Lock(outBufferPtrTemp);
                        if(error == ARNETWORK_OK)
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
                                    ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG, "data acknowledged received, error: %s", ARNETWORK_Error_ToString (error));
                                }
                            }

                            /** unlock the IOBuffer */
                            ARNETWORK_IOBuffer_Unlock(outBufferPtrTemp);

                            /** sending ack even if the seq is not correct */
                            error = ARNETWORK_Receiver_ReturnACK(receiverPtr, frame.id, frame.seq);
                            if( error != ARNETWORK_OK)
                            {
                                ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG, "ReturnACK, error: %d occurred \n", error);
                            }
                        }
                    }
                    break;

                default:
                    ARSAL_PRINT (ARSAL_PRINT_WARNING, ARNETWORK_RECEIVER_TAG, "!!! command type: %d not known  !!!", frame.type);
                    break;
                }

                /** get the next frame*/
                result = receiverPtr->networkALManager->popNextFrameCallback(receiverPtr->networkALManager, &frame);
            }
        }
    }

    return NULL;
}

void ARNETWORK_Receiver_Stop (ARNETWORK_Receiver_t *receiverPtr)
{
    /** -- stop the reception -- */
    receiverPtr->isAlive = 0;
}

eARNETWORK_ERROR ARNETWORK_Receiver_ReturnACK (ARNETWORK_Receiver_t *receiverPtr, int id, uint8_t seq)
{
    /** -- return an acknowledgement -- */
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_IOBuffer_t* ACKIOBufferPtr = receiverPtr->outputBufferPtrMap[ARNETWORK_Manager_IDOutputToIDAck (receiverPtr->networkALManager, id)];

    if (ACKIOBufferPtr != NULL)
    {
        error = ARNETWORK_IOBuffer_AddData (ACKIOBufferPtr, (uint8_t*) &seq, sizeof(seq), NULL, NULL, 1);
    }

    return error;
}

/*****************************************
 *
 *             private implementation:
 *
 *****************************************/
eARNETWORK_ERROR ARNETWORK_Receiver_CopyDataRecv (ARNETWORK_Receiver_t *receiverPtr, ARNETWORK_IOBuffer_t *outputBufferPtr, ARNETWORKAL_Frame_t *framePtr)
{
    /** -- copy the data received to the output buffer -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int semError = 0;
    int dataSize = 0;

    /** get the data size*/
    dataSize = framePtr->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);

    /** if the output buffer can copy the data */
    if (ARNETWORK_IOBuffer_CanCopyData (outputBufferPtr))
    {
        /** copy the data in the IOBuffer */
        error = ARNETWORK_IOBuffer_AddData (outputBufferPtr, framePtr->dataPtr, dataSize, NULL, NULL, 1);
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_RECEIVER_TAG, "Error: output buffer can't copy data");
    }

    if (error == ARNETWORK_OK)
    {
        /** post a semaphore to indicate data ready to be read */
        semError = ARSAL_Sem_Post (&(outputBufferPtr->outputSem));

        if (semError)
        {
            error = ARNETWORK_ERROR_SEMAPHORE;
        }
    }

    return error;
}
