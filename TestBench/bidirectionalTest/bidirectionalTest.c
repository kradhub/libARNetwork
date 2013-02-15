/**
 *  @file bidirectionalTest.h
 *  @brief Test
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

#include <libARSAL/ARSAL_Thread.h>

#include <libNetwork/manager.h>

#include <unistd.h>

#include "Includes/networkDef.h"

#include <signal.h>
#include <termios.h>
 
/*****************************************
 * 
 *             define :
 *
******************************************/
  
#define MILLISECOND 1000
#define RECV_TIMEOUT_MS 10
#define PORT1 12345
#define PORT2 54321

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define NB_OF_INPUT_L1 4
#define NB_OF_INPUT_L2 4

#define FIRST_DEPORTED_DATA 'A'
#define LIMIT_SIZE_DEPORT_DATA 27


/*****************************************
 * 
 *             header :
 *
******************************************/

typedef struct printThread
{
    network_manager_t* pManager;
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
    int id_ioBuff_deportData;
    int alive;
    
}printThread;

void* printBuff(void* data);

eNETWORK_CALLBACK_RETURN callbackDepotData(int OutBufferId,
                                               uint8_t* pData, 
                                               void* customData, 
                                               eNETWORK_CALLBACK_STATUS status);

/** terminal setting */
struct termios initial_settings, new_settings;
 
/*****************************************
 * 
 *             implementation :
 *
******************************************/
 
void setupNonBlockingTerm ()
{
    /** set the terminal on nonBloking mode */
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
 
  tcsetattr(0, TCSANOW, &new_settings);
}

void fixTerminal (int sig)
{
    /** reload terminal setting */
    tcsetattr(0, TCSANOW, &initial_settings);
    
    exit (0);
}

int main(int argc, char *argv[])
{
    network_manager_t* pManager1= NULL;
    
    char netType = 0;
    
    char cmdType = 0;
    char chData = 0;
    
    int intData = 0;
    int dataDeportSize_ack = 1;
    char* pDataDeported_ack;
    
    int dataDeportSize = 1;
    char* pDataDeported;
    
    int ii = 0;
    
    eNETWORK_Error error = NETWORK_OK;
    
    char IpAddress[16];
    int sendPort = 0;
    int recvPort = 0;
    int scanfReturn = 0;
    int connectError = -1;
    
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
    int id_ioBuff_deportData;
    
    printThread printThread1;
    
    ARSAL_Thread_t thread_send1;
    ARSAL_Thread_t thread_recv1;
    ARSAL_Thread_t thread_printBuff;
    
    network_paramNewIoBuffer_t paramNetworkL1[5];
    network_paramNewIoBuffer_t paramNetworkL2[4];
    
    /** save terminal setting */
    tcgetattr(0,&initial_settings);    
    /** call fixTerminal when the terminal kill the program */
    signal (SIGINT, fixTerminal);

    
    /** --- network 1 --- */
    
    /** input ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[0]) );
    paramNetworkL1[0].id = ID_CHAR_DATA;
    paramNetworkL1[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL1[0].sendingWaitTimeMs = 3;
    paramNetworkL1[0].numberOfCell = 1;
    paramNetworkL1[0].cellSize = sizeof(char);
    paramNetworkL1[0].isOverwriting = 1;
    
    /** input ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[1]) );
    paramNetworkL1[1].id = ID_INT_DATA_WITH_ACK;
    paramNetworkL1[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL1[1].sendingWaitTimeMs = 2;
    paramNetworkL1[1].ackTimeoutMs = 10;
    paramNetworkL1[1].nbOfRetry = 5;
    paramNetworkL1[1].numberOfCell = 5;
    paramNetworkL1[1].cellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA_ACK  */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[2]) );
    paramNetworkL1[2].id = ID_DEPORT_DATA_ACK;
    paramNetworkL1[2].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL1[2].sendingWaitTimeMs = 2;
    paramNetworkL1[2].ackTimeoutMs = 10;
    paramNetworkL1[2].nbOfRetry = 5;
    paramNetworkL1[2].numberOfCell = 5;
    paramNetworkL1[2].deportedData = 1;
    
    /** input ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[3]) );
    paramNetworkL1[3].id = ID_DEPORT_DATA;
    paramNetworkL1[3].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL1[3].sendingWaitTimeMs = 2;
    paramNetworkL1[3].numberOfCell = 5;
    paramNetworkL1[3].deportedData = 1;
    
    /** input ID_KEEP_ALIVE char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[4]) );
    paramNetworkL1[4].id = ID_KEEP_ALIVE;
    paramNetworkL1[4].dataType = NETWORK_FRAME_TYPE_KEEP_ALIVE;
    paramNetworkL1[4].sendingWaitTimeMs = 100;
    paramNetworkL1[4].numberOfCell = 1;
    paramNetworkL1[4].cellSize = sizeof(int);
    paramNetworkL1[4].isOverwriting = 1;
    
    /**  ID_CHAR_DATA_2 int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[0]) );
    paramNetworkL2[0].id = ID_CHAR_DATA_2;
    paramNetworkL2[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL2[0].sendingWaitTimeMs = 3;
    paramNetworkL2[0].numberOfCell = 1;
    paramNetworkL2[0].cellSize = sizeof(char);
    paramNetworkL2[0].isOverwriting = 1;
    
    /**  ID_INT_DATA_WITH_ACK_2 char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[1]) );
    paramNetworkL2[1].id = ID_INT_DATA_WITH_ACK_2;
    paramNetworkL2[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL2[1].sendingWaitTimeMs = 2;
    paramNetworkL2[1].ackTimeoutMs = 10;
    paramNetworkL2[1].nbOfRetry = -1 /*5*/;
    paramNetworkL2[1].numberOfCell = 5;
    paramNetworkL2[1].cellSize = sizeof(int);
    paramNetworkL2[1].isOverwriting = 0;    
    
    /** input ID_DEPORT_DATA_ACK_2  */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[2]) );
    paramNetworkL2[2].id = ID_DEPORT_DATA_ACK_2;
    paramNetworkL2[2].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL2[2].sendingWaitTimeMs = 2;
    paramNetworkL2[2].ackTimeoutMs = 10;
    paramNetworkL2[2].nbOfRetry = -1 /*5*/;
    paramNetworkL2[2].numberOfCell = 5;
    paramNetworkL2[2].deportedData = 1;    
    
    /** input ID_DEPORT_DATA_2 */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[3]) );
    paramNetworkL2[3].id = ID_DEPORT_DATA_2;
    paramNetworkL2[3].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL2[3].sendingWaitTimeMs = 2;
    paramNetworkL2[3].numberOfCell = 5;
    paramNetworkL2[3].deportedData = 1;    
                
    printf("\n ~~ This soft sends data and repeater ack ~~ \n \n");
    
    while(netType == 0)
    {
        printf("type 1 or 2 ? : ");
        scanfReturn = scanf("%c",&netType);
        
        switch(netType)
        {
            case '1':
                pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, 
                                                NB_OF_INPUT_L1, paramNetworkL1, 
                                                NB_OF_INPUT_L2,paramNetworkL2, 
                                                &error );
                
                id_ioBuff_char = ID_CHAR_DATA;
                id_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
                id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK;
                id_ioBuff_deportData = ID_DEPORT_DATA;
                
                printThread1.id_ioBuff_char = ID_CHAR_DATA_2;
                printThread1.id_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
                printThread1.id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK_2;
                printThread1.id_ioBuff_deportData = ID_DEPORT_DATA_2;
                
                sendPort = PORT1;
                recvPort = PORT2;
            break;
            
            case '2':
                pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, 
                                                NB_OF_INPUT_L2, paramNetworkL2, 
                                                NB_OF_INPUT_L1 ,paramNetworkL1, 
                                                &error );
                
                id_ioBuff_char = ID_CHAR_DATA_2;
                id_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
                id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK_2;
                id_ioBuff_deportData = ID_DEPORT_DATA_2;
                
                printThread1.id_ioBuff_char = ID_CHAR_DATA;
                printThread1.id_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
                printThread1.id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK;
                printThread1.id_ioBuff_deportData = ID_DEPORT_DATA;
                
                sendPort = PORT2;
                recvPort = PORT1;
            break;
            
            default:
                cmdType = 0;
            break;
        }
    }    
    
    if(error == NETWORK_OK)
    {
        printThread1.pManager = pManager1;
    }
    else
    {
        printf("manager error ");
        printThread1.pManager = NULL;
    }
    
    while(scanfReturn != 1 || connectError != 0)
    {
        printf("repeater IP address ? : ");
        scanfReturn = scanf("%s",IpAddress);
        
        connectError = NETWORK_ManagerSocketsInit( pManager1, IpAddress, sendPort,
                                                    recvPort, RECV_TIMEOUT_MS );
        
        printf("    - connect error: %d \n", connectError );            
        printf("\n");
    }
    
    while ( ((chData = getchar()) != '\n') && chData != EOF)
    {
        
    };
    
    printThread1.alive = 1;
    
    ARSAL_Thread_Create(&thread_printBuff, (ARSAL_Thread_Routine_t) printBuff, &printThread1 );
    ARSAL_Thread_Create(&(thread_recv1), (ARSAL_Thread_Routine_t) NETWORK_ManagerRunReceivingThread, pManager1);
    ARSAL_Thread_Create(&thread_send1, (ARSAL_Thread_Routine_t) NETWORK_ManagerRunSendingThread, pManager1);
    
    chData = 0;
    
    /** set the terminal on nonBloking mode */
    setupNonBlockingTerm ();
    
    while(cmdType != '0')
    {
        printf("press:  1 - send char \n \
    2 - send int acknowledged \n \
    3 - send string acknowledged \n \
    4 - send string \n \
    0 - quit \n");
        
        cmdType = (char) getchar();
        
        switch(cmdType)
        {
            case '0' : 
                printf("wait ... \n");
            break;
            
            case '1' :
                ++chData;
                printf("send char data :%d \n",chData);
            
                error = NETWORK_ManagerSendData(pManager1, id_ioBuff_char, (uint8_t*) &chData);
                if(error != NETWORK_OK )
                {
                    printf("buffer full \n");
                }
                
            break;
            
            case '2' :
                ++intData;
                printf("int data acknowledged :%d \n",intData);
                printf("\n");
                                        
                error = NETWORK_ManagerSendData(pManager1, id_ioBuff_intAck, (uint8_t*) &intData) ;
                if(error != NETWORK_OK )
                {
                    printf("buffer full \n");
                }
                
            break;
            
            case '3':
            
                if( dataDeportSize_ack < LIMIT_SIZE_DEPORT_DATA )
                {
                    ++dataDeportSize_ack;
                }
                else
                {
                    dataDeportSize_ack = 2;
                }
                
                pDataDeported_ack = malloc(dataDeportSize_ack);
                
                /** write data */
                for(ii = 0; ii < dataDeportSize_ack - 1 ; ++ii)
                {
                    pDataDeported_ack[ii] = (FIRST_DEPORTED_DATA + ii) ;
                }
                /** end the string */
                pDataDeported_ack[dataDeportSize_ack - 1] = '\0' ;
                
                error = NETWORK_ManagerSendDeportedData( pManager1, id_ioBuff_deportDataAck,
                                                         (uint8_t*) pDataDeported_ack, dataDeportSize_ack,
                                                         NULL,
                                                         &(callbackDepotData) );
                if(error != NETWORK_OK )
                {
                    printf("buffer full \n");
                }
                
            break;
            
            case '4':
            
                if( dataDeportSize < LIMIT_SIZE_DEPORT_DATA )
                {
                    ++dataDeportSize;
                }
                else
                {
                    dataDeportSize = 2;
                }
                
                pDataDeported = malloc(dataDeportSize);
                
                /** write data */
                for(ii = 0; ii < dataDeportSize - 1 ; ++ii)
                {
                    pDataDeported[ii] = (FIRST_DEPORTED_DATA + ii) ;
                }
                /** end the string */
                pDataDeported[dataDeportSize - 1] = '\0' ;
                
                error = NETWORK_ManagerSendDeportedData( pManager1, id_ioBuff_deportData,
                                                         (uint8_t*) pDataDeported, dataDeportSize,
                                                         NULL, &(callbackDepotData) );
                if(error != NETWORK_OK )
                {
                    printf("buffer full \n");
                }
                
            break;
            
            default:
                cmdType = -1;
            break;
        }
    }
    
    /** stop all therad */
    printThread1.alive = 0;
    NETWORK_ManagerStop(pManager1);
    
    /** kill all thread */
    ARSAL_Thread_Join(thread_send1, NULL);
    ARSAL_Thread_Join(thread_recv1, NULL);
    ARSAL_Thread_Join(thread_printBuff, NULL);

    /** delete */
    ARSAL_Thread_Destroy(&thread_send1);
    ARSAL_Thread_Destroy(&thread_recv1);
    ARSAL_Thread_Destroy(&thread_printBuff);
    NETWORK_DeleteManager( &pManager1 );
    
    printf("end\n");

    fixTerminal (0);

    return 0;
}

void* printBuff(void* data)
{
    char chData = 0;
    int intData = 0;
    char deportData[LIMIT_SIZE_DEPORT_DATA];
    eNETWORK_Error error = NETWORK_OK;
    
    printThread* pprintThread1 = (printThread*) data;
    
    while(pprintThread1->alive)
    {
        usleep(MILLISECOND);

        error = NETWORK_ManagerReadData( pprintThread1->pManager, pprintThread1->id_ioBuff_char, (uint8_t*) &chData ) ;
        if( error ==  NETWORK_OK )
        {
            printf("- char :%d \n", chData);
        }
        
        error = NETWORK_ManagerReadData( pprintThread1->pManager, pprintThread1->id_ioBuff_intAck, (uint8_t*) &intData );
        if( error ==  NETWORK_OK )
        {
            printf("- int ack :%d \n", intData);
        }
        
        error = NETWORK_ManagerReadDeportedData( pprintThread1->pManager, 
                                                 pprintThread1->id_ioBuff_deportDataAck, 
                                                 (uint8_t*) &deportData,LIMIT_SIZE_DEPORT_DATA, NULL );
        if( error ==  NETWORK_OK )
        {
            printf("- deport string data ack :%s \n", deportData);
        }
        
        error = NETWORK_ManagerReadDeportedData( pprintThread1->pManager, 
                                                 pprintThread1->id_ioBuff_deportData, 
                                                 (uint8_t*) &deportData,LIMIT_SIZE_DEPORT_DATA, NULL );
        if( error ==  NETWORK_OK )
        {
            printf("- deport string data :%s \n", deportData);
        }

    }
    
    return NULL;
}

eNETWORK_CALLBACK_RETURN callbackDepotData(int OutBufferId, 
                                              uint8_t* pData, 
                                              void* customData, 
                                              eNETWORK_CALLBACK_STATUS status)
{
    /** local declarations */
    eNETWORK_CALLBACK_RETURN retry = NETWORK_CALLBACK_RETURN_DEFAULT;
    
    printf(" -- callbackDepotData status: %d ",status);
    
    switch(status)
    {
        case NETWORK_CALLBACK_STATUS_SENT :
            printf(" callbackSENT --\n");
            retry = NETWORK_CALLBACK_RETURN_DEFAULT;
            free(pData);
            printf(" free --\n");
        
        break;
        
        case NETWORK_CALLBACK_STATUS_SENT_WITH_ACK :
            printf(" callbackSENTWithACK --\n");
            retry = NETWORK_CALLBACK_RETURN_DEFAULT;
            free(pData);
            printf(" free --\n");
            
        break;
        
        case NETWORK_CALLBACK_STATUS_FREE :
            retry = NETWORK_CALLBACK_RETURN_DEFAULT;
            free(pData);
            printf(" free --\n");
        break;
        
        case NETWORK_CALLBACK_STATUS_TIMEOUT :
            retry = NETWORK_CALLBACK_RETURN_RETRY;
            printf(" retry --\n");
        break;
        
        default:
            printf(" default --\n");
        break;
    }
    
    return retry;
}
