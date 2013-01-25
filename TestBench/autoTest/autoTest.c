/**
 *  @file autoTest.c
 *  @brief libNetWork TestBench automatic
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

#include <libSAL/print.h>
#include <libSAL/thread.h>

#include <string.h>

#include <libNetwork/frame.h>
#include <libNetwork/manager.h>
#include <libNetwork/paramNewIoBuffer.h>

#include <unistd.h>

/*****************************************
 * 
 *             define :
 *
******************************************/

#define NUMBER_DATA_SENT 26
#define SENDING_SLEEP_TIME_US 5000
#define READING_SLEEP_TIME_US 1000

#define RECEIVER_TIMEOUT_SEC 5
#define FIRST_CHAR_SENT 'A'
#define FIRST_INT_ACK_SENT 100

#define FIRST_DEPORTED_DATA 'a'
#define STR_SIZE_OFFSET 2 /** offset add to the send number for calculate the size of the string to send */

#define RECV_TIMEOUT_MS 10
#define PORT1 12345
#define PORT2 54321
#define ADRR_IP "127.0.0.1"

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define NB_OF_INPUT_NET1 4
#define NB_OF_OUTPUT_NET1 4
#define NB_OF_INPUT_NET2 4
#define NB_OF_OUTPUT_NET2 4

#define SENDDATA 1
#define SENDDATADEPORT 1

/** define of the ioBuffer identifiers */
typedef enum eID_BUFF
{
    ID_CHAR_DATA = 5,
    ID_INT_DATA_WITH_ACK,
    ID_INT_DATA,
    ID_DEPORT_DATA,
    ID_DEPORT_DATA_ACK
    
}eID_BUFF;

typedef struct managerCheck_t
{
    network_manager_t* pManager;
    
    sal_thread_t thread_managerSend;
    sal_thread_t thread_managerRecv;
    
    int thRecvCheckAlive; /** life flag of the reading thread */
    
    sal_thread_t thread_checkSend;
    sal_thread_t thread_checkRecv;
    sal_thread_t thread_checkRecvDeported;
    
    int sentDataNumber; /**< number of the data not acknowledged sent */
    int sentDataAckNumber; /**< number of the data acknowledged sent */
    int sentDataDeportedNumber; /**< number of the data deported not acknowledged sent */
    int sentDataDeportedAckNumber; /**< number of the data deported acknowledged sent */
    
    int recvDataNumber; /**< number of the data not acknowledged receved */
    int recvDataAckNumber; /**< number of the data acknowledged receved */
    int recvDataDeportedNumber; /**< number of the data deported not acknowledged receved */
    int recvDataDeportedAckNumber; /**< number of the data deported acknowledged receved */
    
    char lastDataRecv; /**< last date not acknowledged receved */
    int lastDataDeportedSize; /**< last size of the date deported not acknowledged receved */
    
    int numberOfError; /**< number of cheking error */
    
}managerCheck_t;

void initManagerCheck(managerCheck_t* pManagerCheck);

void initParamIoBuffer( network_paramNewIoBuffer_t* pTabInput1, 
                        network_paramNewIoBuffer_t* pTabOutput1,
                        network_paramNewIoBuffer_t* pTabInput2, 
                        network_paramNewIoBuffer_t* pTabOutput2 );
                        
int callbackDepotData(int OutBufferId, uint8_t* pData, void* customData, int status);
                        
void* runCheckSendData(void*);
void* runCheckReadData(void*);
void* runCheckReadDataDeported(void* data);

eNETWORK_Error sendData(managerCheck_t* pManagerCheck);
eNETWORK_Error sendDataAck(managerCheck_t* pManagerCheck);
eNETWORK_Error sendDataDeported(managerCheck_t* pManagerCheck);
eNETWORK_Error sendDataDeportedAck(managerCheck_t* pManagerCheck);

char* allocInitStr(  int size );

int checkData( managerCheck_t* pManagerCheck, char data );
int checkDataACK( managerCheck_t* pManagerCheck, int dataAck );
int checkDeportedData( managerCheck_t* pManagerCheck, int dataSize );
int checkDeportedDataACK( managerCheck_t* pManagerCheck, char* pDataDeportedAck, int dataSizeAck );


/*****************************************
 * 
 *          implementation :
 *
******************************************/

int main(int argc, char *argv[])
{
    /** local declarations */
    managerCheck_t managerCheck1;
    managerCheck_t managerCheck2;
    
    eNETWORK_Error error = NETWORK_OK;
    int ackTransM1toM2Dif = 0;
    int ackDepTransM1toM2Dif = 0;
    int ackTransM2toM1Dif = 0;
    int ackDepTransM2toM1Dif = 0;
    
    int ret = 0;

    network_paramNewIoBuffer_t paramInputNetwork1[NB_OF_INPUT_NET1];
    network_paramNewIoBuffer_t paramOutputNetwork1[NB_OF_OUTPUT_NET1];
    
    network_paramNewIoBuffer_t paramInputNetwork2[NB_OF_INPUT_NET2];
    network_paramNewIoBuffer_t paramOutputNetwork2[NB_OF_OUTPUT_NET2];
    
    /** default init */
    initManagerCheck( &managerCheck1 );
    initManagerCheck( &managerCheck2 );
    initParamIoBuffer( paramInputNetwork1, paramOutputNetwork1, paramInputNetwork2, paramOutputNetwork2 );
    /** initialize random seed: */
    srand ( time(NULL) );
    
    printf(" -- libNetWork TestBench auto -- \n");
    
    /** create the Manager1 */
    managerCheck1.pManager = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE,
                                                 NB_OF_INPUT_NET1, paramInputNetwork1,
                                                 NB_OF_OUTPUT_NET1, paramOutputNetwork1, &error );
    
    if( error == NETWORK_OK )
    {
        error = NETWORK_ManagerSocketsInit(managerCheck1.pManager, ADRR_IP, PORT1, PORT2, RECEIVER_TIMEOUT_SEC);
        if(error != NETWORK_OK)
        {
            printf("managerCheck1.pManager error initsocket = %d \n", error);
        }
    }

    /** create the Manager2 */
    if( error == NETWORK_OK )
    {
        managerCheck2.pManager = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE,
                                                     NB_OF_INPUT_NET2, paramInputNetwork2,
                                                     NB_OF_OUTPUT_NET2, paramOutputNetwork2, &error );  
    }
    
    if( error == NETWORK_OK )
    {
        error = NETWORK_ManagerSocketsInit(managerCheck2.pManager, ADRR_IP, PORT2, PORT1, RECEIVER_TIMEOUT_SEC);
        if(error != NETWORK_OK)
        {
            printf("managerCheck2.pManager error initsocket = %d \n ", error);
        }
    }
    
    if( error == NETWORK_OK )
    {
        printf("check start \n");
        
        /** create the threads */
        sal_thread_create( &(managerCheck2.thread_managerRecv), 
                           (sal_thread_routine) NETWORK_ManagerRunReceivingThread, 
                           managerCheck2.pManager );
        sal_thread_create( &(managerCheck1.thread_managerRecv), 
                           (sal_thread_routine) NETWORK_ManagerRunReceivingThread, 
                           managerCheck1.pManager );
        
        sal_thread_create( &(managerCheck1.thread_managerSend), 
                           (sal_thread_routine) NETWORK_ManagerRunSendingThread, 
                           managerCheck1.pManager );
        sal_thread_create( &(managerCheck2.thread_managerSend), 
                           (sal_thread_routine) NETWORK_ManagerRunSendingThread, 
                           managerCheck2.pManager );
        
        /** manager 1 to manager 2 */
        sal_thread_create( &(managerCheck1.thread_checkSend), 
                           (sal_thread_routine) runCheckSendData, 
                           &managerCheck1 );
        sal_thread_create( &(managerCheck2.thread_checkRecv), 
                           (sal_thread_routine) runCheckReadData, 
                           &managerCheck2 );
        sal_thread_create( &(managerCheck2.thread_checkRecvDeported), 
                           (sal_thread_routine) runCheckReadDataDeported, 
                           &managerCheck2 );
        
        /** manager 2 to manager 1 */
        sal_thread_create( &(managerCheck2.thread_checkSend), 
                           (sal_thread_routine) runCheckSendData, 
                           &managerCheck2 );
        sal_thread_create( &(managerCheck1.thread_checkRecv), 
                           (sal_thread_routine) runCheckReadData, 
                           &managerCheck1 );
        sal_thread_create( &(managerCheck1.thread_checkRecvDeported), 
                           (sal_thread_routine) runCheckReadDataDeported, 
                           &managerCheck1 );

        /** wait the end of the sending */
        if(managerCheck1.thread_checkSend != NULL)
        {
            sal_thread_join( managerCheck1.thread_checkSend, NULL );
        }
        
        if(managerCheck2.thread_checkSend != NULL)
        {
            sal_thread_join( managerCheck2.thread_checkSend, NULL );
        }

        /** wait for receiving the last data sent */
        usleep(SENDING_SLEEP_TIME_US);
        
        /** stop the reading */
        managerCheck2.thRecvCheckAlive = 0;
        if(managerCheck2.thread_checkRecv != NULL)
        {
            sal_thread_join( managerCheck2.thread_checkRecv, NULL );
        }
        if(managerCheck2.thread_checkRecvDeported != NULL)
        {
            sal_thread_join( managerCheck2.thread_checkRecvDeported, NULL );
        }
        
        managerCheck1.thRecvCheckAlive = 0;
        if(managerCheck1.thread_checkRecv != NULL)
        {
            sal_thread_join( managerCheck1.thread_checkRecv, NULL );
        }
        if(managerCheck1.thread_checkRecvDeported != NULL)
        {
            sal_thread_join( managerCheck1.thread_checkRecvDeported, NULL );
        }

        printf(" -- stop -- \n");
        
        /** stop all therad */
        NETWORK_ManagerStop(managerCheck1.pManager);
        NETWORK_ManagerStop(managerCheck2.pManager);
        
        printf("wait ... \n");
        
        /** kill all threads */
        if(managerCheck1.thread_managerSend != NULL)
        {
            sal_thread_join( managerCheck1.thread_managerSend, NULL );
        }
        if(managerCheck2.thread_managerSend != NULL)
        {
            sal_thread_join( managerCheck2.thread_managerSend, NULL );
        }
        
        if(managerCheck1.thread_managerRecv != NULL)
        {
            sal_thread_join( managerCheck1.thread_managerRecv, NULL );
        }
        
        if(managerCheck2.thread_managerRecv != NULL)
        {
            sal_thread_join( managerCheck2.thread_managerRecv, NULL );
        }
    }

    /** print result */

    printf("\n");
    
    printf(" -- managerCheck1 to managerCheck2 -- \n " );
    printf(" %d data sent | %d data receved \n ", managerCheck1.sentDataNumber, managerCheck2.recvDataNumber );
    printf(" %d dataACK sent | %d data receved \n ", managerCheck1.sentDataAckNumber, managerCheck2.recvDataAckNumber );
    printf(" %d dataDeported sent | %d dataDeported receved \n ", managerCheck1.sentDataDeportedNumber, managerCheck2.recvDataDeportedNumber );
    printf(" %d dataDeportedAck sent | %d dataDeportedAck receved \n ", managerCheck1.sentDataDeportedAckNumber, managerCheck2.recvDataDeportedAckNumber );
    printf(" number of transmission error: %d \n", managerCheck2.numberOfError);
    
    printf("\n");
    
    printf(" -- managerCheck2 to managerCheck1 -- \n " );
    printf(" %d data sent | %d data receved \n ", managerCheck2.sentDataNumber, managerCheck1.recvDataNumber );
    printf(" %d dataACK sent | %d data receved \n ", managerCheck2.sentDataAckNumber, managerCheck1.recvDataAckNumber );
    printf(" %d dataDeported sent | %d dataDeported receved \n ", managerCheck2.sentDataDeportedNumber, managerCheck1.recvDataDeportedNumber );
    printf(" %d dataDeportedAck sent | %d dataDeportedAck receved \n ", managerCheck2.sentDataDeportedAckNumber, managerCheck1.recvDataDeportedAckNumber );
    printf(" number of transmission error: %d \n", managerCheck1.numberOfError);

    printf("\n");

    /** global cheking */

    ackTransM1toM2Dif = managerCheck1.sentDataAckNumber - managerCheck2.recvDataAckNumber;
    ackDepTransM1toM2Dif = managerCheck1.sentDataDeportedAckNumber - managerCheck2.recvDataDeportedAckNumber;
    ackTransM2toM1Dif = managerCheck2.sentDataAckNumber - managerCheck1.recvDataAckNumber;
    ackDepTransM2toM1Dif = managerCheck2.sentDataDeportedAckNumber - managerCheck1.recvDataDeportedAckNumber;

    if( error == NETWORK_OK &&
        managerCheck1.numberOfError == 0 &&
        managerCheck2.numberOfError == 0 &&
        ackTransM1toM2Dif == 0 &&
        ackDepTransM1toM2Dif == 0 &&
        ackTransM2toM1Dif == 0 &&
        ackDepTransM2toM1Dif == 0 )
    {
        printf(" # -- Good result of the test bench -- # \n");
    }
    else
    {
        printf(" # -- Bad result of the test bench -- # \n\n");
        
        printf("    libNetWork error : %d \n", error);
        printf("    %d data acknowledged lost of manager 1 to manager 2 \n", ackTransM1toM2Dif);
        printf("    %d data deported acknowledged lost of manager 1 to manager 2 \n", ackDepTransM1toM2Dif);
        printf("    %d data acknowledged lost of manager 2 to manager 1 \n", ackTransM2toM1Dif);
        printf("    %d data deported acknowledged lost of manager 2 to manager 1 \n", ackDepTransM2toM1Dif);
        printf("    %d data corrupted of manager 1 to manager 2 \n", managerCheck2.numberOfError);
        printf("    %d data corrupted of manager 2 to manager 1 \n", managerCheck1.numberOfError);
        
        ret = -1;
    }

    printf("\n");
    printf("end \n");
    
    /** delete */
    sal_thread_destroy( &(managerCheck1.thread_managerSend) );
    sal_thread_destroy( &(managerCheck2.thread_managerSend) );
    sal_thread_destroy( &(managerCheck1.thread_managerRecv) );
    sal_thread_destroy( &(managerCheck2.thread_managerRecv) );
    
    sal_thread_destroy( &(managerCheck1.thread_checkSend) );
    sal_thread_destroy( &(managerCheck2.thread_checkRecv) );
    sal_thread_destroy( &(managerCheck2.thread_checkRecvDeported) );
    
    sal_thread_destroy( &(managerCheck2.thread_checkSend) );
    sal_thread_destroy( &(managerCheck1.thread_checkRecv) );
    sal_thread_destroy( &(managerCheck1.thread_checkRecvDeported) );
    
    NETWORK_DeleteManager( &(managerCheck1.pManager) );
    NETWORK_DeleteManager( &(managerCheck2.pManager) );

    return ret;
}

void initParamIoBuffer( network_paramNewIoBuffer_t* pTabInput1, 
                        network_paramNewIoBuffer_t* pTabOutput1,
                        network_paramNewIoBuffer_t* pTabInput2, 
                        network_paramNewIoBuffer_t* pTabOutput2)
{
    /** initialization of the buffer parameters */
    /** --- network 1 --- */
    
    /** input ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput1[0]) );
    pTabInput1[0].id = ID_CHAR_DATA;
    pTabInput1[0].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabInput1[0].numberOfCell = 1;
    pTabInput1[0].cellSize = sizeof(char);
    pTabInput1[0].isOverwriting = 1;
    
    /** input ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput1[1]) );
    pTabInput1[1].id = ID_INT_DATA_WITH_ACK;
    pTabInput1[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabInput1[1].sendingWaitTimeMs = 2;
    pTabInput1[1].ackTimeoutMs = 5;
    pTabInput1[1].nbOfRetry = NETWORK_IOBUFFER_INFINITE_NUMBER /*20*/;
    pTabInput1[1].numberOfCell = 5;
    pTabInput1[1].cellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput1[2]) );
    pTabInput1[2].id = ID_DEPORT_DATA;
    pTabInput1[2].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabInput1[2].sendingWaitTimeMs = 2;
    pTabInput1[2].numberOfCell = 5;
    pTabInput1[2].deportedData = 1;
    
    /** input ID_DEPORT_DATA_ACK */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput1[3]) );
    pTabInput1[3].id = ID_DEPORT_DATA_ACK;
    pTabInput1[3].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabInput1[3].sendingWaitTimeMs = 2;
    pTabInput1[3].ackTimeoutMs = 5;
    pTabInput1[3].nbOfRetry = NETWORK_IOBUFFER_INFINITE_NUMBER /*20*/;
    pTabInput1[3].numberOfCell = 5;
    pTabInput1[3].deportedData = 1;

    /** outputs: */
    
    /** output ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput1[0]) );
    pTabOutput1[0].id = ID_CHAR_DATA;
    pTabOutput1[0].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabOutput1[0].numberOfCell = 1;
    pTabOutput1[0].cellSize = sizeof(char);
    pTabOutput1[0].isOverwriting = 1;
    
    /** output ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput1[1]) );
    pTabOutput1[1].id = ID_INT_DATA_WITH_ACK;
    pTabOutput1[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabOutput1[1].sendingWaitTimeMs = 2;
    pTabOutput1[1].ackTimeoutMs = 5;
    pTabOutput1[1].nbOfRetry = NETWORK_IOBUFFER_INFINITE_NUMBER/*20*/;
    pTabOutput1[1].numberOfCell = 5;
    pTabOutput1[1].cellSize = sizeof(int);
    
    /** output ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput1[2]) );
    pTabOutput1[2].id = ID_DEPORT_DATA;
    pTabOutput1[2].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabOutput1[2].sendingWaitTimeMs = 2;
    pTabOutput1[2].numberOfCell = 5;
    pTabOutput1[2].deportedData = 1;
    
    /** output ID_DEPORT_DATA_ACK */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput1[3]) );
    pTabOutput1[3].id = ID_DEPORT_DATA_ACK;
    pTabOutput1[3].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabOutput1[3].sendingWaitTimeMs = 2;
    pTabOutput1[3].ackTimeoutMs = 5;
    pTabOutput1[3].nbOfRetry = NETWORK_IOBUFFER_INFINITE_NUMBER/*20*/;
    pTabOutput1[3].numberOfCell = 5;
    pTabOutput1[3].deportedData = 1;
    
    /** ----------------------------- */
    
    /**--- network 2 --- */
    
    /** input ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput2[0]) );
    pTabInput2[0].id = ID_CHAR_DATA;
    pTabInput2[0].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabInput2[0].numberOfCell = 1;
    pTabInput2[0].cellSize = sizeof(char);
    pTabInput2[0].isOverwriting = 1;
    
    /** input ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput2[1]) );
    pTabInput2[1].id = ID_INT_DATA_WITH_ACK;
    pTabInput2[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabInput2[1].sendingWaitTimeMs = 2;
    pTabInput2[1].ackTimeoutMs = 5;
    pTabInput2[1].nbOfRetry = NETWORK_IOBUFFER_INFINITE_NUMBER/*20*/;
    pTabInput2[1].numberOfCell = 5;
    pTabInput2[1].cellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput2[2]) );
    pTabInput2[2].id = ID_DEPORT_DATA;
    pTabInput2[2].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabInput2[2].sendingWaitTimeMs = 2;
    pTabInput2[2].numberOfCell = 5;
    pTabInput2[2].deportedData = 1;
    
    /** input ID_DEPORT_DATA_ACK */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabInput2[3]) );
    pTabInput2[3].id = ID_DEPORT_DATA_ACK;
    pTabInput2[3].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabInput2[3].sendingWaitTimeMs = 2;
    pTabInput2[3].ackTimeoutMs = 5;
    pTabInput2[3].nbOfRetry = NETWORK_IOBUFFER_INFINITE_NUMBER/*20*/;
    pTabInput2[3].numberOfCell = 5;
    pTabInput2[3].deportedData = 1;
    
    /** outputs: */
    
    /**  output ID_CHAR_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput2[0]) );
    pTabOutput2[0].id = ID_CHAR_DATA;
    pTabOutput2[0].dataType = NETWORK_FRAME_TYPE_DATA;
    pTabOutput2[0].sendingWaitTimeMs = 3;
    pTabOutput2[0].numberOfCell = 1;
    pTabOutput2[0].cellSize = sizeof(char);
    pTabOutput2[0].isOverwriting = 1;
    
    /** output ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput2[1]) );
    pTabOutput2[1].id = ID_INT_DATA_WITH_ACK;
    pTabOutput2[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabOutput2[1].numberOfCell = 5;
    pTabOutput2[1].cellSize = sizeof(int);
    
    /** output ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput2[2]) );
    pTabOutput2[2].id = ID_DEPORT_DATA;
    pTabOutput2[2].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabOutput2[2].numberOfCell = 5;
    pTabOutput2[2].deportedData = 1;
    
    /** output ID_DEPORT_DATA_ACK */
    NETWORK_ParamNewIoBufferDefaultInit( &(pTabOutput2[3]) );
    pTabOutput2[3].id = ID_DEPORT_DATA_ACK;
    pTabOutput2[3].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    pTabOutput2[3].numberOfCell = 5;
    pTabOutput2[3].deportedData = 1;
    
    /** ----------------------------- */
}

void initManagerCheck( managerCheck_t* pManagerCheck )
{
    /** -- intitialize the managerCheck -- */
    
    pManagerCheck->pManager = NULL;
    pManagerCheck->thread_managerSend = NULL;
    pManagerCheck->thread_managerRecv = NULL;
    
    pManagerCheck->thRecvCheckAlive  = 1;
    
    pManagerCheck->thread_checkSend = NULL;
    pManagerCheck->thread_checkRecv = NULL ;
    pManagerCheck->thread_checkRecvDeported = NULL;
    
    pManagerCheck->sentDataNumber = 0; 
    pManagerCheck->sentDataAckNumber = 0;
    pManagerCheck->sentDataDeportedNumber = 0;
    pManagerCheck->sentDataDeportedAckNumber = 0;
    
    pManagerCheck->recvDataNumber = 0;
    pManagerCheck->recvDataAckNumber = 0;
    pManagerCheck->recvDataDeportedNumber = 0;
    pManagerCheck->recvDataDeportedAckNumber = 0;
    
    pManagerCheck->lastDataRecv = FIRST_CHAR_SENT - 1;
    pManagerCheck->lastDataDeportedSize = -1;
    
    pManagerCheck->numberOfError = 0;
}

void* runCheckSendData(void* data)
{
    /** -- thread run send data -- */
    
    /** local declarations */
    managerCheck_t* pManagerCheck = (managerCheck_t*)data;
    int alive = 1;
    
    /** send while all data are sent */
    while( alive )
    {
        alive = 0;
        
        /** for all data type, if not all data are sent, try to send with fifty-fifty chance */
        
        if( pManagerCheck->sentDataNumber < NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                sendData( pManagerCheck );
            }
        }
        
        if( pManagerCheck->sentDataAckNumber < NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                sendDataAck( pManagerCheck );
            }
        }
        
        if( pManagerCheck->sentDataDeportedNumber < NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                sendDataDeported( pManagerCheck );
            }
        }
        
        if( pManagerCheck->sentDataDeportedAckNumber < NUMBER_DATA_SENT )
        {
            alive = 1;
            if( rand() % 2 )
            {
                sendDataDeportedAck( pManagerCheck );
            }
        }
        
        usleep(SENDING_SLEEP_TIME_US);
    }
    
    return NULL;
}

void* runCheckReadData(void* data)
{
    /** -- thread run read and check data -- */
    
    /** local declarations */
    managerCheck_t* pManagerCheck = (managerCheck_t*)data;
    
    char chData = 0;
    int intData = 0;
    
    while(pManagerCheck->thRecvCheckAlive)
    {
        /** checking */
        if( ! NETWORK_ManagerReadData(pManagerCheck->pManager, ID_CHAR_DATA, (uint8_t*) &chData) )
        {
            printf("- charData: %c \n", chData);
            if( checkData(pManagerCheck, chData) )
            {
                printf("error \n");
            }
        }
        
        if( ! NETWORK_ManagerReadData(pManagerCheck->pManager, ID_INT_DATA_WITH_ACK, (uint8_t*) &intData) )
        {
            printf("- ackInt: %d \n", intData);
            if( checkDataACK( pManagerCheck, intData ) )
            {
                printf("error \n");
            }
        }
        
        usleep(READING_SLEEP_TIME_US);
    }
    
    return NULL;
}

void* runCheckReadDataDeported(void* data)
{
    /** -- thread run read and check data deported -- */
    
    /** local declarations */
    managerCheck_t* pManagerCheck = (managerCheck_t*)data;
    
    int readSize = 0;
    char dataDeportedRead[ NUMBER_DATA_SENT + STR_SIZE_OFFSET ];
    char dataDeportedReadAck[ NUMBER_DATA_SENT + STR_SIZE_OFFSET ];
    
    while(pManagerCheck->thRecvCheckAlive)
    {
        /** checking */
        
        if( ! NETWORK_ManagerReadDeportedData( pManagerCheck->pManager,
                                               ID_DEPORT_DATA, 
                                               (uint8_t*) dataDeportedRead, 
                                               NUMBER_DATA_SENT + STR_SIZE_OFFSET,
                                               &readSize ) )
        {
            printf("- depData: %s \n",  dataDeportedRead );
            
            if( checkDeportedData( pManagerCheck, readSize) )
            {
                printf("error \n");
            }
        }
        
        if( ! NETWORK_ManagerReadDeportedData( pManagerCheck->pManager, 
                                               ID_DEPORT_DATA_ACK, 
                                               (uint8_t*) dataDeportedReadAck, 
                                               NUMBER_DATA_SENT + STR_SIZE_OFFSET,
                                               &readSize ) )
        {
            
            printf("- depDataACK: %s \n",  dataDeportedReadAck );
            
            if( checkDeportedDataACK( pManagerCheck, dataDeportedReadAck, readSize) )
            {
                printf("error \n");
            } 
        }
        
        usleep(READING_SLEEP_TIME_US);
    }
    
    return NULL;
}

char* allocInitStr( int size )
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
        pStr[ii] = ( FIRST_DEPORTED_DATA + ii );
    }
    
    /** end the string */
    pStr[size-1] = '\0' ;
    

    return pStr;
}

int callbackDepotData(int OutBufferId, uint8_t* pData, void* customData, int status)
{
    /** local declarations */
    int retry = 0;
    
    printf(" -- callbackDepotData status: %d ",status);
    
    switch(status)
    {
        case NETWORK_CALLBACK_STATUS_SENT :
        case NETWORK_CALLBACK_STATUS_SENT_WITH_ACK :
        case NETWORK_CALLBACK_STATUS_FREE :
            retry = 0;
            free(pData);
            printf(" free --\n");
        break;
        
        case NETWORK_CALLBACK_STATUS_TIMEOUT :
            retry = 1;
            printf(" retry --\n");
        break;
        
        default:
            printf(" default --\n");
        break;
    }
    
    return retry;
}

eNETWORK_Error sendData(managerCheck_t* pManagerCheck)
{
    /** -- send data not acknowledged -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    char chData = FIRST_CHAR_SENT + pManagerCheck->sentDataNumber;
    
    printf(" send char: %c \n",chData);
    error = NETWORK_ManagerSendData( pManagerCheck->pManager, ID_CHAR_DATA, (uint8_t*) &chData);
        
    if( error == NETWORK_OK)
    {
        /** increment Number of data sent*/
        ++( pManagerCheck->sentDataNumber );
    }
    else
    {
        printf(" error send char :%d \n", error);
    }
    
    return error;
}

eNETWORK_Error sendDataAck(managerCheck_t* pManagerCheck)
{
    /** -- send data acknowledged -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    int intData = FIRST_INT_ACK_SENT + pManagerCheck->sentDataAckNumber;
    
    printf(" send int: %d \n",intData);        
    error = NETWORK_ManagerSendData( pManagerCheck->pManager, ID_INT_DATA_WITH_ACK, (uint8_t*) &intData );
    
    if( error == NETWORK_OK)
    {
        /** increment Number of data acknowledged sent*/
        ++( pManagerCheck->sentDataAckNumber ); 
    }
    else
    {
        printf(" error send int ack :%d \n", error);
    }
    
    return error;
}

eNETWORK_Error sendDataDeported(managerCheck_t* pManagerCheck)
{
    /** -- send data deported not acknowledged -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    char* pStrDataDeported = NULL;
    int dataDeportSize = pManagerCheck->sentDataDeportedNumber + STR_SIZE_OFFSET;
    
    /** create DataDeported */
    pStrDataDeported = allocInitStr( dataDeportSize );
    
    printf(" send str: %s size: %d \n", pStrDataDeported, dataDeportSize);  
    error = NETWORK_ManagerSendDeportedData( pManagerCheck->pManager, ID_DEPORT_DATA,
                                             (uint8_t*) pStrDataDeported, 
                                             dataDeportSize,
                                             NULL,
                                             &(callbackDepotData) );
                                             
    if( error == NETWORK_OK)
    {
        /** increment Number of data deported not acknowledged sent*/
        ++( pManagerCheck->sentDataDeportedNumber );
    }
    else
    {
        printf(" error send deported data ack :%d \n", error);
    }
    
    return error;
}

eNETWORK_Error sendDataDeportedAck(managerCheck_t* pManagerCheck)
{
    /** -- send data deported acknowledged -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    char* pStrDataDeportedAck = NULL;
    int dataDeportSizeAck = pManagerCheck->sentDataDeportedAckNumber + STR_SIZE_OFFSET;
    
    /** create DataDeported */
    pStrDataDeportedAck = allocInitStr( dataDeportSizeAck );
    
    printf(" send str: %s size: %d \n", pStrDataDeportedAck, dataDeportSizeAck);
    
    error = NETWORK_ManagerSendDeportedData( pManagerCheck->pManager, ID_DEPORT_DATA_ACK,
                                             (uint8_t*) pStrDataDeportedAck, dataDeportSizeAck,
                                             NULL, &(callbackDepotData) );
    
    if( error == NETWORK_OK)
    {
        /** increment Number of data deported acknowledged sent*/
        ++( pManagerCheck->sentDataDeportedAckNumber );
    }
    else
    {
        printf(" error send deported data ack :%d \n", error);
    }
    
    return error;
}


int checkData( managerCheck_t* pManagerCheck, char data )
{
    /** -- check the data receved -- */
    
    /** local declarations */
    int error = 0;
    
    if( data > pManagerCheck->lastDataRecv )
    {
        pManagerCheck->lastDataRecv = data;
    }
    else
    {
        error = 1;
        /** increment the cheking error */
        ++(pManagerCheck->numberOfError);
    }
    
    /** increment data not acknowledged receved*/
    ++( pManagerCheck->recvDataNumber );
    
    return error;
}

int checkDataACK( managerCheck_t* pManagerCheck, int dataAck )
{
    /** -- check the data acknowledged receved -- */
    
    /** local declarations */
    int error = 0;
    int dataAckCheck = FIRST_INT_ACK_SENT + pManagerCheck->recvDataAckNumber;
    
    if( dataAckCheck != dataAck )
    {
        error = 1;
        /** increment the cheking error */
        ++(pManagerCheck->numberOfError);
    }
    
    /** increment data acknowledged receved*/
    ++( pManagerCheck->recvDataAckNumber );
    
    return error;
}

int checkDeportedData( managerCheck_t* pManagerCheck, int dataSize)
{
    /** -- check the data deported not acknowledged receved -- */
    
    /** local declarations */
    int error = 0;
    
    if( dataSize > pManagerCheck->lastDataDeportedSize )
    {
        pManagerCheck->lastDataDeportedSize = dataSize;
    }
    else
    {
        error = 1;
        /** increment the cheking error */
        ++(pManagerCheck->numberOfError);
    }
    
    /** increment data deported not acknowledged receved*/
    ++( pManagerCheck->recvDataDeportedNumber );
    
    return error;
}

int checkDeportedDataACK( managerCheck_t* pManagerCheck, char* pDataDeportedAck, int dataSizeAck )
{
    /** -- check the data deported acknowledged receved -- */
    
    /** local declarations */
    int error = 0;
    char* pCheckStr = NULL;
    int checkStrSize = pManagerCheck->recvDataDeportedAckNumber + 2;

    /** check the size of the data */
    if( dataSizeAck != checkStrSize )
    {
        error = 1;
        /** increment the cheking error */
        ++(pManagerCheck->numberOfError);
    }
    
    if(error == 0)
    {
        /** create DataDeportedAck check */
        pCheckStr = allocInitStr( checkStrSize );
        
        /** compare the data with the string expected */
        error = memcmp(pDataDeportedAck, pCheckStr, checkStrSize);
        
        /** free the checking string */
        free(pCheckStr);
    }
    
    if(error != 0)
    {
        /** increment the cheking error */
        ++(pManagerCheck->numberOfError);
    }
    
    /** increment data deported acknowledged receved*/
    ++( pManagerCheck->recvDataDeportedAckNumber );
    
    return error;
}
