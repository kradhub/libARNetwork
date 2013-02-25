/**
 *  @file autoTest.c
 *  @brief libARNetwork TestBench automatic
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <stdio.h> 
#include <stdlib.h> 

#include <time.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Thread.h>

#include <string.h>

#include <libARNetwork/ARNETWORK_Frame.h>
#include <libARNetwork/ARNETWORK_Manager.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>

#include <unistd.h>

/*****************************************
 * 
 *             define :
 *
******************************************/

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

#define AUTOTEST_SENDING_BUFFER_SIZE 256
#define AUTOTEST_RECEIVER_BUFFER_SIZE 256

#define AUTOTEST_NUMBER_OF_INPUT_NET1 4
#define AUTOTEST_NUMBER_OF_OUTPUT_NET1 4
#define AUTOTEST_NUMBER_OF_INPUT_NET2 4
#define AUTOTEST_NUMBER_OF_OUTPUT_NET2 4

/** define of the ioBuffer identifiers */
typedef enum
{
    ID_IOBUFFER_CHAR_DATA = 5,
    ID_IOBUFFER_INT_DATA_WITH_ACK,
    ID_IOBUFFER_INT_DATA,
    ID_IOBUFFER_VARIABLE_SIZE_DATA,
    ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK
    
}eID_IOBUFFER;

typedef struct
{
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

eARNETWORK_MANAGER_CALLBACK_RETURN AUTOTEST_VariableSizeDataCallback(int OutBufferId, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

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
******************************************/

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
    
    printf(" -- libARNetwork TestBench auto -- \n");
    
    /** create the Manager1 */
    managerCheck1.managerPtr = ARNETWORK_Manager_New( AUTOTEST_RECEIVER_BUFFER_SIZE, AUTOTEST_SENDING_BUFFER_SIZE, AUTOTEST_NUMBER_OF_INPUT_NET1, paramInputNetwork1, AUTOTEST_NUMBER_OF_OUTPUT_NET1, paramOutputNetwork1, &error );
    /** initialize the socket of the Manager1 */
    if( error == ARNETWORK_OK )
    {
        error = ARNETWORK_Manager_SocketsInit(managerCheck1.managerPtr, AUTOTEST_ADRR_IP, AUTOTEST_PORT1, AUTOTEST_PORT2, AUTOTEST_RECEIVER_TIMEOUT_SEC);
        if(error != ARNETWORK_OK)
        {
            printf("managerCheck1.managerPtr error initsocket = %d \n", error);
        }
    }
    
    /** create the Manager2 */
    if( error == ARNETWORK_OK )
    {
        managerCheck2.managerPtr = ARNETWORK_Manager_New( AUTOTEST_RECEIVER_BUFFER_SIZE, AUTOTEST_SENDING_BUFFER_SIZE, AUTOTEST_NUMBER_OF_INPUT_NET2, paramInputNetwork2, AUTOTEST_NUMBER_OF_OUTPUT_NET2, paramOutputNetwork2, &error );  
    }
    /** initialize the socket of the Manager2 */
    if( error == ARNETWORK_OK )
    {
        error = ARNETWORK_Manager_SocketsInit(managerCheck2.managerPtr, AUTOTEST_ADRR_IP, AUTOTEST_PORT2, AUTOTEST_PORT1, AUTOTEST_RECEIVER_TIMEOUT_SEC);
        if(error != ARNETWORK_OK)
        {
            printf("managerCheck2.managerPtr error initsocket = %d \n ", error);
        }
    }
    
    if( error == ARNETWORK_OK )
    {
        printf("check start \n");
        
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

        printf(" -- stop -- \n");
        
        /** stop all therad */
        ARNETWORK_Manager_Stop(managerCheck1.managerPtr);
        ARNETWORK_Manager_Stop(managerCheck2.managerPtr);
        
        printf("wait ... \n");
        
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
    }

    /** print result */

    printf("\n");
    
    printf(" -- managerCheck1 to managerCheck2 -- \n " );
    printf(" %d data sent | %d data receved \n ", managerCheck1.numberOfFixedSizeDataSent, managerCheck2.numberOfFixedSizeDataReceived );
    printf(" %d dataACK sent | %d data receved \n ", managerCheck1.numberOfFixedSizeDataAckSent, managerCheck2.numberOfFixedSizeDataAckReceived );
    printf(" %d dataDeported sent | %d dataDeported receved \n ", managerCheck1.numberOfVariableSizeDataSent, managerCheck2.numberOfVariableSizeDataReceived );
    printf(" %d dataDeportedAck sent | %d dataDeportedAck receved \n ", managerCheck1.numberOfVariableSizeDataAckSent, managerCheck2.numberOfVariableSizeDataAckReceived );
    printf(" number of transmission error: %d \n", managerCheck2.numberOfError);
    
    printf("\n");
    
    printf(" -- managerCheck2 to managerCheck1 -- \n " );
    printf(" %d data sent | %d data receved \n ", managerCheck2.numberOfFixedSizeDataSent, managerCheck1.numberOfFixedSizeDataReceived );
    printf(" %d dataACK sent | %d data receved \n ", managerCheck2.numberOfFixedSizeDataAckSent, managerCheck1.numberOfFixedSizeDataAckReceived );
    printf(" %d dataDeported sent | %d dataDeported receved \n ", managerCheck2.numberOfVariableSizeDataSent, managerCheck1.numberOfVariableSizeDataReceived );
    printf(" %d dataDeportedAck sent | %d dataDeportedAck receved \n ", managerCheck2.numberOfVariableSizeDataAckSent, managerCheck1.numberOfVariableSizeDataAckReceived );
    printf(" number of transmission error: %d \n", managerCheck1.numberOfError);

    printf("\n");

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
        printf(" # -- Good result of the test bench -- # \n");
    }
    else
    {
        printf(" # -- Bad result of the test bench -- # \n\n");
        
        printf("    libARNetwork error : %d \n", error);
        printf("    %d fixed size data acknowledged lost of manager 1 to manager 2 \n", FSDataAckTransM1toM2Dif);
        printf("    %d variable size data acknowledged lost of manager 1 to manager 2 \n", VSDataAckTransM1toM2Dif);
        printf("    %d fixed size data acknowledged lost of manager 2 to manager 1 \n", FSDataAckTransM2toM1Dif);
        printf("    %d variable size data acknowledged lost of manager 2 to manager 1 \n", VSDataAckTransM2toM1Dif);
        printf("    %d data corrupted of manager 1 to manager 2 \n", managerCheck2.numberOfError);
        printf("    %d data corrupted of manager 2 to manager 1 \n", managerCheck1.numberOfError);
        
        ret = -1;
    }
    
    printf("\n");
    printf("end \n");
    
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
    inputArr1[0].dataType = ARNETWORK_FRAME_TYPE_DATA;
    inputArr1[0].numberOfCell = 1;
    inputArr1[0].cellSize = sizeof(char);
    inputArr1[0].isOverwriting = 1;
    
    /** input ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[1]) );
    inputArr1[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    inputArr1[1].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    inputArr1[1].sendingWaitTimeMs = 2;
    inputArr1[1].ackTimeoutMs = 5;
    inputArr1[1].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER /*20*/;
    inputArr1[1].numberOfCell = 5;
    inputArr1[1].cellSize = sizeof(int);
    
    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[2]) );
    inputArr1[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    inputArr1[2].dataType = ARNETWORK_FRAME_TYPE_DATA;
    inputArr1[2].sendingWaitTimeMs = 2;
    inputArr1[2].numberOfCell = 5;
    inputArr1[2].isUsingVariableSizeData = 1;
    
    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr1[3]) );
    inputArr1[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    inputArr1[3].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    inputArr1[3].sendingWaitTimeMs = 2;
    inputArr1[3].ackTimeoutMs = 5;
    inputArr1[3].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER /*20*/;
    inputArr1[3].numberOfCell = 5;
    inputArr1[3].isUsingVariableSizeData = 1;
    
    /** outputs: */
    
    /** output ID_IOBUFFER_CHAR_DATA char */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[0]) );
    outputArr1[0].ID = ID_IOBUFFER_CHAR_DATA;
    outputArr1[0].dataType = ARNETWORK_FRAME_TYPE_DATA;
    outputArr1[0].numberOfCell = 1;
    outputArr1[0].cellSize = sizeof(char);
    outputArr1[0].isOverwriting = 1;
    
    /** output ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[1]) );
    outputArr1[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    outputArr1[1].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    outputArr1[1].sendingWaitTimeMs = 2;
    outputArr1[1].ackTimeoutMs = 5;
    outputArr1[1].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    outputArr1[1].numberOfCell = 5;
    outputArr1[1].cellSize = sizeof(int);
    
    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[2]) );
    outputArr1[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    outputArr1[2].dataType = ARNETWORK_FRAME_TYPE_DATA;
    outputArr1[2].sendingWaitTimeMs = 2;
    outputArr1[2].numberOfCell = 5;
    outputArr1[2].isUsingVariableSizeData = 1;
    
    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr1[3]) );
    outputArr1[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    outputArr1[3].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    outputArr1[3].sendingWaitTimeMs = 2;
    outputArr1[3].ackTimeoutMs = 5;
    outputArr1[3].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    outputArr1[3].numberOfCell = 5;
    outputArr1[3].isUsingVariableSizeData = 1;
    
    /** ----------------------------- */
    
    /**--- network 2 --- */
    
    /** input ID_IOBUFFER_CHAR_DATA char */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[0]) );
    inputArr2[0].ID = ID_IOBUFFER_CHAR_DATA;
    inputArr2[0].dataType = ARNETWORK_FRAME_TYPE_DATA;
    inputArr2[0].numberOfCell = 1;
    inputArr2[0].cellSize = sizeof(char);
    inputArr2[0].isOverwriting = 1;
    
    /** input ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[1]) );
    inputArr2[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    inputArr2[1].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    inputArr2[1].sendingWaitTimeMs = 2;
    inputArr2[1].ackTimeoutMs = 5;
    inputArr2[1].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    inputArr2[1].numberOfCell = 5;
    inputArr2[1].cellSize = sizeof(int);
    
    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[2]) );
    inputArr2[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    inputArr2[2].dataType = ARNETWORK_FRAME_TYPE_DATA;
    inputArr2[2].sendingWaitTimeMs = 2;
    inputArr2[2].numberOfCell = 5;
    inputArr2[2].isUsingVariableSizeData = 1;
    
    /** input ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(inputArr2[3]) );
    inputArr2[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    inputArr2[3].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    inputArr2[3].sendingWaitTimeMs = 2;
    inputArr2[3].ackTimeoutMs = 5;
    inputArr2[3].numberOfRetry = ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER/*20*/;
    inputArr2[3].numberOfCell = 5;
    inputArr2[3].isUsingVariableSizeData = 1;
    
    /** outputs: */
    
    /**  output ID_IOBUFFER_CHAR_DATA int */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[0]) );
    outputArr2[0].ID = ID_IOBUFFER_CHAR_DATA;
    outputArr2[0].dataType = ARNETWORK_FRAME_TYPE_DATA;
    outputArr2[0].sendingWaitTimeMs = 3;
    outputArr2[0].numberOfCell = 1;
    outputArr2[0].cellSize = sizeof(char);
    outputArr2[0].isOverwriting = 1;
    
    /** output ID_IOBUFFER_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[1]) );
    outputArr2[1].ID = ID_IOBUFFER_INT_DATA_WITH_ACK;
    outputArr2[1].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    outputArr2[1].numberOfCell = 5;
    outputArr2[1].cellSize = sizeof(int);
    
    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[2]) );
    outputArr2[2].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA;
    outputArr2[2].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    outputArr2[2].numberOfCell = 5;
    outputArr2[2].isUsingVariableSizeData = 1;
    
    /** output ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK */
    ARNETWORK_IOBufferParam_DefaultInit( &(outputArr2[3]) );
    outputArr2[3].ID = ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK;
    outputArr2[3].dataType = ARNETWORK_FRAME_TYPE_DATA_WITH_ACK;
    outputArr2[3].numberOfCell = 5;
    outputArr2[3].isUsingVariableSizeData = 1;
    
    /** ----------------------------- */
}

void AUTOTEST_InitManagerCheck( AUTOTEST_ManagerCheck_t *managerCheckPtr )
{
    /** -- intitialize the managerCheck -- */
    
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
    
    char chData = 0;
    int intData = 0;
    
    while(managerCheckPtr->isReadingThreadAlive)
    {
        /** checking */
        if( ARNETWORK_OK == ARNETWORK_Manager_ReadFixedSizeData( managerCheckPtr->managerPtr, ID_IOBUFFER_CHAR_DATA,(uint8_t*) &chData) )
        {
            printf("- charData: %c \n", chData);
            if( AUTOTEST_CheckFixedSizeData(managerCheckPtr, chData) )
            {
                printf("error \n");
            }
        }
        
        if( ARNETWORK_OK == ARNETWORK_Manager_ReadFixedSizeData(managerCheckPtr->managerPtr, ID_IOBUFFER_INT_DATA_WITH_ACK, (uint8_t*) &intData ))
        {
            printf("- ackInt: %d \n", intData);
            if( AUTOTEST_CheckFixedSizeDataACK( managerCheckPtr, intData ) )
            {
                printf("error \n");
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
        
        if( ARNETWORK_OK == ARNETWORK_Manager_ReadVariableSizeData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA, (uint8_t*) dataDeportedRead, AUTOTEST_NUMBER_DATA_SENT + AUTOTEST_STR_SIZE_OFFSET, &readSize ))
        {
            printf("- dataDeportedRead: %s \n",  dataDeportedRead );
            
            if( AUTOTEST_CheckVariableSizeData( managerCheckPtr, readSize) )
            {
                printf("error \n");
            }
        }
        
        if( ARNETWORK_OK == ARNETWORK_Manager_ReadVariableSizeData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK, (uint8_t*) dataDeportedReadAck, AUTOTEST_NUMBER_DATA_SENT + AUTOTEST_STR_SIZE_OFFSET, &readSize ))
        {
            
            printf("- dataDeportedReadAck: %s \n",  dataDeportedReadAck );
            
            if( AUTOTEST_CheckVariableSizeDataACK(managerCheckPtr, dataDeportedReadAck, readSize) )
            {
                printf("error \n");
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

eARNETWORK_MANAGER_CALLBACK_RETURN AUTOTEST_VariableSizeDataCallback(int OutBufferId, uint8_t* dataPtr, void* customData, eARNETWORK_MANAGER_CALLBACK_STATUS status)
{
    /** local declarations */
    int retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    printf(" -- AUTOTEST_VariableSizeDataCallback status: %d ",status);
    
    switch(status)
    {
        case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT :
        case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT_WITH_ACK :
        case ARNETWORK_MANAGER_CALLBACK_STATUS_FREE :
            retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
            free(dataPtr);
            printf(" free --\n");
            break;
        
        case ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT :
            retry = ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY;
            printf(" retry --\n");
            break;
        
        default:
            printf(" default --\n");
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
    
    printf(" send char: %c \n",chData);
    error = ARNETWORK_Manager_SendFixedSizeData( managerCheckPtr->managerPtr, ID_IOBUFFER_CHAR_DATA, (uint8_t*) &chData);
        
    if( error == ARNETWORK_OK)
    {
        /** increment Number of data sent*/
        ++( managerCheckPtr->numberOfFixedSizeDataSent );
    }
    else
    {
        printf(" error send char :%d \n", error);
    }
    
    return error;
}

eARNETWORK_ERROR AUTOTEST_SendFixedSizeDataAck(AUTOTEST_ManagerCheck_t *managerCheckPtr)
{
    /** -- send data acknowledged -- */
    
    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;
    int intData = AUTOTEST_FIRST_INT_ACK_SENT + managerCheckPtr->numberOfFixedSizeDataAckSent;
    
    printf(" send int: %d \n",intData);        
    error = ARNETWORK_Manager_SendFixedSizeData( managerCheckPtr->managerPtr, ID_IOBUFFER_INT_DATA_WITH_ACK, (uint8_t*) &intData );
    
    if( error == ARNETWORK_OK)
    {
        /** increment Number of data acknowledged sent*/
        ++(managerCheckPtr->numberOfFixedSizeDataAckSent); 
    }
    else
    {
        printf(" error send int ack :%d \n", error);
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
    
    printf(" send str: %s size: %d \n", pStrDataDeported, dataDeportSize);  
    error = ARNETWORK_Manager_SendVariableSizeData( managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA, (uint8_t*) pStrDataDeported, dataDeportSize, NULL, &(AUTOTEST_VariableSizeDataCallback) );
    
    if( error == ARNETWORK_OK)
    {
        /** increment Number of data deported not acknowledged sent*/
        ++( managerCheckPtr->numberOfVariableSizeDataSent );
    }
    else
    {
        printf(" error send deported data ack :%d \n", error);
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
    
    printf(" send str: %s size: %d \n", pStrDataDeportedAck, dataDeportSizeAck);
    
    error = ARNETWORK_Manager_SendVariableSizeData(managerCheckPtr->managerPtr, ID_IOBUFFER_VARIABLE_SIZE_DATA_ACK, (uint8_t*) pStrDataDeportedAck, dataDeportSizeAck, NULL, &(AUTOTEST_VariableSizeDataCallback));
    
    if( error == ARNETWORK_OK)
    {
        /** increment Number of data deported acknowledged sent*/
        ++( managerCheckPtr->numberOfVariableSizeDataAckSent );
    }
    else
    {
        printf(" error send deported data ack :%d \n", error);
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
