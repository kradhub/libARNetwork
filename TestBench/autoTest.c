/**
 *	@file main.h
 *  @brief libNetWork TestBench automatic
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

#include <string.h>

#include <libNetwork/common.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/manager.h>
#include <libSAL/socket.h>

#include <unistd.h>

/*****************************************
 * 
 * 			define :
 *
******************************************/

#define NUMBER_DATA_SENT 5
#define SENDING_SLEEP_TIME_US 50000
#define RECEIVER_TIMEOUT_SEC 5
#define FIRST_CHAR_SENT 'A'
#define FIRST_INT_ACK_SENT 100

#define RING_BUFFER_SIZE 256
#define RING_BUFFER_CELL_SIZE 10

/** define of the ioBuuffer identifiers */
typedef enum eID_BUFF
{
	ID_CHAR_DATA = 5,
	ID_INT_DATA_WITH_ACK,
	ID_INT_DATA
}eID_BUFF;

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

int main(int argc, char *argv[])
{
	/** local declarations */
	network_manager_t* pManager1= NULL;
	network_manager_t* pManager2= NULL;
    sal_thread_t thread_send1 = NULL;
	sal_thread_t thread_recv1 = NULL;
	sal_thread_t thread_send2 = NULL;
	sal_thread_t thread_recv2 = NULL;
	int ii = 0;
	int error = 0;
	char chData = 0;
	int intData = 0;
	
	network_paramNewInOutBuffer_t paramInputNetwork1[2];
	network_paramNewInOutBuffer_t paramOutputNetwork1[1];
	
	network_paramNewInOutBuffer_t paramInputNetwork2[1];
	network_paramNewInOutBuffer_t paramOutputNetwork2[2];
	
    /** initialization of the buffer parameters */
	/** --- network 1 --- */
	
	/** input ID_CHAR_DATA int */
    paramNewInOutBufferDefaultInit( &(paramInputNetwork1[0]) );
	paramInputNetwork1[0].id = ID_CHAR_DATA;
	paramInputNetwork1[0].dataType = network_frame_t_TYPE_DATA;
	paramInputNetwork1[0].buffSize = 1;
	paramInputNetwork1[0].buffCellSize = sizeof(char);
	paramInputNetwork1[0].overwriting = 1;
	
	/** input ID_INT_DATA_WITH_ACK char */
    paramNewInOutBufferDefaultInit( &(paramInputNetwork1[1]) );
	paramInputNetwork1[1].id = ID_INT_DATA_WITH_ACK;
	paramInputNetwork1[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramInputNetwork1[1].sendingWaitTime = 2;
	paramInputNetwork1[1].ackTimeoutMs = 10;
	paramInputNetwork1[1].nbOfRetry = -1/*20*/;
	paramInputNetwork1[1].buffSize = 5;
	paramInputNetwork1[1].buffCellSize = sizeof(int);
	paramInputNetwork1[1].overwriting = 0;
	
	/** output ID_INT_DATA int */
    paramNewInOutBufferDefaultInit( &(paramOutputNetwork1[0]) );
	paramOutputNetwork1[0].id = ID_INT_DATA;
	paramOutputNetwork1[0].dataType = network_frame_t_TYPE_DATA;
	paramOutputNetwork1[0].buffSize = 10;
	paramOutputNetwork1[0].buffCellSize = sizeof(int);
	paramOutputNetwork1[0].overwriting = 1;
	
	/** ----------------------------- */
	
	/**--- network 2 --- */
	
	/** input ID_INT_DATA char */
    paramNewInOutBufferDefaultInit( &(paramInputNetwork2[0]) );
	paramInputNetwork2[0].id = ID_INT_DATA;
	paramInputNetwork2[0].dataType = network_frame_t_TYPE_DATA;
	paramInputNetwork2[0].sendingWaitTime = 2;
	paramInputNetwork2[0].buffSize = 2;
	paramInputNetwork2[0].buffCellSize = sizeof(int);
	paramInputNetwork2[0].overwriting = 1;
	
	/**  output ID_CHAR_DATA int */
    paramNewInOutBufferDefaultInit( &(paramOutputNetwork2[0]) );
	paramOutputNetwork2[0].id = ID_CHAR_DATA;
	paramOutputNetwork2[0].dataType = network_frame_t_TYPE_DATA;
	paramOutputNetwork2[0].sendingWaitTime = 3;
	paramOutputNetwork2[0].buffSize = 1;
	paramOutputNetwork2[0].buffCellSize = sizeof(char);
	paramOutputNetwork2[0].overwriting = 1;
	
	/** output ID_INT_DATA_WITH_ACK int */
    paramNewInOutBufferDefaultInit( &(paramOutputNetwork2[1]) );
	paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
	paramOutputNetwork2[1].buffSize = 5;
	paramOutputNetwork2[1].buffCellSize = sizeof(int);
	paramOutputNetwork2[1].overwriting = 0;
	
	//-----------------------------
	
    printf(" -- libNetWork TestBench auto -- \n");
    
    /** create the Manger1 */
    
	pManager1 = NETWORK_NewManager( 256, 256, 2, paramInputNetwork1, 1, paramOutputNetwork1);
							
	printf(" -pManager1->pSender connect error: %d \n", 
                            NETWORK_SenderConnection(pManager1->pSender,"127.0.0.1", 5551) );
								
	printf(" -pManager1->pReceiver Bind  error: %d \n", 
                            NETWORK_ReceiverBind(pManager1->pReceiver, 5552, RECEIVER_TIMEOUT_SEC) );

    /** create the Manger2 */
	pManager2 = NETWORK_NewManager( 256, 256, 1, paramInputNetwork2, 2, paramOutputNetwork2);
							
	printf(" -pManager2->pReceiver Bind  error: %d \n",
                            NETWORK_ReceiverBind(pManager2->pReceiver, 5551, RECEIVER_TIMEOUT_SEC) );
	printf(" -pManager2->pSender connect error: %d \n",
                            NETWORK_SenderConnection(pManager2->pSender,"127.0.0.1", 5552) );
	
	printf("main start \n");
	
    /** create the threads */
	sal_thread_create(&(thread_recv2), (sal_thread_routine) NETWORK_RunReceivingThread, pManager2->pReceiver);
	sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_RunReceivingThread, pManager1->pReceiver);
	
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_RunSendingThread, pManager1->pSender);
	sal_thread_create(&thread_send2, (sal_thread_routine) NETWORK_RunSendingThread, pManager2->pSender);

    /** loop sending data */
    for(ii = 0; ii < NUMBER_DATA_SENT; ii++)
    {
		chData = FIRST_CHAR_SENT + ii;
		printf(" send char: %c \n",chData);
		error = NETWORK_ManagerSendData(pManager1, ID_CHAR_DATA, &chData);
		
		if( error )
		{
			printf(" error send char \n");
		}

		intData = FIRST_INT_ACK_SENT + ii;
		printf(" send int: %d \n",intData);		
		error = NETWORK_ManagerSendData(pManager1, ID_INT_DATA_WITH_ACK, &intData);
		
		if( error )
		{
			printf(" error send int ack \n");
		}
		
        usleep(SENDING_SLEEP_TIME_US);
    }
	
	printf(" -- stop-- \n");
	
	/** stop all therad */
	NETWORK_StopSender(pManager1->pSender);
	NETWORK_StopSender(pManager2->pSender);
	NETWORK_StopReceiver(pManager1->pReceiver);
	NETWORK_StopReceiver(pManager2->pReceiver);
	
	printf("\n the last char transmited:\n");
    while( ! NETWORK_ManagerReadData(pManager2, ID_CHAR_DATA, &chData) )
    {
        printf("- %c \n", chData);
        error = error || ( chData != ( FIRST_CHAR_SENT + (NUMBER_DATA_SENT - 1) ) );
    }
	
	printf("\n the integers transmited:\n");
    ii = 0;
    while( ! NETWORK_ManagerReadData(pManager2, ID_INT_DATA_WITH_ACK, &intData) )
    {
        printf("- %d \n", intData);
        error = error || ( intData != FIRST_INT_ACK_SENT + ii );
        ++ii;
    }
	
	printf("\n");
	printf("wait ... \n");
	
	/** kill all threads */
	if(thread_send1 != NULL)
	{
		sal_thread_join(&(thread_send1), NULL);
	}
	if(thread_send2 != NULL)
	{
		sal_thread_join(&(thread_send2), NULL);
	}
	
	if(thread_recv1 != NULL)
	{
		sal_thread_join(&(thread_recv1), NULL);
	}
	
	if(thread_recv2 != NULL)
	{
		sal_thread_join(&(thread_recv2), NULL);
	}

	/** delete */
	NETWORK_DeleteManager( &pManager1 );
	NETWORK_DeleteManager( &pManager2 );

    if(error)
    {
        printf("Bad result of the test bench \n");
    }
    else
    {
        printf("Good result of the test bench \n");
    }

	printf("end \n");

	return 0;
}

