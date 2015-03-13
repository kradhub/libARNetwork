/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 * @file autoTest.c
 * @brief libARNetwork TestBench automatic
 * @date 05/18/2012
 * @author maxime.maitre@parrot.com
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Thread.h>

#include <string.h>

#include <libARNetworkAL/ARNETWORKAL_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Frame.h>

#include <libARNetwork/ARNETWORK_Manager.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>

#include <unistd.h>

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define AUTOTEST_TAG "Autotest"

#define AUTOTEST_PING_DELAY (0) // Use default value

#define AUTOTEST_NUMBER_DATA_SENT 26
#define AUTOTEST_SENDING_SLEEP_TIME_US 5000
#define AUTOTEST_READING_SLEEP_TIME_US 1000

#define AUTOTEST_RECEIVER_TIMEOUT_SEC 5
#define AUTOTEST_FIRST_CHAR_SENT 'A'
#define AUTOTEST_FIRST_INT_ACK_SENT 100

#define AUTOTEST_FIRST_DEPORTED_DATA 'a'
#define AUTOTEST_STR_SIZE_OFFSET 2 /** offset add to the send number for calculate the size of the string to send */

#define AUTOTEST_RECV_TIMEOUT_MS 10
#define AUTOTEST_PORT1 12345
#define AUTOTEST_PORT2 54321
#define AUTOTEST_ADRR_IP "127.0.0.1"

#define AUTOTEST_NUMBER_OF_INPUT_NET1 4
#define AUTOTEST_NUMBER_OF_OUTPUT_NET1 4
#define AUTOTEST_NUMBER_OF_INPUT_NET2 4
#define AUTOTEST_NUMBER_OF_OUTPUT_NET2 4

/** define of the ioBuffer identifiers */
typedef enum
{
    ID_IOBUFFER_CHAR_DATA = 10,
    ID_IOBUFFER_INT_DATA_WITH_ACK,
    ID_IOBUFFER_INT_DATA,
    ID_IOBUFFER_VARIABLE_SIZE_DATA,
    ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK

}eID_IOBUFFER;

typedef struct
{
    ARNETWORKAL_Manager_t *networkALManagerPtr;
    ARNETWORK_Manager_t *managerPtr;

    ARSAL_Thread_t managerSendingThread;
    ARSAL_Thread_t managerReceiverThread;

    int isReadingThreadAlive; /** life flag of the reading thread */

    ARSAL_Thread_t dataSendingThread;
    ARSAL_Thread_t fixedSizeDataReadingThread;
    ARSAL_Thread_t variableSizeDataReadingThread;

    int numberOfFixedSizeDataSent; /**< number of fixed size data not acknowledged sent */
    int numberOfFixedSizeDataAckSent; /**< number of fixed size data acknowledged sent */
    int numberOfVariableSizeDataSent; /**< number of variable size data not acknowledged sent */
    int numberOfVariableSizeDataAckSent; /**< number of variable size data acknowledged sent */

    int numberOfFixedSizeDataReceived; /**< number of fixed size data not acknowledged receved */
    int numberOfFixedSizeDataAckReceived; /**< number of fixed size data acknowledged receved */
    int numberOfVariableSizeDataReceived; /**< number of variable size data not acknowledged receved */
    int numberOfVariableSizeDataAckReceived; /**< number of variable size data acknowledged receved */

    char lastFSDataRecv; /**< last date not acknowledged receved */
    int lastSizeOfVSDataRecv; /**< last size of the date deported not acknowledged receved */

    int numberOfError; /**< number of cheking error */

}AUTOTEST_ManagerCheck_t;

void AUTOTEST_InitManagerCheck(AUTOTEST_ManagerCheck_t *managerCheckPtr);

void AUTOTEST_InitParamIOBuffer( ARNETWORK_IOBufferParam_t *inputArr1, ARNETWORK_IOBufferParam_t *outputArr1, ARNETWORK_IOBufferParam_t *inputArr2, ARNETWORK_IOBufferParam_t *outputArr2 );

eARNETWORK_MANAGER_CALLBACK_RETURN AUTOTEST_DataCallback(int OutBufferId, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

void* AUTOTEST_DataSendingRun(void*);
void* AUTOTEST_FixedSizeDataReadingRun(void*);
void* AUTOTEST_VariableSizeDataReadingRun(void* data);

eARNETWORK_ERROR AUTOTEST_SendFixedSizeData(AUTOTEST_ManagerCheck_t *managerCheckPtr);
eARNETWORK_ERROR AUTOTEST_SendFixedSizeDataAck(AUTOTEST_ManagerCheck_t *managerCheckPtr);
eARNETWORK_ERROR AUTOTEST_SendVariableSizeData(AUTOTEST_ManagerCheck_t *managerCheckPtr);
eARNETWORK_ERROR AUTOTEST_SendVaribleSizeDatadAck(AUTOTEST_ManagerCheck_t *managerCheckPtr);

char* AUTOTEST_AllocInitString(  int size );

int AUTOTEST_CheckFixedSizeData( AUTOTEST_ManagerCheck_t *managerCheckPtr, char data );
int AUTOTEST_CheckFixedSizeDataACK( AUTOTEST_ManagerCheck_t *managerCheckPtr, int dataAck );
int AUTOTEST_CheckVariableSizeData( AUTOTEST_ManagerCheck_t *managerCheckPtr, int dataSize );
int AUTOTEST_CheckVariableSizeDataACK( AUTOTEST_ManagerCheck_t *managerCheckPtr, char* dataPtrDeportedAck, int dataSizeAck );


/*****************************************
 *
 *          implementation :
 *
 *****************************************/

int main(int argc, char *argv[])
{
    /** local declarations */
    AUTOTEST_ManagerCheck_t managerCheck1;
    AUTOTEST_ManagerCheck_t managerCheck2;

    eARNETWORK_ERROR error = ARNETWORK_OK;
    int FSDataAckTransM1toM2Dif = 0;
    int VSDataAckTransM1toM2Dif = 0;
    int FSDataAckTransM2toM1Dif = 0;
    int VSDataAckTransM2toM1Dif = 0;

    int ret = 0;

    ARNETWORK_IOBufferParam_t paramInputNetwork1[AUTOTEST_NUMBER_OF_INPUT_NET1];
    ARNETWORK_IOBufferParam_t paramOutputNetwork1[AUTOTEST_NUMBER_OF_OUTPUT_NET1];

    ARNETWORK_IOBufferParam_t paramInputNetwork2[AUTOTEST_NUMBER_OF_INPUT_NET2];
    ARNETWORK_IOBufferParam_t paramOutputNetwork2[AUTOTEST_NUMBER_OF_OUTPUT_NET2];

    /** default init */
    AUTOTEST_InitManagerCheck( &managerCheck1 );
    AUTOTEST_InitManagerCheck( &managerCheck2 );
    AUTOTEST_InitParamIOBuffer( paramInputNetwork1, paramOutputNetwork1, paramInputNetwork2, paramOutputNetwork2 );
    /** initialize random seed: */
    srand ( time(NULL) );

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " -- libARNetwork TestBench auto --");

    /** create the Manager1 */
    /** Initialize the OS Specific Network */
    eARNETWORKAL_ERROR specificError = ARNETWORKAL_OK;
    managerCheck1.networkALManagerPtr = ARNETWORKAL_Manager_New(&specificError);
    if(specificError == ARNETWORKAL_OK)
    {
        specificError = ARNETWORKAL_Manager_InitWifiNetwork(managerCheck1.networkALManagerPtr, AUTOTEST_ADRR_IP, AUTOTEST_PORT1, AUTOTEST_PORT2, AUTOTEST_RECEIVER_TIMEOUT_SEC);
        if(specificError != ARNETWORKAL_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "Manager 1 : Can't init Wifi Network = %d", error);
        }
    }

    if(specificError == ARNETWORKAL_OK)
    {
        managerCheck1.managerPtr = ARNETWORK_Manager_New(managerCheck1.networkALManagerPtr, AUTOTEST_NUMBER_OF_INPUT_NET1, paramInputNetwork1, AUTOTEST_NUMBER_OF_OUTPUT_NET1, paramOutputNetwork1, AUTOTEST_PING_DELAY, NULL, NULL, &error );
    }
    else
    {
        error = ARNETWORK_ERROR;
    }

    /** Initialize the OS Specific Network */
    managerCheck2.networkALManagerPtr = ARNETWORKAL_Manager_New(&specificError);
    if(specificError == ARNETWORKAL_OK)
    {
        specificError = ARNETWORKAL_Manager_InitWifiNetwork(managerCheck2.networkALManagerPtr, AUTOTEST_ADRR_IP, AUTOTEST_PORT2, AUTOTEST_PORT1, AUTOTEST_RECEIVER_TIMEOUT_SEC);
        if(specificError != ARNETWORKAL_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "Manager 2 : Can't init Wifi Network = %d", error);
        }
    }

    if(specificError == ARNETWORKAL_OK)
    {
        managerCheck2.managerPtr = ARNETWORK_Manager_New( managerCheck2.networkALManagerPtr, AUTOTEST_NUMBER_OF_INPUT_NET2, paramInputNetwork2, AUTOTEST_NUMBER_OF_OUTPUT_NET2, paramOutputNetwork2, AUTOTEST_PING_DELAY, NULL, NULL, &error );
    }
    else
    {
        error = ARNETWORK_ERROR;
    }

    if( error == ARNETWORK_OK )
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "check start");

        /** create the threads */
        ARSAL_Thread_Create( &(managerCheck2.managerReceiverThread), (ARSAL_Thread_Routine_t) ARNETWORK_Manager_ReceivingThreadRun, managerCheck2.managerPtr );
        ARSAL_Thread_Create( &(managerCheck1.managerReceiverThread), (ARSAL_Thread_Routine_t) ARNETWORK_Manager_ReceivingThreadRun, managerCheck1.managerPtr );

        ARSAL_Thread_Create( &(managerCheck1.managerSendingThread), (ARSAL_Thread_Routine_t) ARNETWORK_Manager_SendingThreadRun, managerCheck1.managerPtr );
        ARSAL_Thread_Create( &(managerCheck2.managerSendingThread), (ARSAL_Thread_Routine_t) ARNETWORK_Manager_SendingThreadRun, managerCheck2.managerPtr );

        /** manager 1 to manager 2 */
        ARSAL_Thread_Create( &(managerCheck1.dataSendingThread), (ARSAL_Thread_Routine_t) AUTOTEST_DataSendingRun, &managerCheck1 );
        ARSAL_Thread_Create( &(managerCheck2.fixedSizeDataReadingThread), (ARSAL_Thread_Routine_t) AUTOTEST_FixedSizeDataReadingRun, &managerCheck2 );
        ARSAL_Thread_Create( &(managerCheck2.variableSizeDataReadingThread), (ARSAL_Thread_Routine_t) AUTOTEST_VariableSizeDataReadingRun, &managerCheck2 );

        /** manager 2 to manager 1 */
        ARSAL_Thread_Create( &(managerCheck2.dataSendingThread), (ARSAL_Thread_Routine_t) AUTOTEST_DataSendingRun, &managerCheck2 );
        ARSAL_Thread_Create( &(managerCheck1.fixedSizeDataReadingThread), (ARSAL_Thread_Routine_t) AUTOTEST_FixedSizeDataReadingRun, &managerCheck1 );
        ARSAL_Thread_Create( &(managerCheck1.variableSizeDataReadingThread), (ARSAL_Thread_Routine_t) AUTOTEST_VariableSizeDataReadingRun, &managerCheck1 );

        /** wait the end of the sending */
        if(managerCheck1.dataSendingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck1.dataSendingThread, NULL );
        }

        if(managerCheck2.dataSendingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck2.dataSendingThread, NULL );
        }

        /** wait for receiving the last data sent */
        usleep(AUTOTEST_SENDING_SLEEP_TIME_US);

        /** stop the reading */
        managerCheck2.isReadingThreadAlive = 0;
        if(managerCheck2.fixedSizeDataReadingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck2.fixedSizeDataReadingThread, NULL );
        }
        if(managerCheck2.variableSizeDataReadingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck2.variableSizeDataReadingThread, NULL );
        }

        managerCheck1.isReadingThreadAlive = 0;
        if(managerCheck1.fixedSizeDataReadingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck1.fixedSizeDataReadingThread, NULL );
        }
        if(managerCheck1.variableSizeDataReadingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck1.variableSizeDataReadingThread, NULL );
        }

        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " -- stop --");

        /** stop all therad */
        ARNETWORK_Manager_Stop(managerCheck1.managerPtr);
        ARNETWORK_Manager_Stop(managerCheck2.managerPtr);

        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "wait ...");

        ARNETWORKAL_Manager_Unlock(managerCheck1.networkALManagerPtr);
        ARNETWORKAL_Manager_Unlock(managerCheck2.networkALManagerPtr);

        /** kill all threads */
        if(managerCheck1.managerSendingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck1.managerSendingThread, NULL );
        }
        if(managerCheck2.managerSendingThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck2.managerSendingThread, NULL );
        }

        if(managerCheck1.managerReceiverThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck1.managerReceiverThread, NULL );
        }

        if(managerCheck2.managerReceiverThread != NULL)
        {
            ARSAL_Thread_Join( managerCheck2.managerReceiverThread, NULL );
        }

        ARNETWORKAL_Manager_CloseWifiNetwork(managerCheck1.networkALManagerPtr);
        ARNETWORKAL_Manager_CloseWifiNetwork(managerCheck2.networkALManagerPtr);
    }

    /** print result */

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " ");

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " -- managerCheck1 to managerCheck2 --" );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d data sent | %d data receved", managerCheck1.numberOfFixedSizeDataSent, managerCheck2.numberOfFixedSizeDataReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d dataACK sent | %d data receved", managerCheck1.numberOfFixedSizeDataAckSent, managerCheck2.numberOfFixedSizeDataAckReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d dataDeported sent | %d dataDeported receved", managerCheck1.numberOfVariableSizeDataSent, managerCheck2.numberOfVariableSizeDataReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d dataDeportedAck sent | %d dataDeportedAck receved", managerCheck1.numberOfVariableSizeDataAckSent, managerCheck2.numberOfVariableSizeDataAckReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " number of transmission error: %d", managerCheck2.numberOfError);

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " ");

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " -- managerCheck2 to managerCheck1 --" );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d data sent | %d data receved", managerCheck2.numberOfFixedSizeDataSent, managerCheck1.numberOfFixedSizeDataReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d dataACK sent | %d data receved", managerCheck2.numberOfFixedSizeDataAckSent, managerCheck1.numberOfFixedSizeDataAckReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d dataDeported sent | %d dataDeported receved", managerCheck2.numberOfVariableSizeDataSent, managerCheck1.numberOfVariableSizeDataReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " %d dataDeportedAck sent | %d dataDeportedAck receved", managerCheck2.numberOfVariableSizeDataAckSent, managerCheck1.numberOfVariableSizeDataAckReceived );
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " number of transmission error: %d", managerCheck1.numberOfError);

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " ");

    /** global cheking */

    FSDataAckTransM1toM2Dif = managerCheck1.numberOfFixedSizeDataAckSent - managerCheck2.numberOfFixedSizeDataAckReceived;
    VSDataAckTransM1toM2Dif = managerCheck1.numberOfVariableSizeDataAckSent - managerCheck2.numberOfVariableSizeDataAckReceived;
    FSDataAckTransM2toM1Dif = managerCheck2.numberOfFixedSizeDataAckSent - managerCheck1.numberOfFixedSizeDataAckReceived;
    VSDataAckTransM2toM1Dif = managerCheck2.numberOfVariableSizeDataAckSent - managerCheck1.numberOfVariableSizeDataAckReceived;

    if( error == ARNETWORK_OK &&
        managerCheck1.numberOfError == 0 &&
        managerCheck2.numberOfError == 0 &&
        FSDataAckTransM1toM2Dif == 0 &&
        VSDataAckTransM1toM2Dif == 0 &&
        FSDataAckTransM2toM1Dif == 0 &&
        VSDataAckTransM2toM1Dif == 0 )
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " # -- Good result of the test bench -- #");
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " # -- Bad result of the test bench -- #");

        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    libARNetwork error : %s", ARNETWORK_Error_ToString (error));
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    %d fixed size data acknowledged lost of manager 1 to manager 2", FSDataAckTransM1toM2Dif);
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    %d variable size data acknowledged lost of manager 1 to manager 2", VSDataAckTransM1toM2Dif);
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    %d fixed size data acknowledged lost of manager 2 to manager 1", FSDataAckTransM2toM1Dif);
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    %d variable size data acknowledged lost of manager 2 to manager 1", VSDataAckTransM2toM1Dif);
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    %d data corrupted of manager 1 to manager 2", managerCheck2.numberOfError);
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "    %d data corrupted of manager 2 to manager 1", managerCheck1.numberOfError);

        ret = -1;
    }

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, " ");
    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "end");

    /** delete */
    ARSAL_Thread_Destroy( &(managerCheck1.managerSendingThread) );
    ARSAL_Thread_Destroy( &(managerCheck2.managerSendingThread) );
    ARSAL_Thread_Destroy( &(managerCheck1.managerReceiverThread) );
    ARSAL_Thread_Destroy( &(managerCheck2.managerReceiverThread) );

    ARSAL_Thread_Destroy( &(managerCheck1.dataSendingThread) );
    ARSAL_Thread_Destroy( &(managerCheck2.fixedSizeDataReadingThread) );
    ARSAL_Thread_Destroy( &(managerCheck2.variableSizeDataReadingThread) );

    ARSAL_Thread_Destroy( &(managerCheck2.dataSendingThread) );
    ARSAL_Thread_Destroy( &(managerCheck1.fixedSizeDataReadingThread) );
    ARSAL_Thread_Destroy( &(managerCheck1.variableSizeDataReadingThread) );

    ARNETWORK_Manager_Delete( &(managerCheck1.managerPtr) );
    ARNETWORK_Manager_Delete( &(managerCheck2.managerPtr) );

    return ret;
}

void AUTOTEST_InitParamIOBuffer(ARNETWORK_IOBufferParam_t *inputArr1, ARNETWORK_IOBufferParam_t *outputArr1, ARNETWORK_IOBufferParam_t *inputArr2, ARNETWORK_IOBufferParam_t *outputArr2)
{
    /** initialization of the buffer parameters */
    /** --- network 1 --- */

    /** input ID_IOBUFFER_CHAR_DATA char */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[0]) );
    inputArr1[0].ID = ID_IOBUFFER_CHAR_DATA;
    inputArr1[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    inputArr1[0].numberOfCell = 1;
    inputArr1[0].dataCopyMaxSize = sizeof(char);
    inputArr1[0].isOverwriting = 1;

    /** input ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[1]) );
    inputArr1[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    inputArr1[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    inputArr1[1].sendingWaitTimeMs = 2;
    inputArr1[1].ackTimeoutMs = 5;
    inputArr1[1].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER /*20*/;
    inputArr1[1].numberOfCell = 5;
    inputArr1[1].dataCopyMaxSize = sizeof(int);

    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[2]) );
    inputArr1[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    inputArr1[2].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    inputArr1[2].sendingWaitTimeMs = 2;
    inputArr1[2].numberOfCell = 5;

    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[3]) );
    inputArr1[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    inputArr1[3].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    inputArr1[3].sendingWaitTimeMs = 2;
    inputArr1[3].ackTimeoutMs = 5;
    inputArr1[3].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER /*20*/;
    inputArr1[3].numberOfCell = 5;

    /** outputs: */

    /** output ID_IOBUFFER_CHAR_DATA char */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[0]) );
    outputArr1[0].ID = ID_IOBUFFER_CHAR_DATA;
    outputArr1[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    outputArr1[0].numberOfCell = 1;
    outputArr1[0].dataCopyMaxSize = sizeof(char);
    outputArr1[0].isOverwriting = 1;

    /** output ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[1]) );
    outputArr1[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    outputArr1[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    outputArr1[1].sendingWaitTimeMs = 2;
    outputArr1[1].ackTimeoutMs = 5;
    outputArr1[1].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER /*20*/;
    outputArr1[1].numberOfCell = 5;
    outputArr1[1].dataCopyMaxSize = sizeof(int);

    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[2]) );
    outputArr1[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    outputArr1[2].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    outputArr1[2].sendingWaitTimeMs = 2;
    outputArr1[2].numberOfCell = 5;
    outputArr1[2].dataCopyMaxSize = sizeof(char) * 30; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[3]) );
    outputArr1[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    outputArr1[3].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    outputArr1[3].sendingWaitTimeMs = 2;
    outputArr1[3].ackTimeoutMs = 5;
    outputArr1[3].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    outputArr1[3].numberOfCell = 5;
    outputArr1[3].dataCopyMaxSize = sizeof(char) * 30; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    /** ----------------------------- */

    /**--- network 2 --- */

    /** input ID_IOBUFFER_CHAR_DATA char */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[0]) );
    inputArr2[0].ID = ID_IOBUFFER_CHAR_DATA;
    inputArr2[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    inputArr2[0].numberOfCell = 1;
    inputArr2[0].dataCopyMaxSize = sizeof(char);
    inputArr2[0].isOverwriting = 1;

    /** input ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[1]) );
    inputArr2[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    inputArr2[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    inputArr2[1].sendingWaitTimeMs = 2;
    inputArr2[1].ackTimeoutMs = 5;
    inputArr2[1].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    inputArr2[1].numberOfCell = 5;
    inputArr2[1].dataCopyMaxSize = sizeof(int);

    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[2]) );
    inputArr2[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    inputArr2[2].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    inputArr2[2].sendingWaitTimeMs = 2;
    inputArr2[2].numberOfCell = 5;

    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[3]) );
    inputArr2[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    inputArr2[3].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    inputArr2[3].sendingWaitTimeMs = 2;
    inputArr2[3].ackTimeoutMs = 5;
    inputArr2[3].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    inputArr2[3].numberOfCell = 5;

    /** outputs: */

    /**  output ID_IOBUFFER_CHAR_DATA int */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[0]) );
    outputArr2[0].ID = ID_IOBUFFER_CHAR_DATA;
    outputArr2[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    outputArr2[0].sendingWaitTimeMs = 3;
    outputArr2[0].numberOfCell = 1;
    outputArr2[0].dataCopyMaxSize = sizeof(char);
    outputArr2[0].isOverwriting = 1;

    /** output ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[1]) );
    outputArr2[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    outputArr2[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    outputArr2[1].numberOfCell = 5;
    outputArr2[1].dataCopyMaxSize = sizeof(int);

    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[2]) );
    outputArr2[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    outputArr2[2].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    outputArr2[2].numberOfCell = 5;
    outputArr2[2].dataCopyMaxSize = sizeof(char) * 30; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[3]) );
    outputArr2[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    outputArr2[3].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    outputArr2[3].numberOfCell = 5;
    outputArr2[3].dataCopyMaxSize = sizeof(char) * 30; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    /** ----------------------------- */
}

void AUTOTEST_InitManagerCheck( AUTOTEST_ManagerCheck_t *managerCheckPtr )
{
    /** -- intitialize the managerCheck -- */
    managerCheckPtr->networkALManagerPtr = NULL;
    managerCheckPtr->managerPtr = NULL;
    managerCheckPtr->managerSendingThread = NULL;
    managerCheckPtr->managerReceiverThread = NULL;

    managerCheckPtr->isReadingThreadAlive  = 1;

    managerCheckPtr->dataSendingThread = NULL;
    managerCheckPtr->fixedSizeDataReadingThread = NULL ;
    managerCheckPtr->variableSizeDataReadingThread = NULL;

    managerCheckPtr->numberOfFixedSizeDataSent = 0;
    managerCheckPtr->numberOfFixedSizeDataAckSent = 0;
    managerCheckPtr->numberOfVariableSizeDataSent = 0;
    managerCheckPtr->numberOfVariableSizeDataAckSent = 0;

    managerCheckPtr->numberOfFixedSizeDataReceived = 0;
    managerCheckPtr->numberOfFixedSizeDataAckReceived = 0;
    managerCheckPtr->numberOfVariableSizeDataReceived = 0;
    managerCheckPtr->numberOfVariableSizeDataAckReceived = 0;

    managerCheckPtr->lastFSDataRecv = AUTOTEST_FIRST_CHAR_SENT - 1;
    managerCheckPtr->lastSizeOfVSDataRecv = -1;

    managerCheckPtr->numberOfError = 0;
}

void* AUTOTEST_DataSendingRun(void* data)
{
    /** -- Data Sending Run thread -- */

    /** local declarations */
    AUTOTEST_ManagerCheck_t *managerCheckPtr = (AUTOTEST_ManagerCheck_t*)data;
    int alive = 1;

    /** send while all data are sent */
    while( alive )
    {
        alive = 0;

        /** for all data type, if not all data are sent, try to send with fifty-fifty chance */

        if( managerCheckPtr->numberOfFixedSizeDataSent < AUTOTEST_NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                AUTOTEST_SendFixedSizeData( managerCheckPtr );
            }
        }

        if( managerCheckPtr->numberOfFixedSizeDataAckSent < AUTOTEST_NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                AUTOTEST_SendFixedSizeDataAck( managerCheckPtr );
            }
        }

        if( managerCheckPtr->numberOfVariableSizeDataSent < AUTOTEST_NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                AUTOTEST_SendVariableSizeData( managerCheckPtr );
            }
        }

        if( managerCheckPtr->numberOfVariableSizeDataAckSent < AUTOTEST_NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                AUTOTEST_SendVaribleSizeDatadAck( managerCheckPtr );
            }
        }

        usleep(AUTOTEST_SENDING_SLEEP_TIME_US);
    }

    return NULL;
}

void* AUTOTEST_FixedSizeDataReadingRun(void* data)
{
    /** -- thread run read and check data -- */

    /** local declarations */
    AUTOTEST_ManagerCheck_t *managerCheckPtr = (AUTOTEST_ManagerCheck_t*)data;

    int readSize = 0;
    char chData = 0;
    int intData = 0;

    while(managerCheckPtr->isReadingThreadAlive)
    {
        /** checking */
        if( ARNETWORK_OK == ARNETWORK_Manager_TryReadData(managerCheckPtr->managerPtr, ID_IOBUFFER_CHAR_DATA, (uint8_t*) &chData, sizeof(char), &readSize) )
        {
            ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "- charData: %c", chData);
            if( AUTOTEST_CheckFixedSizeData(managerCheckPtr, chData) )
            {
                ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error");
            }
        }

        if( ARNETWORK_OK == ARNETWORK_Manager_TryReadData(managerCheckPtr->managerPtr, ID_IOBUFFER_INT_DATA_WITH_ACK, (uint8_t*) &intData, sizeof(int), &readSize) )
        {
            ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "- ackInt: %d", intData);
            if( AUTOTEST_CheckFixedSizeDataACK( managerCheckPtr, intData ) )
            {
                ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error");
            }
        }

        usleep(AUTOTEST_READING_SLEEP_TIME_US);
    }

    return NULL;
}

void* AUTOTEST_VariableSizeDataReadingRun(void* data)
{
    /** -- thread run read and check data deported -- */

    /** local declarations */
    AUTOTEST_ManagerCheck_t *managerCheckPtr = (AUTOTEST_ManagerCheck_t*) data;

    int readSize = 0;
    char dataDeportedRead[ AUTOTEST_NUMBER_DATA_SENT + AUTOTEST_STR_SIZE_OFFSET ];
    char dataDeportedReadAck[ AUTOTEST_NUMBER_DATA_SENT + AUTOTEST_STR_SIZE_OFFSET ];

    while(managerCheckPtr->isReadingThreadAlive)
    {
        /** checking */

        if( ARNETWORK_OK == ARNETWORK_Manager_TryReadData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA, (uint8_t*) dataDeportedRead, AUTOTEST_NUMBER_DATA_SENT + AUTOTEST_STR_SIZE_OFFSET, &readSize ))
        {
            ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "- dataDeportedRead: %s",  dataDeportedRead );

            if( AUTOTEST_CheckVariableSizeData( managerCheckPtr, readSize) )
            {
                ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error");
            }
        }

        if( ARNETWORK_OK == ARNETWORK_Manager_TryReadData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK, (uint8_t*) dataDeportedReadAck, AUTOTEST_NUMBER_DATA_SENT + AUTOTEST_STR_SIZE_OFFSET, &readSize))
        {

            ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "- dataDeportedReadAck: %s",  dataDeportedReadAck );

            if( AUTOTEST_CheckVariableSizeDataACK(managerCheckPtr, dataDeportedReadAck, readSize) )
            {
                ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error");
            }
        }

        usleep(AUTOTEST_READING_SLEEP_TIME_US);
    }

    return NULL;
}

char* AUTOTEST_AllocInitString( int size )
{
    /** allocate and initialize the string to send */

    /** local declarations */
    int ii = 0;
    char* pStr = NULL;

    /** allocate the string */
    pStr = malloc(size);

    /** write data */
    for(ii = 0; ii < size - 1 ; ++ii)
    {
        pStr[ii] = ( AUTOTEST_FIRST_DEPORTED_DATA + ii );
    }

    /** end the string */
    pStr[size-1] = '\0' ;

    return pStr;
}

eARNETWORK_MANAGER_CALLBACK_RETURN AUTOTEST_DataCallback(int OutBufferId, uint8_t* dataPtr, void* customData, eARNETWORK_MANAGER_CALLBACK_STATUS status)
{
    /** local declarations */
    int retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;

    ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " -- AUTOTEST_DataCallback status: %d",status);

    switch(status)
    {
    case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT :
        ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " sent --");

        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_ACK_RECEIVED :
        ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " ack received --");
        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT :
        retry = ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY;
        ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " timeout retry --");
        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL :
        ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " cancel --");
        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_FREE :
        free(dataPtr);
        ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " free --");
        break;

    default:
        ARSAL_PRINT (ARSAL_PRINT_DEBUG, AUTOTEST_TAG, " default --");
        break;
    }

    return retry;
}

eARNETWORK_ERROR AUTOTEST_SendFixedSizeData(AUTOTEST_ManagerCheck_t *managerCheckPtr)
{
    /** -- send data not acknowledged -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    char chData = AUTOTEST_FIRST_CHAR_SENT + managerCheckPtr->numberOfFixedSizeDataSent;

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "send char: %c",chData);
    error = ARNETWORK_Manager_SendData( managerCheckPtr->managerPtr, ID_IOBUFFER_CHAR_DATA, (uint8_t*) &chData, sizeof(char), NULL, &(AUTOTEST_DataCallback), 1 );

    if( error == ARNETWORK_OK)
    {
        /** increment Number of data sent*/
        ++( managerCheckPtr->numberOfFixedSizeDataSent );
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error send char :%s", ARNETWORK_Error_ToString (error));
    }

    return error;
}

eARNETWORK_ERROR AUTOTEST_SendFixedSizeDataAck(AUTOTEST_ManagerCheck_t *managerCheckPtr)
{
    /** -- send data acknowledged -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int intData = AUTOTEST_FIRST_INT_ACK_SENT + managerCheckPtr->numberOfFixedSizeDataAckSent;

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "send int: %d",intData);
    error = ARNETWORK_Manager_SendData(managerCheckPtr->managerPtr, ID_IOBUFFER_INT_DATA_WITH_ACK, (uint8_t*) &intData, sizeof(int), NULL, &(AUTOTEST_DataCallback), 1);

    if( error == ARNETWORK_OK)
    {
        /** increment Number of data acknowledged sent*/
        ++(managerCheckPtr->numberOfFixedSizeDataAckSent);
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error send int ack :%s", ARNETWORK_Error_ToString (error));
    }

    return error;
}

eARNETWORK_ERROR AUTOTEST_SendVariableSizeData(AUTOTEST_ManagerCheck_t *managerCheckPtr)
{
    /** -- send data deported not acknowledged -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    char* pStrDataDeported = NULL;
    int dataDeportSize = managerCheckPtr->numberOfVariableSizeDataSent + AUTOTEST_STR_SIZE_OFFSET;

    /** create DataDeported */
    pStrDataDeported = AUTOTEST_AllocInitString( dataDeportSize );

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "send str: %s size: %d", pStrDataDeported, dataDeportSize);
    error = ARNETWORK_Manager_SendData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA, (uint8_t*) pStrDataDeported, dataDeportSize, NULL, &(AUTOTEST_DataCallback),0);

    if( error == ARNETWORK_OK)
    {
        /** increment Number of data deported not acknowledged sent*/
        ++( managerCheckPtr->numberOfVariableSizeDataSent );
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error send deported data ack :%s", ARNETWORK_Error_ToString (error));
    }

    return error;
}

eARNETWORK_ERROR AUTOTEST_SendVaribleSizeDatadAck(AUTOTEST_ManagerCheck_t *managerCheckPtr)
{
    /** -- send data deported acknowledged -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    char* pStrDataDeportedAck = NULL;
    int dataDeportSizeAck = managerCheckPtr->numberOfVariableSizeDataAckSent + AUTOTEST_STR_SIZE_OFFSET;

    /** create DataDeported */
    pStrDataDeportedAck = AUTOTEST_AllocInitString( dataDeportSizeAck );

    ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "send str: %s size: %d", pStrDataDeportedAck, dataDeportSizeAck);

    error = ARNETWORK_Manager_SendData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK, (uint8_t*) pStrDataDeportedAck, dataDeportSizeAck, NULL, &(AUTOTEST_DataCallback), 0);

    if( error == ARNETWORK_OK)
    {
        /** increment Number of data deported acknowledged sent*/
        ++( managerCheckPtr->numberOfVariableSizeDataAckSent );
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_WARNING, AUTOTEST_TAG, "error send deported data ack :%s", ARNETWORK_Error_ToString (error));
    }

    return error;
}

int AUTOTEST_CheckFixedSizeData( AUTOTEST_ManagerCheck_t *managerCheckPtr, char data )
{
    /** -- check the fixed size data receved -- */

    /** local declarations */
    int error = 0;

    if( data > managerCheckPtr->lastFSDataRecv )
    {
        managerCheckPtr->lastFSDataRecv = data;
    }
    else
    {
        error = 1;
        /** increment the cheking error */
        ++(managerCheckPtr->numberOfError);
    }

    /** increment data not acknowledged receved*/
    ++( managerCheckPtr->numberOfFixedSizeDataReceived );

    return error;
}

int AUTOTEST_CheckFixedSizeDataACK( AUTOTEST_ManagerCheck_t *managerCheckPtr, int dataAck )
{
    /** -- check the fixed size data acknowledged receved -- */

    /** local declarations */
    int error = 0;
    int dataAckCheck = AUTOTEST_FIRST_INT_ACK_SENT + managerCheckPtr->numberOfFixedSizeDataAckReceived;

    if( dataAckCheck != dataAck )
    {
        error = 1;
        /** increment the cheking error */
        ++(managerCheckPtr->numberOfError);
    }

    /** increment data acknowledged receved*/
    ++( managerCheckPtr->numberOfFixedSizeDataAckReceived );

    return error;
}

int AUTOTEST_CheckVariableSizeData( AUTOTEST_ManagerCheck_t *managerCheckPtr, int dataSize)
{
    /** -- check the variable size data not acknowledged receved -- */

    /** local declarations */
    int error = 0;

    if( dataSize > managerCheckPtr->lastSizeOfVSDataRecv )
    {
        managerCheckPtr->lastSizeOfVSDataRecv = dataSize;
    }
    else
    {
        error = 1;
        /** increment the cheking error */
        ++(managerCheckPtr->numberOfError);
    }

    /** increment data deported not acknowledged receved*/
    ++( managerCheckPtr->numberOfVariableSizeDataReceived );

    return error;
}

int AUTOTEST_CheckVariableSizeDataACK( AUTOTEST_ManagerCheck_t *managerCheckPtr, char* dataPtrDeportedAck, int dataSizeAck )
{
    /** -- check the variable size data acknowledged receved -- */

    /** local declarations */
    int error = 0;
    char* pCheckStr = NULL;
    int checkStrSize = managerCheckPtr->numberOfVariableSizeDataAckReceived + 2;

    /** check the size of the data */
    if( dataSizeAck != checkStrSize )
    {
        error = 1;
        /** increment the cheking error */
        ++(managerCheckPtr->numberOfError);
    }

    if(error == 0)
    {
        /** create DataDeportedAck check */
        pCheckStr = AUTOTEST_AllocInitString( checkStrSize );

        /** compare the data with the string expected */
        error = memcmp(dataPtrDeportedAck, pCheckStr, checkStrSize);

        /** free the checking string */
        free(pCheckStr);
    }

    if(error != 0)
    {
        /** increment the cheking error */
        ++(managerCheckPtr->numberOfError);
    }

    /** increment data deported acknowledged receved*/
    ++(managerCheckPtr->numberOfVariableSizeDataAckReceived);

    return error;
}
