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
 * @file bidirectionalTest.h
 * @brief Test
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

#include <libARSAL/ARSAL_Thread.h>

#include <libARNetwork/ARNETWORK_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#include <unistd.h>

#include "Includes/networkDef.h"

#include <signal.h>
#include <termios.h>

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define BIDIR_PING_DELAY (0) // Use default value

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
 *****************************************/

typedef struct
{
    ARNETWORK_Manager_t* pManager;
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
    int id_ioBuff_deportData;
    int alive;

}printThread;

void* printBuff(void* data);

eARNETWORK_MANAGER_CALLBACK_RETURN callbackDepotData(int OutBufferId, uint8_t* dataPtr, void* customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

/** terminal setting */
struct termios initial_settings, new_settings;

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

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
    ARNETWORK_Manager_t* pManager1 = NULL;
    ARNETWORKAL_Manager_t *pOSSecificManager1 = NULL;
    char netType = 0;

    char cmdType = 0;
    char chData = 0;

    int intData = 0;
    int dataDeportSize_ack = 1;
    char* dataPtrDeported_ack;

    int dataDeportSize = 1;
    char* dataPtrDeported;

    int ii = 0;

    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORKAL_ERROR specificError = ARNETWORKAL_OK;
    eARNETWORKAL_ERROR connectError = ARNETWORKAL_OK;

    char IpAddress[16];
    int sendPort = 0;
    int recvPort = 0;
    int scanfReturn = 0;

    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
    int id_ioBuff_deportData;

    printThread printThread1;

    ARSAL_Thread_t thread_send1;
    ARSAL_Thread_t thread_recv1;
    ARSAL_Thread_t thread_printBuff;

    ARNETWORK_IOBufferParam_t paramNetworkL1[5];
    ARNETWORK_IOBufferParam_t paramNetworkL2[4];

    /** save terminal setting */
    tcgetattr(0,&initial_settings);
    /** call fixTerminal when the terminal kill the program */
    signal (SIGINT, fixTerminal);


    /** --- network 1 --- */

    /** input ID_CHAR_DATA char */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL1[0]) );
    paramNetworkL1[0].ID = ID_CHAR_DATA;
    paramNetworkL1[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    paramNetworkL1[0].sendingWaitTimeMs = 3;
    paramNetworkL1[0].numberOfCell = 1;
    paramNetworkL1[0].dataCopyMaxSize = sizeof(char);
    paramNetworkL1[0].isOverwriting = 1;

    /** input ID_INT_DATA_WITH_ACK int */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL1[1]) );
    paramNetworkL1[1].ID = ID_INT_DATA_WITH_ACK;
    paramNetworkL1[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL1[1].sendingWaitTimeMs = 2;
    paramNetworkL1[1].ackTimeoutMs = 10;
    paramNetworkL1[1].numberOfRetry = 5;
    paramNetworkL1[1].numberOfCell = 5;
    paramNetworkL1[1].dataCopyMaxSize = sizeof(int);

    /** input ID_DEPORT_DATA_ACK  */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL1[2]) );
    paramNetworkL1[2].ID = ID_DEPORT_DATA_ACK;
    paramNetworkL1[2].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL1[2].sendingWaitTimeMs = 2;
    paramNetworkL1[2].ackTimeoutMs = 10;
    paramNetworkL1[2].numberOfRetry = 5;
    paramNetworkL1[2].numberOfCell = 5;

    /** input ID_DEPORT_DATA */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL1[3]) );
    paramNetworkL1[3].ID = ID_DEPORT_DATA;
    paramNetworkL1[3].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    paramNetworkL1[3].sendingWaitTimeMs = 2;
    paramNetworkL1[3].numberOfCell = 5;

    /** input ID_DATA_LOW_LATENCY char */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL1[4]) );
    paramNetworkL1[4].ID = ID_DATA_LOW_LATENCY;
    paramNetworkL1[4].dataType = ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY;
    paramNetworkL1[4].sendingWaitTimeMs = 100;
    paramNetworkL1[4].numberOfCell = 1;
    paramNetworkL1[4].dataCopyMaxSize = sizeof(int);
    paramNetworkL1[4].isOverwriting = 1;

    /**  ID_CHAR_DATA_2 int */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL2[0]) );
    paramNetworkL2[0].ID = ID_CHAR_DATA_2;
    paramNetworkL2[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    paramNetworkL2[0].sendingWaitTimeMs = 3;
    paramNetworkL2[0].numberOfCell = 1;
    paramNetworkL2[0].dataCopyMaxSize = sizeof(char);
    paramNetworkL2[0].isOverwriting = 1;

    /**  ID_INT_DATA_WITH_ACK_2 char */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL2[1]) );
    paramNetworkL2[1].ID = ID_INT_DATA_WITH_ACK_2;
    paramNetworkL2[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL2[1].sendingWaitTimeMs = 2;
    paramNetworkL2[1].ackTimeoutMs = 10;
    paramNetworkL2[1].numberOfRetry = -1 /*5*/;
    paramNetworkL2[1].numberOfCell = 5;
    paramNetworkL2[1].dataCopyMaxSize = sizeof(int);
    paramNetworkL2[1].isOverwriting = 0;

    /** input ID_DEPORT_DATA_ACK_2  */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL2[2]) );
    paramNetworkL2[2].ID = ID_DEPORT_DATA_ACK_2;
    paramNetworkL2[2].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL2[2].sendingWaitTimeMs = 2;
    paramNetworkL2[2].ackTimeoutMs = 10;
    paramNetworkL2[2].numberOfRetry = -1 /*5*/;
    paramNetworkL2[2].numberOfCell = 5;

    /** input ID_DEPORT_DATA_2 */
    ARNETWORK_IOBufferParam_DefaultInit( &(paramNetworkL2[3]) );
    paramNetworkL2[3].ID = ID_DEPORT_DATA_2;
    paramNetworkL2[3].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
    paramNetworkL2[3].sendingWaitTimeMs = 2;
    paramNetworkL2[3].numberOfCell = 5;

    printf("\n ~~ This soft sends data and repeater ack ~~ \n \n");

    /** Initialize the OS Specific Network */
    pOSSecificManager1 = ARNETWORKAL_Manager_New(&specificError);
    if(specificError != ARNETWORKAL_OK)
    {
        printf("Error during OS specific creation : %d\n", specificError);
        return ARNETWORK_ERROR;
    }

    while(netType == 0)
    {
        printf("type 1 or 2 ? : ");
        scanfReturn = scanf("%c",&netType);

        switch(netType)
        {
        case '1':
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

    do
    {
        printf("repeater IP address ? : ");
        scanfReturn = scanf("%s",IpAddress);

        connectError = ARNETWORKAL_Manager_InitWifiNetwork(pOSSecificManager1, IpAddress, sendPort, recvPort, RECV_TIMEOUT_MS);
        printf("    - connect error: %d \n", connectError);
        printf("\n");
    }
    while((scanfReturn != 1) || (connectError != ARNETWORKAL_OK));

    switch(netType)
    {
    case 1 :
        pManager1 = ARNETWORK_Manager_New(pOSSecificManager1, NB_OF_INPUT_L1, paramNetworkL1, NB_OF_INPUT_L2,paramNetworkL2, BIDIR_PING_DELAY, NULL, NULL, &error);
        break;

    case 2 :
        pManager1 = ARNETWORK_Manager_New(pOSSecificManager1, NB_OF_INPUT_L2, paramNetworkL2, NB_OF_INPUT_L1 ,paramNetworkL1, BIDIR_PING_DELAY, NULL, NULL, &error);
        break;

    default:
        break;
    }

    if(error == ARNETWORK_OK)
    {
        printThread1.pManager = pManager1;
    }
    else
    {
        printf("manager error ");
        printThread1.pManager = NULL;
    }

    while ( ((chData = getchar()) != '\n') && chData != EOF)
    {

    };

    printThread1.alive = 1;

    ARSAL_Thread_Create(&thread_printBuff, (ARSAL_Thread_Routine_t) printBuff, &printThread1 );
    ARSAL_Thread_Create(&(thread_recv1), (ARSAL_Thread_Routine_t) ARNETWORK_Manager_ReceivingThreadRun, pManager1);
    ARSAL_Thread_Create(&thread_send1, (ARSAL_Thread_Routine_t) ARNETWORK_Manager_SendingThreadRun, pManager1);

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

            error = ARNETWORK_Manager_SendData(pManager1, id_ioBuff_char, (uint8_t*) &chData, sizeof(char), NULL, &(callbackDepotData),1); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if(error != ARNETWORK_OK )
            {
                printf("buffer full \n");
            }

            break;

        case '2' :
            ++intData;
            printf("int data acknowledged :%d \n",intData);
            printf("\n");

            error = ARNETWORK_Manager_SendData(pManager1, id_ioBuff_intAck, (uint8_t*) &intData, sizeof(char), NULL, &(callbackDepotData),1); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if(error != ARNETWORK_OK )
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

            dataPtrDeported_ack = malloc(dataDeportSize_ack);

            /** write data */
            for(ii = 0; ii < dataDeportSize_ack - 1 ; ++ii)
            {
                dataPtrDeported_ack[ii] = (FIRST_DEPORTED_DATA + ii) ;
            }
            /** end the string */
            dataPtrDeported_ack[dataDeportSize_ack - 1] = '\0' ;

            error = ARNETWORK_Manager_SendData(pManager1, id_ioBuff_deportDataAck, (uint8_t*) dataPtrDeported_ack, dataDeportSize_ack, NULL, &(callbackDepotData),0);
            if(error != ARNETWORK_OK )
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

            dataPtrDeported = malloc(dataDeportSize);

            /** write data */
            for(ii = 0; ii < dataDeportSize - 1 ; ++ii)
            {
                dataPtrDeported[ii] = (FIRST_DEPORTED_DATA + ii) ;
            }
            /** end the string */
            dataPtrDeported[dataDeportSize - 1] = '\0' ;

            error = ARNETWORK_Manager_SendData(pManager1, id_ioBuff_deportData, (uint8_t*) dataPtrDeported, dataDeportSize, NULL, &(callbackDepotData),0);
            if(error != ARNETWORK_OK )
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
    ARNETWORK_Manager_Stop(pManager1);

    /** kill all thread */
    ARSAL_Thread_Join(thread_send1, NULL);
    ARSAL_Thread_Join(thread_recv1, NULL);
    ARSAL_Thread_Join(thread_printBuff, NULL);

    ARNETWORKAL_Manager_CloseWifiNetwork(pOSSecificManager1);

    /** delete */
    ARSAL_Thread_Destroy(&thread_send1);
    ARSAL_Thread_Destroy(&thread_recv1);
    ARSAL_Thread_Destroy(&thread_printBuff);
    ARNETWORK_Manager_Delete( &pManager1 );

    printf("end\n");

    fixTerminal (0);

    return 0;
}

void* printBuff(void* data)
{
    char chData = 0;
    int intData = 0;
    char deportData[LIMIT_SIZE_DEPORT_DATA];
    eARNETWORK_ERROR error = ARNETWORK_OK;

    printThread* pprintThread1 = (printThread*) data;

    while(pprintThread1->alive)
    {
        usleep(MILLISECOND);

        error = ARNETWORK_Manager_TryReadData( pprintThread1->pManager, pprintThread1->id_ioBuff_char, (uint8_t*) &chData, sizeof(char), NULL); // !!!!!!!!!
        if( error ==  ARNETWORK_OK )
        {
            printf("- char :%d \n", chData);
        }

        error = ARNETWORK_Manager_TryReadData( pprintThread1->pManager, pprintThread1->id_ioBuff_intAck, (uint8_t*) &intData, sizeof(int), NULL);// !!!!!!!!!
        if( error ==  ARNETWORK_OK )
        {
            printf("- int ack :%d \n", intData);
        }

        error = ARNETWORK_Manager_TryReadData(pprintThread1->pManager, pprintThread1->id_ioBuff_deportDataAck, (uint8_t*) &deportData,LIMIT_SIZE_DEPORT_DATA, NULL); // !!!!!!!!
        if( error ==  ARNETWORK_OK )
        {
            printf("- deport string data ack :%s \n", deportData);
        }

        error = ARNETWORK_Manager_TryReadData(pprintThread1->pManager, pprintThread1->id_ioBuff_deportData, (uint8_t*) &deportData, LIMIT_SIZE_DEPORT_DATA, NULL); // !!!!!!!
        if( error ==  ARNETWORK_OK )
        {
            printf("- deport string data :%s \n", deportData);
        }

    }

    return NULL;
}

eARNETWORK_MANAGER_CALLBACK_RETURN callbackDepotData(int OutBufferId, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status)
{
    /** local declarations */
    eARNETWORK_MANAGER_CALLBACK_RETURN retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;

    printf(" -- callbackDepotData status: %d ",status);

    switch(status)
    {
    case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT :
        printf(" SENT --\n");
        retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;

        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_ACK_RECEIVED :
        printf(" ACK  received--\n");
        retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;

        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_FREE :
        printf(" free --\n");
        retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
        free(dataPtr);
        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT :
        retry = ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY;
        printf(" retry --\n");
        break;

    case ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL :
        printf(" cancel --\n");
        retry = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
        break;

    default:
        printf(" default --\n");
        break;
    }

    return retry;
}
