/**
 * @file ARNETWORK_Manager.c
 * @brief network manager allow to send data acknowledged or not.
 * @date 28/09/2012
 * @author maxime.maitre@parrot.com
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <stdlib.h>

#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Socket.h>
#include <libARSAL/ARSAL_Time.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetworkAL/ARNETWORKAL_Frame.h>
#include "ARNETWORK_RingBuffer.h"
#include "ARNETWORK_DataDescriptor.h"
#include <libARNetwork/ARNETWORK_IOBufferParam.h>
#include "ARNETWORK_IOBuffer.h"
#include "ARNETWORK_Sender.h"
#include "ARNETWORK_Receiver.h"

#include <libARNetwork/ARNETWORK_Manager.h>
#include "ARNETWORK_Manager.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORK_MANAGER_TAG "ARNETWORK_Manager"
#define ARNETWORK_NETWORK_BUFFER_SIZE   1500

/*****************************************
 *
 *             private header:
 *
 *****************************************/

/**
 * @brief create manager's IOBuffers.
 * @warning only call by ARNETWORK_Manager_New()
 * @pre managerPtr->outputBufferPtrArr and managerPtr->inputBufferPtrArr must be allocated and not set to NULL.
 * @param managerPtr pointer on the Manager
 * @param[in] inputParamArr array of the parameters of creation of the inputs. The array must contain as many parameters as the number of input buffer.
 * @param[in] outputParamArr array of the parameters of creation of the outputs. The array must contain as many parameters as the number of output buffer.
 * @param[in] sendBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 * @return error equal to ARNETWORK_OK if the IOBuffer are correctly created otherwise see eARNETWORK_ERROR.
 * @see ARNETWORK_Manager_New()
 */
eARNETWORK_ERROR ARNETWORK_Manager_CreateIOBuffer (ARNETWORK_Manager_t *managerPtr, ARNETWORK_IOBufferParam_t *inputParamArr, ARNETWORK_IOBufferParam_t *outputParamArr, unsigned int sendBufferSize);

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

ARNETWORK_Manager_t* ARNETWORK_Manager_New(ARNETWORKAL_Manager_t *networkALManager, unsigned int numberOfInput, ARNETWORK_IOBufferParam_t *inputParamArr, unsigned int numberOfOutput, ARNETWORK_IOBufferParam_t *outputParamArr, int pingDelayMs, eARNETWORK_ERROR *errorPtr)
{
    /** -- Create a new Manager -- */

    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;

    /** Create the Manager */
    managerPtr = malloc (sizeof (ARNETWORK_Manager_t));
    if (managerPtr != NULL)
    {
        /** Initialize to default values */
        managerPtr->networkALManager = NULL;
        managerPtr->senderPtr = NULL;
        managerPtr->receiverPtr = NULL;
        managerPtr->inputBufferPtrArr = NULL;
        managerPtr->outputBufferPtrArr = NULL;
        managerPtr->internalInputBufferPtrArr = NULL;
        managerPtr->numberOfOutput = 0;
        managerPtr->numberOfOutputWithoutAck = 0;
        managerPtr->numberOfInput = 0;
        managerPtr->numberOfInputWithoutAck = 0;
        managerPtr->numberOfInternalInputs = 0;
        managerPtr->inputBufferPtrMap = NULL;
        managerPtr->outputBufferPtrMap = NULL;
    }
    else
    {
        error = ARNETWORK_ERROR_ALLOC;
    }

    if (error == ARNETWORK_OK)
    {
        if(networkALManager != NULL)
        {
            managerPtr->networkALManager = networkALManager;
        }
        else
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }
    }

    /**
     * For each output buffer a buffer of acknowledgement is add and referenced
     * in the output buffer list and the input buffer list.
     */

    if (error == ARNETWORK_OK)
    {
        /**
         * Allocate the output buffer list of size of the number of output plus the number of
         * buffer of acknowledgement. The size of the list is then two times the number of output.
         */
        managerPtr->numberOfOutputWithoutAck = numberOfOutput;
        managerPtr->numberOfOutput = 2 * numberOfOutput;
        managerPtr->outputBufferPtrArr = calloc (managerPtr->numberOfOutput, sizeof (ARNETWORK_IOBuffer_t*));
        if (managerPtr->outputBufferPtrArr == NULL)
        {
            error = ARNETWORK_ERROR_ALLOC;
            managerPtr->numberOfOutput = 0;
            managerPtr->numberOfOutputWithoutAck = 0;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /**
         * Allocate the input buffer list of size of the number of input plus the number of
         * buffer of acknowledgement.
         * The size of the list is then number of input plus the number of output.
         */
        managerPtr->numberOfInputWithoutAck = numberOfInput;
        managerPtr->numberOfInput = numberOfInput + numberOfOutput;
        managerPtr->inputBufferPtrArr = calloc (managerPtr->numberOfInput, sizeof (ARNETWORK_IOBuffer_t*));
        if (managerPtr->inputBufferPtrArr == NULL)
        {
            error = ARNETWORK_ERROR_ALLOC;
            managerPtr->numberOfInput = 0;
            managerPtr->numberOfInputWithoutAck = numberOfOutput;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /**
         * Allocate the internal input buffer list
         * Size is the number of internal buffers
         */
        managerPtr->numberOfInternalInputs = ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_MAX;
        managerPtr->internalInputBufferPtrArr = calloc (managerPtr->numberOfInternalInputs, sizeof (ARNETWORK_IOBuffer_t*));
        if (managerPtr->internalInputBufferPtrArr == NULL)
        {
            error = ARNETWORK_ERROR_ALLOC;
            managerPtr->numberOfInternalInputs = 0;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /** Allocate the output buffer map  storing the IOBuffer by their identifier */
        managerPtr->outputBufferPtrMap = calloc (managerPtr->networkALManager->maxIds, sizeof (ARNETWORK_IOBuffer_t*));
        if (managerPtr->outputBufferPtrMap == NULL)
        {
            error = ARNETWORK_ERROR_ALLOC;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /** Allocate the input buffer map  storing the IOBuffer by their identifier */
        managerPtr->inputBufferPtrMap = calloc (managerPtr->networkALManager->maxIds, sizeof (ARNETWORK_IOBuffer_t*));
        if (managerPtr->inputBufferPtrMap == NULL)
        {
            error = ARNETWORK_ERROR_ALLOC;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /** Create manager's IOBuffers and stor it in the inputMap and outputMap*/
        error = ARNETWORK_Manager_CreateIOBuffer (managerPtr, inputParamArr, outputParamArr, ARNETWORK_NETWORK_BUFFER_SIZE);
    }

    if (error == ARNETWORK_OK)
    {
        /** Create the Sender */
        managerPtr->senderPtr = ARNETWORK_Sender_New (managerPtr->networkALManager, managerPtr->numberOfInput, managerPtr->inputBufferPtrArr, managerPtr->numberOfInternalInputs, managerPtr->internalInputBufferPtrArr, managerPtr->inputBufferPtrMap, pingDelayMs);
        if (managerPtr->senderPtr == NULL)
        {
            error = ARNETWORK_ERROR_MANAGER_NEW_SENDER;
        }
    }

    if (error == ARNETWORK_OK)
    {
        /** Create the Receiver */
        managerPtr->receiverPtr = ARNETWORK_Receiver_New (managerPtr->networkALManager, managerPtr->numberOfOutput, managerPtr->outputBufferPtrArr, managerPtr->outputBufferPtrMap);
        if (managerPtr->receiverPtr != NULL)
        {
            managerPtr->receiverPtr->senderPtr = managerPtr->senderPtr;
        }
        else
        {
            error = ARNETWORK_ERROR_MANAGER_NEW_RECEIVER;
        }
    }

    /** delete the Manager if an error occurred */
    if (error != ARNETWORK_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_MANAGER_TAG, "error: %s", ARNETWORK_Error_ToString (error));
        ARNETWORK_Manager_Delete (&managerPtr);
    }

    /** return the error */
    if (errorPtr != NULL)
    {
        *errorPtr = error;
    }

    return managerPtr;
}

void ARNETWORK_Manager_Delete (ARNETWORK_Manager_t **managerPtrAddr)
{
    /** -- Delete the Manager -- */

    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = NULL;
    int ii = 0;

    if (managerPtrAddr)
    {
        managerPtr = *managerPtrAddr;

        if (managerPtr)
        {
            ARNETWORK_Sender_Delete (&(managerPtr->senderPtr));
            ARNETWORK_Receiver_Delete (&(managerPtr->receiverPtr));

            /* Delete all internal buffers */
            for (ii = 0; ii < managerPtr->numberOfInternalInputs; ++ii)
            {
                ARNETWORK_IOBuffer_t **buffer = &(managerPtr->internalInputBufferPtrArr[ii]);
                if (*buffer != NULL)
                {
                    ARNETWORK_IOBuffer_Delete (buffer);
                }
                managerPtr->internalInputBufferPtrArr[ii] = NULL;
            }
            free (managerPtr->internalInputBufferPtrArr);
            managerPtr->internalInputBufferPtrArr = NULL;

            /** Delete all output buffers including the buffers of acknowledgement */
            for (ii = 0; ii< managerPtr->numberOfOutput ; ++ii)
            {
                ARNETWORK_IOBuffer_Delete (&(managerPtr->outputBufferPtrArr[ii]));
            }
            free (managerPtr->outputBufferPtrArr);
            managerPtr->outputBufferPtrArr = NULL;

            /** Delete the input buffers but not the buffers of acknowledgement already deleted */
            for (ii = 0; ii< managerPtr->numberOfInputWithoutAck ; ++ii)
            {
                ARNETWORK_IOBuffer_Delete (&(managerPtr->inputBufferPtrArr[ii]));
            }
            free (managerPtr->inputBufferPtrArr);
            managerPtr->inputBufferPtrArr = NULL;

            free (managerPtr->inputBufferPtrMap);
            managerPtr->inputBufferPtrMap = NULL;

            free (managerPtr->outputBufferPtrMap);
            managerPtr->outputBufferPtrMap = NULL;

            managerPtr->networkALManager = NULL;

            free (managerPtr);
            managerPtr = NULL;
        }

        *managerPtrAddr = NULL;
    }
}

void* ARNETWORK_Manager_SendingThreadRun (void *data)
{
    /** -- Manage the sending of the data -- */

    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = data;
    void *ret = NULL;

    /** check paratemters */
    if (managerPtr != NULL)
    {
        ret = ARNETWORK_Sender_ThreadRun (managerPtr->senderPtr);
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_MANAGER_TAG, "error: %s", ARNETWORK_Error_ToString (ARNETWORK_ERROR_BAD_PARAMETER));
    }

    return ret;
}

void* ARNETWORK_Manager_ReceivingThreadRun (void *data)
{
    /** -- Manage the reception of the data -- */

    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = data;
    void *ret = NULL;

    /** check paratemters */
    if (managerPtr != NULL)
    {
        ret = ARNETWORK_Receiver_ThreadRun (managerPtr->receiverPtr);
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_MANAGER_TAG,"error: %s", ARNETWORK_Error_ToString (ARNETWORK_ERROR_BAD_PARAMETER));
    }

    return ret;
}

void ARNETWORK_Manager_Stop (ARNETWORK_Manager_t *managerPtr)
{
    /** -- stop the threads of sending and reception -- */
    
    /** check paratemters */
    if (managerPtr != NULL)
    {
        ARNETWORK_Sender_Stop (managerPtr->senderPtr);
        ARNETWORK_Receiver_Stop (managerPtr->receiverPtr);
        
        /* unlock all functions of the networkAL to permit to join the threads */
        if (managerPtr->networkALManager->unlock != NULL)
        {
            managerPtr->networkALManager->unlock (managerPtr->networkALManager);
        }
    }
}

eARNETWORK_ERROR ARNETWORK_Manager_Flush (ARNETWORK_Manager_t *managerPtr)
{
    /** -- Flush all buffers of the network manager -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int ii = 0;

    /** Flush all output buffers including the buffers of acknowledgement */
    for (ii = 0; ii< managerPtr->numberOfOutput && error == ARNETWORK_OK; ++ii)
    {
        ARNETWORK_Manager_FlushOutputBuffer (managerPtr, ii);
    }

    /** Flush the input buffers but not the buffers of acknowledgement already flushed */
    for (ii = 0; ii< managerPtr->numberOfInputWithoutAck && error == ARNETWORK_OK; ++ii)
    {
        ARNETWORK_Manager_FlushInputBuffer (managerPtr, ii);
    }

    return error;
}

eARNETWORK_ERROR ARNETWORK_Manager_SendData (ARNETWORK_Manager_t *managerPtr, int inputBufferID, uint8_t *dataPtr, int dataSize, void *customData, ARNETWORK_Manager_Callback_t callback, int doDataCopy)
{
    /** -- Add data to send in a IOBuffer using fixed size data -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_IOBuffer_t *inputBufferPtr = NULL;
    int bufferWasEmpty = 0;

    /** check paratemters:
     *  -   the manager ponter is not NUL
     *  -   the data pointer is not NULL
     *  -   the callback is not NULL
     */
    if ((managerPtr != NULL) && (dataPtr != NULL) && (callback != NULL))
    {
        /** get the address of the inputBuffer */
        inputBufferPtr = managerPtr->inputBufferPtrMap[inputBufferID];

        if (inputBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORK_OK)
    {
        /** lock the IOBuffer */
        error = ARNETWORK_IOBuffer_Lock(inputBufferPtr);
    }

    if(error == ARNETWORK_OK)
    {
        bufferWasEmpty = ARNETWORK_RingBuffer_IsEmpty(inputBufferPtr->dataDescriptorRBufferPtr);
    }

    if(error == ARNETWORK_OK)
    {
        /** add the data in the inputBuffer */
        error = ARNETWORK_IOBuffer_AddData (inputBufferPtr, dataPtr, dataSize, customData, callback, doDataCopy);
        ARNETWORK_IOBuffer_Unlock(inputBufferPtr);
    }

    if (error == ARNETWORK_OK)
    {
        if ((inputBufferPtr->dataType == ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY) ||
            (bufferWasEmpty > 0))
        {
            ARNETWORK_Sender_SignalNewData (managerPtr->senderPtr);
        }
    }

    return error;
}

eARNETWORK_ERROR ARNETWORK_Manager_ReadData (ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr)
{
    /** -- Read data received in a IOBuffer using variable size data (blocking function) -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_IOBuffer_t *outputBufferPtr = NULL;
    int semError = 0;

    /** check paratemters */
    if (managerPtr != NULL)
    {
        /** get the address of the outputBuffer */
        outputBufferPtr = managerPtr->outputBufferPtrMap[outputBufferID];

        /** check outputBufferPtr */
        if (outputBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORK_OK)
    {
        /** try to take the semaphore */
        semError = ARSAL_Sem_Wait (&(outputBufferPtr->outputSem));

        if (semError)
        {
            switch (errno)
            {
            case EAGAIN : /** no semaphore */
                error = ARNETWORK_ERROR_BUFFER_EMPTY;
                break;

            default:
                error = ARNETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }

    /** read data */

    if (error == ARNETWORK_OK)
    {
        /** lock the IOBuffer */
        error = ARNETWORK_IOBuffer_Lock (outputBufferPtr);
    }

    if (error == ARNETWORK_OK)
    {
        error = ARNETWORK_IOBuffer_ReadData (outputBufferPtr, dataPtr, dataLimitSize, readSizePtr);

        /** unlock the IOBuffer */
        ARNETWORK_IOBuffer_Unlock (outputBufferPtr);
    }

    return error;
}

eARNETWORK_ERROR ARNETWORK_Manager_TryReadData (ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr)
{
    /** -- try to read data received in a IOBuffer using variable size data (non-blocking function) -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_IOBuffer_t *outputBufferPtr = NULL;
    int semError = 0;

    /** check paratemters */
    if (managerPtr != NULL)
    {
        /** get the address of the outputBuffer */
        outputBufferPtr = managerPtr->outputBufferPtrMap[outputBufferID];

        /** check outputBufferPtr */
        if (outputBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORK_OK)
    {
        /** try to take the semaphore */
        semError = ARSAL_Sem_Trywait (&(outputBufferPtr->outputSem));

        if (semError)
        {
            switch (errno)
            {
            case EAGAIN : /** no semaphore */
                error = ARNETWORK_ERROR_BUFFER_EMPTY;
                break;

            default:
                error = ARNETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }

    /** read data */

    if (error == ARNETWORK_OK)
    {
        /** lock the IOBuffer */
        error = ARNETWORK_IOBuffer_Lock (outputBufferPtr);
    }

    if (error == ARNETWORK_OK)
    {
        error = ARNETWORK_IOBuffer_ReadData (outputBufferPtr, dataPtr, dataLimitSize, readSizePtr);

        /** unlock the IOBuffer */
        ARNETWORK_IOBuffer_Unlock (outputBufferPtr);
    }

    return error;
}

eARNETWORK_ERROR ARNETWORK_Manager_ReadDataWithTimeout (ARNETWORK_Manager_t *managerPtr, int outputBufferID, uint8_t *dataPtr, int dataLimitSize, int *readSizePtr, int timeoutMs)
{
    /** -- Read, with timeout, a data received in IOBuffer using variable size data -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_IOBuffer_t *outputBufferPtr = NULL;
    int semError = 0;
    struct timespec semTimeout;

    /** check paratemters */
    if (managerPtr != NULL)
    {
        outputBufferPtr = managerPtr->outputBufferPtrMap[outputBufferID];

        /** check pOutputBuffer */
        if (outputBufferPtr == NULL)
        {
            error = ARNETWORK_ERROR_ID_UNKNOWN;
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORK_OK)
    {
        /** convert timeoutMs in timespec */
        semTimeout.tv_sec = timeoutMs / 1000;
        semTimeout.tv_nsec = (timeoutMs % 1000) * 1000000;

        /** try to take the semaphore with timeout*/
        semError = ARSAL_Sem_Timedwait (&(outputBufferPtr->outputSem), &semTimeout);

        if (semError)
        {
            switch (errno)
            {
            case ETIMEDOUT : /** semaphore time out */
                error = ARNETWORK_ERROR_BUFFER_EMPTY;
                break;

            default:
                error = ARNETWORK_ERROR_SEMAPHORE;
                break;
            }
        }
    }

    /** read data */

    if (error == ARNETWORK_OK)
    {
        /** lock the IOBuffer */
        error = ARNETWORK_IOBuffer_Lock (outputBufferPtr);
    }

    if (error == ARNETWORK_OK)
    {
        error = ARNETWORK_IOBuffer_ReadData (outputBufferPtr, dataPtr, dataLimitSize, readSizePtr);

        /** unlock the IOBuffer */
        ARNETWORK_IOBuffer_Unlock (outputBufferPtr);
    }

    return error;
}

/*****************************************
 *
 *             private implementation:
 *
 *****************************************/

eARNETWORK_ERROR ARNETWORK_Manager_CreateIOBuffer (ARNETWORK_Manager_t *managerPtr, ARNETWORK_IOBufferParam_t *inputParamArr, ARNETWORK_IOBufferParam_t *outputParamArr, unsigned int sendBufferSize)
{
    /** -- Create manager's IoBuffers --*/

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int inputIndex = 0;
    int outputIndex = 0;
    int indexAckOutput = 0;
    ARNETWORK_IOBufferParam_t paramNewACK;
    ARNETWORK_IOBufferParam_t paramPingBuffer;

    /** Initialize the default parameters for the buffers of acknowledgement. */
    ARNETWORK_IOBufferParam_DefaultInit (&paramNewACK);
    paramNewACK.dataType = ARNETWORKAL_FRAME_TYPE_ACK;
    paramNewACK.numberOfCell = 1;
    paramNewACK.dataCopyMaxSize = sizeof (( (ARNETWORKAL_Frame_t *)NULL)->seq);
    paramNewACK.isOverwriting = 0;

    /** Initialize the ping buffers parameters */
    ARNETWORK_IOBufferParam_DefaultInit (&paramPingBuffer);
    paramPingBuffer.dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    paramPingBuffer.numberOfCell = 1;
    paramPingBuffer.dataCopyMaxSize = sizeof (struct timeval);
    paramPingBuffer.isOverwriting = 1;

    /**
     *  For each output buffer a buffer of acknowledgement is add and referenced
     *  in the output buffer list and the input buffer list.
     */

    /*
     * Create the input/output buffers for the "library reserved" part
     * - Start by creating the input-only buffers
     * - Then create all output buffers, and add loopback buffers to the end of the internal input buffers
     */
    outputIndex = 0;
    inputIndex  = 0;
    /* Generate inputs */
    // Iterate on all ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_XXX values
    //  - ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING
    if (error == ARNETWORK_OK)
    {
        paramPingBuffer.ID = ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING;
        managerPtr->internalInputBufferPtrArr[inputIndex] = ARNETWORK_IOBuffer_New (&paramPingBuffer, 1);
        if (managerPtr->internalInputBufferPtrArr[inputIndex] == NULL)
        {
            error = ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER;
        }
        managerPtr->inputBufferPtrMap [ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING] = managerPtr->internalInputBufferPtrArr [inputIndex];
        inputIndex++;
    }
    //  - ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG
    if (error == ARNETWORK_OK)
    {
        paramPingBuffer.ID = ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG;
        managerPtr->internalInputBufferPtrArr[inputIndex] = ARNETWORK_IOBuffer_New (&paramPingBuffer, 1);
        if (managerPtr->internalInputBufferPtrArr[inputIndex] == NULL)
        {
            error = ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER;
        }
        managerPtr->inputBufferPtrMap [ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG] = managerPtr->internalInputBufferPtrArr [inputIndex];
        inputIndex++;
    }

    /* Generate outputs */
    // Iterate on all ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_XXX values
    //  - Don't create output buffer for ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING;
    //  - Don't create output buffer for ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG;

    /* END OF INTERNAL BUFFERS CREATION */

    /** Create the output buffers and the buffers of acknowledgement */
    for (outputIndex = 0; outputIndex < managerPtr->numberOfOutputWithoutAck && error == ARNETWORK_OK ; ++outputIndex)
    {
        /** check parameters */
        /** -   all output buffer must have the ability to copy */
        /** -   id must be within range [ackIdOffset;ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_MAX] */
        if ((outputParamArr[outputIndex].ID >= (managerPtr->networkALManager->maxIds / 2)) ||
            (outputParamArr[outputIndex].ID <  ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_MAX) ||
            (outputParamArr[outputIndex].dataCopyMaxSize == 0))
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }

        if (error == ARNETWORK_OK)
        {
            /** Create the output buffer */
            managerPtr->outputBufferPtrArr[outputIndex] = ARNETWORK_IOBuffer_New (&(outputParamArr[outputIndex]), 0);
            if (managerPtr->outputBufferPtrArr[outputIndex] == NULL)
            {
                error = ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER;
            }
        }

        if (error == ARNETWORK_OK)
        {
            /** Create the buffer of acknowledgement associated with the output buffer */

            paramNewACK.ID = ARNETWORK_Manager_IDOutputToIDAck (managerPtr->networkALManager, outputParamArr[outputIndex].ID);
            indexAckOutput = managerPtr->numberOfOutputWithoutAck + outputIndex;

            managerPtr->outputBufferPtrArr[indexAckOutput] = ARNETWORK_IOBuffer_New (&paramNewACK, 1);
            if (managerPtr->outputBufferPtrArr[indexAckOutput] == NULL)
            {
                error = ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER;
            }
        }

        if (error == ARNETWORK_OK)
        {
            /** store buffer of acknowledgement at the end of the output and input buffer lists. */
            managerPtr->inputBufferPtrArr[managerPtr->numberOfInputWithoutAck + outputIndex] = managerPtr->outputBufferPtrArr[indexAckOutput];

            /** store the outputBuffer and the buffer of acknowledgement in the IOBuffer Maps*/
            managerPtr->outputBufferPtrMap[managerPtr->outputBufferPtrArr[outputIndex]->ID] = managerPtr->outputBufferPtrArr[outputIndex];
            managerPtr->outputBufferPtrMap[managerPtr->outputBufferPtrArr[indexAckOutput]->ID] = managerPtr->outputBufferPtrArr[indexAckOutput];
            managerPtr->inputBufferPtrMap[managerPtr->outputBufferPtrArr[indexAckOutput]->ID] = managerPtr->outputBufferPtrArr[indexAckOutput];
        }
    }

    /** Create the input buffers */
    for (inputIndex = 0; inputIndex< managerPtr->numberOfInputWithoutAck && error == ARNETWORK_OK ; ++inputIndex)
    {
        /** check parameters: */
        /** -   id is smaller than the id acknowledge offset */
        /** -   dataCopyMaxSize isn't too big */
        if ((inputParamArr[inputIndex].ID >= (managerPtr->networkALManager->maxIds / 2)) ||
            (inputParamArr[inputIndex].ID <  ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_MAX) ||
            (inputParamArr[inputIndex].dataCopyMaxSize >= (sendBufferSize - offsetof (ARNETWORKAL_Frame_t, dataPtr))))
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }

        if (error == ARNETWORK_OK)
        {
            /** Create the intput buffer */
            managerPtr->inputBufferPtrArr[inputIndex] = ARNETWORK_IOBuffer_New (&(inputParamArr[inputIndex]), 0);
            if (managerPtr->inputBufferPtrArr[inputIndex] == NULL)
            {
                error = ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER;
            }
        }

        if (error == ARNETWORK_OK)
        {
            /** store the inputBuffer in the ioBuffer Map */
            managerPtr->inputBufferPtrMap[managerPtr->inputBufferPtrArr[inputIndex]->ID] = managerPtr->inputBufferPtrArr[inputIndex];
        }
    }

    return error;
}

eARNETWORK_ERROR ARNETWORK_Manager_FlushInputBuffer (ARNETWORK_Manager_t *managerPtr, int inBufferID)
{
    eARNETWORK_ERROR error = ARNETWORK_OK;
    if (managerPtr == NULL)
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    else
    {
        ARNETWORK_IOBuffer_t *buffer = managerPtr->inputBufferPtrMap[inBufferID];
        if (buffer != NULL)
        {
            /** lock the IOBuffer */
            error = ARNETWORK_IOBuffer_Lock (buffer);

            if (error == ARNETWORK_OK)
            {
                /** flush the IOBuffer*/
                error = ARNETWORK_IOBuffer_Flush (buffer);

                /** unlock the IOBuffer */
                ARNETWORK_IOBuffer_Unlock (buffer);
            }
        }
        else
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }
    }
    return error;
}

eARNETWORK_ERROR ARNETWORK_Manager_FlushOutputBuffer (ARNETWORK_Manager_t *managerPtr, int outBufferID)
{
    eARNETWORK_ERROR error = ARNETWORK_OK;
    if (managerPtr == NULL)
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    else
    {
        ARNETWORK_IOBuffer_t *buffer = managerPtr->outputBufferPtrMap[outBufferID];

        if (buffer != NULL)
        {
            /** lock the IOBuffer */
            error = ARNETWORK_IOBuffer_Lock (buffer);

            if (error == ARNETWORK_OK)
            {
                /** flush the IOBuffer*/
                error = ARNETWORK_IOBuffer_Flush (buffer);

                /** unlock the IOBuffer */
                ARNETWORK_IOBuffer_Unlock (buffer);
            }
        }
        else
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }
    }
    return error;
}

int ARNETWORK_Manager_GetEstimatedLatency (ARNETWORK_Manager_t *managerPtr)
{
    int result = -1;
    if (managerPtr != NULL)
    {
        result = ARNETWORK_Sender_GetPing (managerPtr->senderPtr);
    }
    return result;
}

int ARNETWORK_Manager_GetEstimatedMissPercentage (ARNETWORK_Manager_t *managerPtr, int outBufferID)
{
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int result = 0;
    if (managerPtr == NULL)
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    else
    {
        ARNETWORK_IOBuffer_t *buffer = managerPtr->outputBufferPtrMap[outBufferID];

        if (buffer != NULL)
        {
            /** lock the IOBuffer */
            error = ARNETWORK_IOBuffer_Lock (buffer);

            if (error == ARNETWORK_OK)
            {
                /** Gets the buffer estimated miss percentage */
                result = ARNETWORK_IOBuffer_GetEstimatedMissPercentage (buffer);
                if (result < 0)
                {
                    error = (eARNETWORK_ERROR)result;
                }

                /** unlock the IOBuffer */
                ARNETWORK_IOBuffer_Unlock (buffer);
            }
        }
        else
        {
            error = ARNETWORK_ERROR_BAD_PARAMETER;
        }
    }

    if (error != ARNETWORK_OK)
    {
        return (int)error;
    }
    else
    {
        return result;
    }
}


eARNETWORK_ERROR ARNETWORK_Manager_SetMinimumTimeBetweenSends (ARNETWORK_Manager_t *managerPtr, int minimumTimeMs)
{
    if ((managerPtr == NULL) ||
        (managerPtr->senderPtr == NULL))
    {
        return ARNETWORK_ERROR_BAD_PARAMETER;
    }
    managerPtr->senderPtr->minimumTimeBetweenSendsMs = minimumTimeMs;
    return ARNETWORK_OK;
}
