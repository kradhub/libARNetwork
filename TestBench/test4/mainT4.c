/**
 *	@file main.h
 *  @brief Test
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include <libSAL/print.h>
#include <libSAL/thread.h>

#include <libNetwork/frame.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/manager.h>

#include <unistd.h>

#include "Includes/networkDef.h"

#include <signal.h>

#include <termios.h>
 
/*****************************************
 * 
 * 			define :
 *
******************************************/
  
#define MILLISECOND 1000
#define RECV_TIMEOUT_MS 10
#define PORT1 5551
#define PORT2 5552

/*****************************************
 * 
 * 			header :
 *
******************************************/

typedef struct printThread
{
	network_ioBuffer_t* pOutBuffChar;
	network_ioBuffer_t* pOutBuffIntAck;
	int alive;
	
}printThread;

void* printBuff(void* data);
 
struct termios initial_settings, new_settings;
 
/*****************************************
 * 
 * 			implementation :
 *
******************************************/
 
void setupNonBlockingTerm ()
{
  new_settings = initial_settings;
  new_settings.c_lflag &= ~ICANON;
  new_settings.c_lflag &= ~ECHO;
 
  tcsetattr(0, TCSANOW, &new_settings);
}

void fixTerminal (int sig)
{
	tcsetattr(0, TCSANOW, &initial_settings);
	
	exit (0);
}

int main(int argc, char *argv[])
{
	tcgetattr(0,&initial_settings);	
	signal (SIGINT, fixTerminal);
		
	network_manager_t* pManager1= NULL;
	
	char netType = 0;
	
	char cmdType = 0;
	char chData = 0;
	
	int intData = 0;
	
	char IpAddress[16];
    int sendPort = 0;
	int recvPort = 0;
	int scanfReturn = 0;
	int connectError = -1;
	
	printThread printThread1;
	
	network_ioBuffer_t* pInputBuffChar = NULL;
	network_ioBuffer_t* pInputBuffIntAck = NULL;
	
	sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
	sal_thread_t thread_printBuff;
	
	network_paramNewIoBuffer_t paramNetworkL1[3];
	network_paramNewIoBuffer_t paramNetworkL2[2];

	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[0]) );
	paramNetworkL1[0].id = ID_CHAR_DATA;
	paramNetworkL1[0].dataType = network_frame_t_TYPE_DATA;
	paramNetworkL1[0].sendingWaitTime = 3;
	paramNetworkL1[0].buffSize = 1;
	paramNetworkL1[0].buffCellSize = sizeof(char);
	paramNetworkL1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[1]) );
	paramNetworkL1[1].id = ID_INT_DATA_WITH_ACK;
	paramNetworkL1[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL1[1].sendingWaitTime = 2;
	paramNetworkL1[1].ackTimeoutMs = 10;
	paramNetworkL1[1].nbOfRetry = -1 /*5*/;
	paramNetworkL1[1].buffSize = 5;
	paramNetworkL1[1].buffCellSize = sizeof(int);
	paramNetworkL1[1].overwriting = 0;
	
	// input ID_KEEP_ALIVE char
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[2]) );
	paramNetworkL1[2].id = ID_KEEP_ALIVE;
	paramNetworkL1[2].dataType = network_frame_t_TYPE_KEEP_ALIVE;
	paramNetworkL1[2].sendingWaitTime = 100;
	paramNetworkL1[2].buffSize = 1;
	paramNetworkL1[2].buffCellSize = sizeof(int);
	paramNetworkL1[2].overwriting = 1;
	
	//  ID_CHAR_DATA_2 int
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[0]) );
	paramNetworkL2[0].id = ID_CHAR_DATA_2;
	paramNetworkL2[0].dataType = network_frame_t_TYPE_DATA;
	paramNetworkL2[0].sendingWaitTime = 3;
	paramNetworkL2[0].buffSize = 1;
	paramNetworkL2[0].buffCellSize = sizeof(char);
	paramNetworkL2[0].overwriting = 1;
	
	//  ID_INT_DATA_WITH_ACK_2 char
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[1]) );
	paramNetworkL2[1].id = ID_INT_DATA_WITH_ACK_2;
	paramNetworkL2[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL2[1].sendingWaitTime = 2;
	paramNetworkL2[1].ackTimeoutMs = 10;
	paramNetworkL2[1].nbOfRetry = -1 /*5*/;
	paramNetworkL2[1].buffSize = 5;
	paramNetworkL2[1].buffCellSize = sizeof(int);
	paramNetworkL2[1].overwriting = 0;		
				
	printf("\n ~~ This soft sends data and repeater ack ~~ \n \n");
    
    pManager1 = NETWORK_NewManager( 256, 256, 2/*3*/, paramNetworkL1, 2 ,paramNetworkL2);
    
    printThread1.pOutBuffChar = NETWORK_IoBufferFromId(	pManager1->ppTabOutput, pManager1->numOfOutput, ID_CHAR_DATA_2);
    printThread1.pOutBuffIntAck = NETWORK_IoBufferFromId(	pManager1->ppTabOutput, pManager1->numOfOutput, ID_INT_DATA_WITH_ACK_2);
				
    pInputBuffChar  = NETWORK_IoBufferFromId(	pManager1->ppTabInput, pManager1->numOfInput, ID_CHAR_DATA);
    pInputBuffIntAck = NETWORK_IoBufferFromId(	pManager1->ppTabInput, pManager1->numOfInput, ID_INT_DATA_WITH_ACK);
	
	while(netType == 0)
	{
		sal_print(PRINT_WARNING,"type 1 or 2 ? : ");
		scanfReturn = scanf("%c",&netType);
		
		switch(netType)
		{
			case '1':

                sendPort = PORT1;
                recvPort = PORT2;
				
			break;
			
			case '2':
                
                sendPort = PORT2;
                recvPort = PORT1;
                
			break;
			
			default:
				cmdType = 0;
			break;
		}
	}	
	
	while(scanfReturn != 1 || connectError != 0)
	{
		sal_print(PRINT_WARNING,"repeater IP address ? : ");
		scanfReturn = scanf("%s",IpAddress);
        
        connectError = NETWORK_ManagerScoketsInit( pManager1, IpAddress, sendPort,
                                                    recvPort, RECV_TIMEOUT_MS );
		
		printf("    - connect error: %d \n", connectError );			
		printf("\n");
	}
	
	while ( ((chData = getchar()) != '\n') && chData != EOF)
	{
	};
	
	printThread1.alive=1;
	
	sal_thread_create(&thread_printBuff, (sal_thread_routine) printBuff, &printThread1 );
	sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_RunReceivingThread, pManager1->pReceiver);
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_RunSendingThread, pManager1->pSender);
	
	chData = 0;
	
	setupNonBlockingTerm ();
	
	while(cmdType != '0')
	{
		printf("press: 	1 - send char \n \
	2 - send int acknowledged \n \
	0 - quit \n");
	
		//scanfReturn = scanf("%d",&cmdType);
		
		cmdType = (char) getchar();
		
		switch(cmdType)
		{
			case '0' : 
				printf("wait ... \n");
			break;
			
			case '1' :
				++chData;
				printf("send char data :%d \n",chData);
				
				ringBuffPushBack(pInputBuffChar->pBuffer, &chData);
				
			break;
			
			case '2' :
				++intData;
				printf("int data acknowledged :%d \n",intData);
				printf("\n");
										
				if( ringBuffPushBack(pInputBuffIntAck->pBuffer, &intData) )
				{
					printf("buffer fulll \n");
				}
				
			break;
				
			default:
				cmdType = -1;
			break;
		}
	}
	
	//stop all therad
	printThread1.alive = 0;
	NETWORK_StopSender(pManager1->pSender);
	NETWORK_StopReceiver(pManager1->pReceiver);
	
	//kill all thread
	
	sal_thread_join(&(thread_send1), NULL);
	sal_thread_join(&(thread_recv1), NULL);
	sal_thread_join(&(thread_printBuff), NULL);

	//delete
	NETWORK_DeleteManager( &pManager1 );
	
	printf("end\n");

	fixTerminal (0);

	return 0;
}

void* printBuff(void* data)
{
	char chData = 0;
	int intData = 0;
	
	printThread* pprintThread1 = (printThread*) data;
	
	while(pprintThread1->alive)
	{
		usleep(MILLISECOND);

		while( !ringBuffPopFront(pprintThread1->pOutBuffChar->pBuffer, &chData) )
		{
			printf("- char :%d \n",chData);
		}
		
		while( !ringBuffPopFront(pprintThread1->pOutBuffIntAck->pBuffer, &intData) )
		{
			printf("- int ack :%d \n",intData);
		}
	}
	
	return NULL;
}
