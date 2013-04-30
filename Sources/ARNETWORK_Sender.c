/**
 *  @file ARNETWORK_Sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

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
#include "ARNETWORK_DataDescriptor.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_Manager.h"
#include "ARNETWORK_IOBuffer.h"

#include "ARNETWORK_Sender.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORK_SENDER_TAG "ARNETWORK_Sender"
#define ARNETWORK_SENDER_MILLISECOND 1
#define ARNETWORK_SENDER_FIRST_SEQ 1 /**< first sequence number sent */

/*****************************************
 *
 *             private header:
 *
 *****************************************/

/**
 *  @brief send the data
 *  @param senderPtr the pointer on the Sender
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
 */
void ARNETWORK_Sender_Send (ARNETWORK_Sender_t *senderPtr);

/**
 *  @brief add data to the sender buffer and callback with sent status
 *  @param senderPtr the pointer on the Sender
 *  @param inputBufferPtr Pointer on the input buffer
 *  @param seqNum sequence number
 *  @return error eARNETWORK_ERROR
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
 */
eARNETWORK_ERROR ARNETWORK_Sender_AddToBuffer (ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *inputBufferPtr, int seqNum);

/**
 *  @brief call the Callback this timeout status
 *  @param senderPtr the pointer on the Sender
 *  @param inputBufferPtr Pointer on the input buffer
 *  @return eARNETWORK_MANAGER_CALLBACK_RETURN
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
 */
eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_Sender_TimeOutCallback (ARNETWORK_Sender_t *senderPtr, const ARNETWORK_IOBuffer_t *inputBufferPtr);

/**
 *  @brief manage the return of the callback
 *  @param senderPtr the pointer on the Sender
 *  @param[in] inputBufferPtr Pointer on the input buffer
 *  @param[in] callbackReturn return of the callback
 *  @note only call by ARNETWORK_Sender_ThreadRun()
 *  @see ARNETWORK_Sender_ThreadRun()
 */
void ARNETWORK_Sender_ManageTimeOut (ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *inputBufferPtr, eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn);



void ARNETWORK_Sender_ManageIOBufferIsInRemovingStatus(ARNETWORK_IOBuffer_t *inputBufferPtr, eARNETWORK_MANAGER_CALLBACK_STATUS callbackStatus);

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

ARNETWORK_Sender_t* ARNETWORK_Sender_New (unsigned int sendingBufferSize, unsigned int numberOfInputBuffer, ARNETWORK_IOBuffer_t **inputBufferPtrArr, unsigned int numberOfInternalInputBuffer, ARNETWORK_IOBuffer_t **internalInputBufferPtrArr, ARNETWORK_IOBuffer_t **inputBufferPtrMap)
{
    /** -- Create a new sender -- */

    /** local declarations */
    ARNETWORK_Sender_t* senderPtr =  NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;

    /** Create the sender */
    senderPtr =  malloc (sizeof (ARNETWORK_Sender_t));

    if (senderPtr)
    {
        senderPtr->isAlive = 1;
        senderPtr->numberOfInputBuff = numberOfInputBuffer;
        senderPtr->inputBufferPtrArr = inputBufferPtrArr;
        senderPtr->numberOfInternalInputBuff = numberOfInternalInputBuffer;
        senderPtr->internalInputBufferPtrArr = internalInputBufferPtrArr;
        senderPtr->inputBufferPtrMap = inputBufferPtrMap;
        gettimeofday (&(senderPtr->pingStartTime), NULL);

        /** Create the Sending buffer */
        senderPtr->sendingBufferPtr = ARNETWORK_Buffer_New (sendingBufferSize, 1);

        if (senderPtr->sendingBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_NEW_BUFFER;
        }

        /* Create the mutex/condition */
        if ( (error == ARNETWORK_OK) &&
             (ARSAL_Mutex_Init (&(senderPtr->nextSendMutex)) != 0))
        {
            error = ARNETWORK_ERROR_NEW_BUFFER;
        }
        if ( (error == ARNETWORK_OK) &&
             (ARSAL_Cond_Init (&(senderPtr->nextSendCond)) != 0))
        {
            error = ARNETWORK_ERROR_NEW_BUFFER;
        }
        if ( (error == ARNETWORK_OK) &&
             (ARSAL_Mutex_Init (&(senderPtr->pingMutex)) != 0))
        {
            error = ARNETWORK_ERROR_NEW_BUFFER;
        }

        /** delete the sender if an error occurred */
        if (error != ARNETWORK_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_SENDER_TAG, "error: %s", ARNETWORK_Error_ToString (error));
            ARNETWORK_Sender_Delete (&senderPtr);
        }

        senderPtr->seq = ARNETWORK_SENDER_FIRST_SEQ;
    }

    return senderPtr;
}

void ARNETWORK_Sender_Delete (ARNETWORK_Sender_t **senderPtrAddr)
{
    /** -- Delete the sender -- */

    /** local declarations */
    ARNETWORK_Sender_t *senderPtr = NULL;

    if (senderPtrAddr != NULL)
    {
        senderPtr = *senderPtrAddr;

        if (senderPtr != NULL)
        {
            ARSAL_Cond_Destroy (&(senderPtr->nextSendCond));
            ARSAL_Mutex_Destroy (&(senderPtr->nextSendMutex));
            ARSAL_Mutex_Destroy (&(senderPtr->pingMutex));
            ARNETWORK_Buffer_Delete (&(senderPtr->sendingBufferPtr));

            free (senderPtr);
            senderPtr = NULL;
        }
        *senderPtrAddr = NULL;
    }
}

void* ARNETWORK_Sender_ThreadRun (void* data)
{
    /** -- Manage the sending of the data on the sender' socket -- */

    /** local declarations */
    ARNETWORK_Sender_t *senderPtr = data;
    int inputBufferIndex = 0;
    ARNETWORK_IOBuffer_t *inputBufferPtrTemp = NULL; /**< pointer of the input buffer in processing */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int mustWait = 1;
    struct timeval now;
    int timeDiffMs;

    while (senderPtr->isAlive)
    {
        mustWait = 1;
        for (inputBufferIndex = 0; inputBufferIndex < senderPtr->numberOfInputBuff && mustWait == 1; ++inputBufferIndex)
        {
            inputBufferPtrTemp = senderPtr->inputBufferPtrArr[inputBufferIndex];
            if (inputBufferPtrTemp->dataType == ARNETWORK_FRAME_TYPE_DATA_LOW_LATENCY)
            {
                error = ARNETWORK_IOBuffer_Lock(inputBufferPtrTemp);
                if ((error == ARNETWORK_OK) &&
                    (!ARNETWORK_RingBuffer_IsEmpty (inputBufferPtrTemp->dataDescriptorRBufferPtr)))
                {
                    mustWait = 0;
                }
                ARNETWORK_IOBuffer_Unlock(inputBufferPtrTemp);
            }
        }
        if (mustWait == 1)
        {
            ARSAL_Mutex_Lock (&(senderPtr->nextSendMutex));
            ARSAL_Cond_Timedwait (&(senderPtr->nextSendCond), &(senderPtr->nextSendMutex), ARNETWORK_SENDER_MILLISECOND);
            ARSAL_Mutex_Unlock (&(senderPtr->nextSendMutex));
        }

        /** Process internal input buffers */
        /*
         *   - Send a new ping if:
         *      - We're not waiting for a ping, AND the previous one was sent more than
         *        ARNETWORK_SENDER_MINIMUM_TIME_BETWEEN_PINGS_MS ago
         *      - The previous ping was sent more than ARNETWORK_SENDER_PING_TIMEOUT_MS ago
         */
        gettimeofday (&now, NULL);
        ARSAL_Mutex_Lock (&(senderPtr->pingMutex));
        timeDiffMs = ARSAL_Time_ComputeMsTimeDiff (&(senderPtr->pingStartTime), &now);
        if (((senderPtr->isPingRunning == 0) &&
             (timeDiffMs > ARNETWORK_SENDER_MINIMUM_TIME_BETWEEN_PINGS_MS)) ||
            (timeDiffMs > ARNETWORK_SENDER_PING_TIMEOUT_MS))
        {
            if (timeDiffMs > ARNETWORK_SENDER_PING_TIMEOUT_MS)
            {
                senderPtr->lastPingValue = -1;
            }
            inputBufferPtrTemp = senderPtr->inputBufferPtrMap[ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING];
            ARNETWORK_IOBuffer_Lock (inputBufferPtrTemp);
            ARNETWORK_IOBuffer_AddData (inputBufferPtrTemp, (uint8_t *)&now, sizeof (now), NULL, NULL, 1);
            ARNETWORK_IOBuffer_Unlock (inputBufferPtrTemp);
            senderPtr->pingStartTime.tv_sec = now.tv_sec;
            senderPtr->pingStartTime.tv_usec = now.tv_usec;
            senderPtr->isPingRunning = 1;
        }

        ARSAL_Mutex_Unlock (&(senderPtr->pingMutex));

        for (inputBufferIndex = 0; inputBufferIndex < ARNETWORK_MANAGER_IOBUFFER_MAP_SIZE; inputBufferIndex++)
        {
            inputBufferPtrTemp = senderPtr->inputBufferPtrMap[inputBufferIndex];
            if (inputBufferPtrTemp != NULL)
            {
                ARNETWORK_Sender_ProcessBufferToSend (senderPtr, inputBufferPtrTemp, mustWait);
            }
        }

        ARNETWORK_Sender_Send (senderPtr);
    }

    ARSAL_Socket_Close (senderPtr->socket);

    return NULL;
}

void ARNETWORK_Sender_ProcessBufferToSend (ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *buffer, int hasWaited)
{
    eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    /** lock the IOBuffer */
    error = ARNETWORK_IOBuffer_Lock(buffer);

    if(error == ARNETWORK_OK)
    {
        /** decrement the time to wait */
        if ((buffer->waitTimeCount > 0) && (hasWaited == 1))
        {
            -- (buffer->waitTimeCount);
        }

        if (ARNETWORK_IOBuffer_IsWaitAck (buffer))
        {
            if (buffer->ackWaitTimeCount == 0)
            {
                if (buffer->retryCount == 0)
                {
                    /** if there are timeout and too sending retry ... */

                    ARSAL_PRINT (ARSAL_PRINT_DEBUG, ARNETWORK_SENDER_TAG, "!!! too retry !!!");

                    callbackReturn = ARNETWORK_Sender_TimeOutCallback (senderPtr, buffer);

                    ARNETWORK_Sender_ManageTimeOut (senderPtr, buffer, callbackReturn);

                }
                else
                {
                    /** if there is a timeout, retry to send the data */

                    error = ARNETWORK_Sender_AddToBuffer (senderPtr, buffer, buffer->seqWaitAck);
                    if (error == ARNETWORK_OK)
                    {
                        /** reset the timeout counter*/
                        buffer->ackWaitTimeCount = buffer->ackTimeoutMs;

                        /** decrement the number of retry still possible is retryCount isn't -1 */
                        if (buffer->retryCount > 0)
                        {
                            -- (buffer->retryCount);
                        }
                    }
                }
            }

            /** decrement the time to wait before considering as a timeout */
            if ((buffer->ackWaitTimeCount > 0) && (hasWaited == 1))
            {
                -- (buffer->ackWaitTimeCount);
            }
        }

        else if ((!ARNETWORK_RingBuffer_IsEmpty (buffer->dataDescriptorRBufferPtr)) && (buffer->waitTimeCount == 0))
        {
            /** try to add the latest data of the input buffer in the sending buffer; callback with sent status */
            if (!ARNETWORK_Sender_AddToBuffer (senderPtr, buffer, senderPtr->seq))
            {
                buffer->waitTimeCount = buffer->sendingWaitTimeMs;

                switch (buffer->dataType)
                {
                case ARNETWORK_FRAME_TYPE_DATA_WITH_ACK:
                    /**
                     * reinitialize the input buffer parameters,
                     * save the sequence wait for the acknowledgement,
                     * and pass on waiting acknowledgement.
                     */
                    buffer->isWaitAck = 1;
                    buffer->seqWaitAck = senderPtr->seq;
                    buffer->ackWaitTimeCount = buffer->ackTimeoutMs;
                    buffer->retryCount = buffer->numberOfRetry;
                    break;

                case ARNETWORK_FRAME_TYPE_DATA:
                    /** pop the data sent */
                    ARNETWORK_IOBuffer_PopData (buffer);
                    break;

                case ARNETWORK_FRAME_TYPE_DATA_LOW_LATENCY:
                    /** pop the data sent */
                    ARNETWORK_IOBuffer_PopData (buffer);
                    break;

                case ARNETWORK_FRAME_TYPE_ACK:
                    /** pop the acknowledgement sent */
                    ARNETWORK_IOBuffer_PopData (buffer);
                    break;

                default:
                    ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_SENDER_TAG, "dataType: %d unknow \n", buffer->dataType);
                    break;
                }

                /** increment the sequence number */
                ++ (senderPtr->seq);
            }
        }

        /** unlock the IOBuffer */
        ARNETWORK_IOBuffer_Unlock(buffer);
    }
}

void ARNETWORK_Sender_Stop (ARNETWORK_Sender_t *senderPtr)
{
    /** -- Stop the sending -- */
    senderPtr->isAlive = 0;
}

void ARNETWORK_Sender_SignalNewData (ARNETWORK_Sender_t *senderPtr)
{
    ARSAL_Cond_Signal (&(senderPtr->nextSendCond));
}

eARNETWORK_ERROR ARNETWORK_Sender_AckReceived (ARNETWORK_Sender_t *senderPtr, int ID, int seqNumber)
{
    /** -- Receive an acknowledgment fo a data -- */

    /** local declarations */
    ARNETWORK_IOBuffer_t *inputBufferPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;

    inputBufferPtr = senderPtr->inputBufferPtrMap[ID];

    if (inputBufferPtr != NULL)
    {
        /** lock the IOBuffer */
        error = ARNETWORK_IOBuffer_Lock (inputBufferPtr);

        if (error == ARNETWORK_OK)
        {
            /**
             *  Transmit the acknowledgment to the input buffer.
             *     if the acknowledgment is suiarray the waiting data is popped
             */
            error = ARNETWORK_IOBuffer_AckReceived (inputBufferPtr, seqNumber);

            /** unlock the IOBuffer */
            ARNETWORK_IOBuffer_Unlock (inputBufferPtr);
        }
    }
    else
    {
        error = ARNETWORK_ERROR_ID_UNKNOWN;
    }

    return error;
}

eARNETWORK_ERROR ARNETWORK_Sender_Connect (ARNETWORK_Sender_t *senderPtr, const char *addr, int port)
{
    /** -- Connect the socket in UDP to a port of an address -- */

    /** local declarations */
    struct sockaddr_in sendSin;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int connectError;

    sendSin.sin_addr.s_addr = inet_addr (addr);
    sendSin.sin_family = AF_INET;
    sendSin.sin_port = htons (port);

    senderPtr->socket = ARSAL_Socket_Create (AF_INET, SOCK_DGRAM, 0);

    connectError = ARSAL_Socket_Connect (senderPtr->socket, (struct sockaddr*) &sendSin, sizeof (sendSin));

    if (connectError != 0)
    {
        switch (errno)
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

eARNETWORK_ERROR ARNETWORK_Sender_Flush (ARNETWORK_Sender_t *senderPtr)
{
    /** -- Flush all IoBuffer -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int inputIndex;
    ARNETWORK_IOBuffer_t *inputBufferTemp = NULL;

    /** for each input buffer */
    for (inputIndex = 0; inputIndex < senderPtr->numberOfInputBuff && error == ARNETWORK_OK; ++inputIndex)
    {
        inputBufferTemp = senderPtr->inputBufferPtrArr[inputIndex];
        /** lock the IOBuffer */
        error = ARNETWORK_IOBuffer_Lock (inputBufferTemp);

        if (error == ARNETWORK_OK)
        {
            /**  flush the IoBuffer */
            error = ARNETWORK_IOBuffer_Flush (inputBufferTemp);

            /** unlock the IOBuffer */
            ARNETWORK_IOBuffer_Unlock (inputBufferTemp);
        }
    }

    return error;
}

void ARNETWORK_Sender_Reset (ARNETWORK_Sender_t *senderPtr)
{
    /** -- Reset the Sender -- */

    /** local declarations */

    /** flush all IoBuffer */
    ARNETWORK_Sender_Flush (senderPtr);

    /** reset */
    senderPtr->seq = ARNETWORK_SENDER_FIRST_SEQ;
}

/*****************************************
 *
 *             private implementation:
 *
 *****************************************/

void ARNETWORK_Sender_Send (ARNETWORK_Sender_t *senderPtr)
{
    /**  -- send the data -- */

    /** local declarations */
    int numberOfByteToCopy = 0;

    if (!ARNETWORK_Buffer_IsEmpty (senderPtr->sendingBufferPtr))
    {
        numberOfByteToCopy = senderPtr->sendingBufferPtr->frontPtr - senderPtr->sendingBufferPtr->startPtr;

        ARSAL_Socket_Send (senderPtr->socket, senderPtr->sendingBufferPtr->startPtr, numberOfByteToCopy, 0);

        senderPtr->sendingBufferPtr->frontPtr = senderPtr->sendingBufferPtr->startPtr;
    }
}

eARNETWORK_ERROR ARNETWORK_Sender_AddToBuffer (ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *inputBufferPtr, int seqNum)
{
    /** -- add data to the sender buffer and callback with sent status -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint32_t droneEndianInt32 = 0;
    int sizeNeed = 0;
    ARNETWORK_DataDescriptor_t dataDescriptor;

    /** pop data descriptor*/
    error = ARNETWORK_RingBuffer_Front (inputBufferPtr->dataDescriptorRBufferPtr, (uint8_t*) &dataDescriptor);

    if (error == ARNETWORK_OK)
    {
        /** calculate the size needed; (header size of the frame + size of the data) */
        sizeNeed = offsetof (ARNETWORK_Frame_t, dataPtr) + dataDescriptor.dataSize;

        /** check the free size */
        if (ARNETWORK_Buffer_GetFreeCellNumber (senderPtr->sendingBufferPtr) < sizeNeed)
        {
            error = ARNETWORK_ERROR_BUFFER_SIZE;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /** add type */
        memcpy (senderPtr->sendingBufferPtr->frontPtr, &(inputBufferPtr->dataType), sizeof (uint8_t));
        senderPtr->sendingBufferPtr->frontPtr += sizeof (uint8_t);

        /** add id */
        memcpy (senderPtr->sendingBufferPtr->frontPtr, &(inputBufferPtr->ID), sizeof (uint8_t));
        senderPtr->sendingBufferPtr->frontPtr += sizeof (uint8_t);

        /** add seq */
        droneEndianInt32 =  htodl (seqNum);
        memcpy (senderPtr->sendingBufferPtr->frontPtr, &(droneEndianInt32), sizeof (uint32_t));
        senderPtr->sendingBufferPtr->frontPtr += sizeof (uint32_t);

        /** add size */
        droneEndianInt32 =  htodl (sizeNeed);
        memcpy (senderPtr->sendingBufferPtr->frontPtr, &(droneEndianInt32), sizeof (uint32_t));
        senderPtr->sendingBufferPtr->frontPtr += sizeof (uint32_t);

        /** add data */
        /** copy the data pointed by the data descriptor */
        memcpy (senderPtr->sendingBufferPtr->frontPtr, dataDescriptor.dataPtr, dataDescriptor.dataSize);
        /**increment the front of the Sending Buffer */
        senderPtr->sendingBufferPtr->frontPtr += dataDescriptor.dataSize;

        /** callback with sent status */
        if (dataDescriptor.callback != NULL)
        {
            dataDescriptor.callback (inputBufferPtr->ID, dataDescriptor.dataPtr, dataDescriptor.customData, ARNETWORK_MANAGER_CALLBACK_STATUS_SENT);
        }
    }

    return error;
}

eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_Sender_TimeOutCallback (ARNETWORK_Sender_t *senderPtr, const ARNETWORK_IOBuffer_t *inputBufferPtr)
{
    /** -- call the Callback this timeout status -- */

    /** local declarations */
    ARNETWORK_DataDescriptor_t dataDescriptor;
    eARNETWORK_MANAGER_CALLBACK_RETURN callbackRetrun = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;

    /** get dataDescriptor */
    ARNETWORK_RingBuffer_Front (inputBufferPtr->dataDescriptorRBufferPtr, (uint8_t*) &dataDescriptor);

    /** callback with timeout status*/
    if (dataDescriptor.callback != NULL)
    {
        callbackRetrun = dataDescriptor.callback (inputBufferPtr->ID, dataDescriptor.dataPtr, dataDescriptor.customData, ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT);
    }

    return callbackRetrun;
}

void ARNETWORK_Sender_ManageTimeOut (ARNETWORK_Sender_t *senderPtr, ARNETWORK_IOBuffer_t *inputBufferPtr, eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn)
{
    /**  -- Manager the return of the callback -- */

    /** local declarations */

    switch (callbackReturn)
    {
    case ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY :
        /** reset the retry counter */
        inputBufferPtr->retryCount = inputBufferPtr->numberOfRetry;
        break;

    case ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP :
        /** pop the data*/
        ARNETWORK_IOBuffer_PopDataWithCallBack (inputBufferPtr, ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL);

        /** force the waiting acknowledge at 0 */
        inputBufferPtr->isWaitAck = 0;

        break;

    case ARNETWORK_MANAGER_CALLBACK_RETURN_FLUSH :
        /** fluch all IOBuffers */
        ARNETWORK_Sender_Flush (senderPtr);
        break;

    default:
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_SENDER_TAG, "Bad CallBack return :%d", callbackReturn);
        break;
    }
}

int ARNETWORK_Sender_GetPing (ARNETWORK_Sender_t *senderPtr)
{
    int retVal = -1;
    ARSAL_Mutex_Lock (&(senderPtr->pingMutex));
    if (senderPtr->isPingRunning == 1)
    {
        struct timeval now;
        gettimeofday (&now, NULL);
        retVal = ARSAL_Time_ComputeMsTimeDiff (&(senderPtr->pingStartTime), &now);
    }
    if ((senderPtr->lastPingValue > retVal) ||
        (senderPtr->lastPingValue == -1))
    {
        retVal = senderPtr->lastPingValue;
    }
    ARSAL_Mutex_Unlock (&(senderPtr->pingMutex));
    return retVal;
}


void ARNETWORK_Sender_GotPingAck (ARNETWORK_Sender_t *senderPtr, struct timeval *startTime, struct timeval *endTime)
{
    ARSAL_Mutex_Lock (&(senderPtr->pingMutex));
    if ((senderPtr->isPingRunning == 1) &&
        (ARSAL_Time_TimevalEquals (startTime, &(senderPtr->pingStartTime))))
    {
        senderPtr->lastPingValue = ARSAL_Time_ComputeMsTimeDiff (startTime, endTime);
        senderPtr->isPingRunning = 0;
    }
    ARSAL_Mutex_Unlock (&(senderPtr->pingMutex));
}

void ARNETWORK_Sender_SendPong (ARNETWORK_Sender_t *senderPtr, struct timeval *data)
{
    ARNETWORK_IOBuffer_t *inputBufferPtrTemp;
    inputBufferPtrTemp = senderPtr->inputBufferPtrMap[ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG];
    ARNETWORK_IOBuffer_Lock (inputBufferPtrTemp);
    ARNETWORK_IOBuffer_AddData (inputBufferPtrTemp, (uint8_t *)data, sizeof (*data), NULL, NULL, 1);
    ARNETWORK_IOBuffer_Unlock (inputBufferPtrTemp);
}
