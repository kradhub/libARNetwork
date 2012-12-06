/**
 *	@file bidirectionalTest.h
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

#include <libSAL/thread.h>

#include <libNetwork/frame.h>
#include <libNetwork/manager.h>
#include <libNetwork/paramNewIoBuffer.h>

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

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define NB_OF_INPUT_L1 3
#define NB_OF_INPUT_L2 3

#define FIRST_DEPORTED_DATA 'A'
#define LIMIT_SIZE_DEPORT_DATA 27


/*****************************************
 * 
 * 			header :
 *
******************************************/

typedef struct printThread
{
    network_manager_t* pManager;
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
	int alive;
	
}printThread;

void* printBuff(void* data);

int callBackDepotData(int OutBufferId, void* pData, int status);

/** terminal setting */
struct termios initial_settings, new_settings;
 
/*****************************************
 * 
 * 			implementation :
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
    int dataDeportSize = 1;
    char* pDataDeported;
    int ii = 0;
    
    int error = NETWORK_OK;
	
	char IpAddress[16];
    int sendPort = 0;
	int recvPort = 0;
	int scanfReturn = 0;
	int connectError = -1;
	
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
    
	printThread printThread1;
	
	sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
	sal_thread_t thread_printBuff;
	
	network_paramNewIoBuffer_t paramNetworkL1[4];
	network_paramNewIoBuffer_t paramNetworkL2[3];
    
    /** save terminal setting */
	tcgetattr(0,&initial_settings);	
    /** call fixTerminal when the terminal kill the program */
	signal (SIGINT, fixTerminal);

	
	/** --- network 1 --- */
	
	/** input ID_CHAR_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[0]) );
	paramNetworkL1[0].id = ID_CHAR_DATA;
	paramNetworkL1[0].dataType = network_frame_t_TYPE_DATA;
	paramNetworkL1[0].sendingWaitTime = 3;
	paramNetworkL1[0].buffSize = 1;
	paramNetworkL1[0].buffCellSize = sizeof(char);
	paramNetworkL1[0].overwriting = 1;
	
	/** input ID_INT_DATA_WITH_ACK char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[1]) );
	paramNetworkL1[1].id = ID_INT_DATA_WITH_ACK;
	paramNetworkL1[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL1[1].sendingWaitTime = 2;
	paramNetworkL1[1].ackTimeoutMs = 10;
	paramNetworkL1[1].nbOfRetry = -1 /*5*/;
	paramNetworkL1[1].buffSize = 5;
	paramNetworkL1[1].buffCellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA_ACK char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[2]) );
	paramNetworkL1[2].id = ID_DEPORT_DATA_ACK;
	paramNetworkL1[2].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL1[2].sendingWaitTime = 2;
	paramNetworkL1[2].ackTimeoutMs = 10;
	paramNetworkL1[2].nbOfRetry = -1 /*5*/;
	paramNetworkL1[2].buffSize = 5;
	paramNetworkL1[2].deportedData = 1;
	
	/** input ID_KEEP_ALIVE char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[3]) );
	paramNetworkL1[3].id = ID_KEEP_ALIVE;
	paramNetworkL1[3].dataType = network_frame_t_TYPE_KEEP_ALIVE;
	paramNetworkL1[3].sendingWaitTime = 100;
	paramNetworkL1[3].buffSize = 1;
	paramNetworkL1[3].buffCellSize = sizeof(int);
	paramNetworkL1[3].overwriting = 1;
	
	/**  ID_CHAR_DATA_2 int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[0]) );
	paramNetworkL2[0].id = ID_CHAR_DATA_2;
	paramNetworkL2[0].dataType = network_frame_t_TYPE_DATA;
	paramNetworkL2[0].sendingWaitTime = 3;
	paramNetworkL2[0].buffSize = 1;
	paramNetworkL2[0].buffCellSize = sizeof(char);
	paramNetworkL2[0].overwriting = 1;
	
	/**  ID_INT_DATA_WITH_ACK_2 char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[1]) );
	paramNetworkL2[1].id = ID_INT_DATA_WITH_ACK_2;
	paramNetworkL2[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL2[1].sendingWaitTime = 2;
	paramNetworkL2[1].ackTimeoutMs = 10;
	paramNetworkL2[1].nbOfRetry = -1 /*5*/;
	paramNetworkL2[1].buffSize = 5;
	paramNetworkL2[1].buffCellSize = sizeof(int);
	paramNetworkL2[1].overwriting = 0;	
    
    /** input ID_DEPORT_DATA_ACK_2 char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[2]) );
	paramNetworkL2[2].id = ID_DEPORT_DATA_ACK_2;
	paramNetworkL2[2].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL2[2].sendingWaitTime = 2;
	paramNetworkL2[2].ackTimeoutMs = 10;
	paramNetworkL2[2].nbOfRetry = -1 /*5*/;
	paramNetworkL2[2].buffSize = 5;
	paramNetworkL2[2].deportedData = 1;	
				
	printf("\n ~~ This soft sends data and repeater ack ~~ \n \n");
	
	while(netType == 0)
	{
		printf("type 1 or 2 ? : ");
		scanfReturn = scanf("%c",&netType);
		
		switch(netType)
		{
			case '1':
                pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, NB_OF_INPUT_L1, paramNetworkL1, NB_OF_INPUT_L2,paramNetworkL2);
                
                id_ioBuff_char = ID_CHAR_DATA;
                id_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
                id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK;
                
                printThread1.id_ioBuff_char = ID_CHAR_DATA_2;
                printThread1.id_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
                printThread1.id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK_2;
                
                sendPort = PORT1;
                recvPort = PORT2;
			break;
			
			case '2':
                pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, NB_OF_INPUT_L2, paramNetworkL2, NB_OF_INPUT_L1 ,paramNetworkL1);
                
                id_ioBuff_char = ID_CHAR_DATA_2;
                id_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
                id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK_2;
                
                printThread1.id_ioBuff_char = ID_CHAR_DATA;
                printThread1.id_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
                printThread1.id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK;
                
                sendPort = PORT2;
                recvPort = PORT1;
			break;
			
			default:
				cmdType = 0;
			break;
		}
	}	
    
    printThread1.pManager = pManager1;
	
	while(scanfReturn != 1 || connectError != 0)
	{
		printf("repeater IP address ? : ");
		scanfReturn = scanf("%s",IpAddress);
        
        connectError = NETWORK_ManagerScoketsInit( pManager1, IpAddress, sendPort,
                                                    recvPort, RECV_TIMEOUT_MS );
		
		printf("    - connect error: %d \n", connectError );			
		printf("\n");
	}
	
	while ( ((chData = getchar()) != '\n') && chData != EOF)
	{
        
	};
	
	printThread1.alive = 1;
	
	sal_thread_create(&thread_printBuff, (sal_thread_routine) printBuff, &printThread1 );
	sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_ManagerRunReceivingThread, pManager1);
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_ManagerRunSendingThread, pManager1);
	
	chData = 0;
	
    /** set the terminal on nonBloking mode */
	setupNonBlockingTerm ();
	
	while(cmdType != '0')
	{
		printf("press:  1 - send char \n \
    2 - send int acknowledged \n \
    3 - send string acknowledged \n \
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
            
                error = NETWORK_ManagerSendData(pManager1, id_ioBuff_char, &chData);
                if(error != NETWORK_OK )
                {
                    printf("buffer full \n");
                }
				
			break;
			
			case '2' :
				++intData;
				printf("int data acknowledged :%d \n",intData);
				printf("\n");
										
                error = NETWORK_ManagerSendData(pManager1, id_ioBuff_intAck, &intData) ;
                if(error != NETWORK_OK )
				{
					printf("buffer full \n");
				}
				
			break;
            
            case '3':
            
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
                
                error = NETWORK_ManagerSenddeportedData( pManager1, id_ioBuff_deportDataAck,
                                                         pDataDeported, dataDeportSize,
                                                         &(callBackDepotData) );
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
	
	sal_thread_join(&(thread_send1), NULL);
	sal_thread_join(&(thread_recv1), NULL);
	sal_thread_join(&(thread_printBuff), NULL);

	/** delete */
    sal_thread_destroy(&thread_send1);
    sal_thread_destroy(&thread_recv1);
    sal_thread_destroy(&thread_printBuff);
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
    int error = NETWORK_OK;
	
	printThread* pprintThread1 = (printThread*) data;
	
	while(pprintThread1->alive)
	{
		usleep(MILLISECOND);

        error = NETWORK_ManagerReadData(pprintThread1->pManager, pprintThread1->id_ioBuff_char, &chData) ;
		if( error ==  NETWORK_OK )
		{
			printf("- char :%d \n", chData);
		}
		
        error = NETWORK_ManagerReadData(pprintThread1->pManager, pprintThread1->id_ioBuff_intAck, &intData);
		if( error ==  NETWORK_OK )
		{
			printf("- int ack :%d \n", intData);
		}
        
        error = NETWORK_ManagerReaddeportedData(pprintThread1->pManager, 
                                                    pprintThread1->id_ioBuff_deportDataAck, 
                                                    &deportData,LIMIT_SIZE_DEPORT_DATA );
        
        if( error > 0 )
		{
			printf("- deport string data ack :%s \n", deportData);
		}
	}
	
	return NULL;
}


int callBackDepotData(int OutBufferId, void* pData, int status)
{
    /** local declarations */
    int error = 0;
    
    printf(" -- callBackDepotData -- \n");
    
    free(pData);
    
    return error;
}
