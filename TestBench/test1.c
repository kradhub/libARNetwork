/**
 *	@file main.h
 *  @brief Test
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

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

#define RING_BUFFER_SIZE 256
#define RING_BUFFER_CELL_SIZE 10

#define ID_SEND_RING_BUFF 1
#define ID_SEND_SING_BUFF 2

#define ID_RECV_RING_BUFF 1

typedef enum eID_BUFF
{
	ID_CHAR_DATA = 5,
	ID_INT_DATA_WITH_ACK,
	ID_INT_DATA
}eID_BUFF;


int main(int argc, char *argv[])
{
	
	network_t* pNetwork1= NULL;
	network_t* pNetwork2= NULL;
	
	int ii = 0;
	
	int error = 0;
	
	char chData = 0;
	int intData = 0;
	
	network_ioBuffer_t* pInOutTemp = NULL;
	
	network_paramNewInOutBuffer_t paramInputNetwork1[2];
	network_paramNewInOutBuffer_t paramOutputNetwork1[1];
	
	network_paramNewInOutBuffer_t paramInputNetwork2[1];
	network_paramNewInOutBuffer_t paramOutputNetwork2[2];
	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
	paramInputNetwork1[0].id = ID_CHAR_DATA;
	paramInputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork1[0].sendingWaitTime = 3;//not used
	paramInputNetwork1[0].ackTimeoutMs = 10;//not used
	paramInputNetwork1[0].nbOfRetry = 5;//not used
	paramInputNetwork1[0].buffSize = 1;
	paramInputNetwork1[0].buffCellSize = sizeof(char);
	paramInputNetwork1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
	paramInputNetwork1[1].id = ID_INT_DATA_WITH_ACK;
	paramInputNetwork1[1].dataType = CMD_TYPE_DATA_WITH_ACK;
	paramInputNetwork1[1].sendingWaitTime = 2;
	paramInputNetwork1[1].ackTimeoutMs = 10;
	paramInputNetwork1[1].nbOfRetry = -1/*20*/;
	paramInputNetwork1[1].buffSize = 5;
	paramInputNetwork1[1].buffCellSize = sizeof(int);
	paramInputNetwork1[1].overwriting = 0;
	
	// output ID_INT_DATA int
	paramOutputNetwork1[0].id = ID_INT_DATA;
	paramOutputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork1[0].sendingWaitTime = 3;//not used
	paramOutputNetwork1[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork1[0].nbOfRetry = 5;//not used
	paramOutputNetwork1[0].buffSize = 10;
	paramOutputNetwork1[0].buffCellSize = sizeof(int);
	paramOutputNetwork1[0].overwriting = 1;
	
	//-----------------------------
	
	//--- network 2 ---
	
	// input ID_INT_DATA char
	paramInputNetwork2[0].id = ID_INT_DATA;
	paramInputNetwork2[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork2[0].sendingWaitTime = 2;
	paramInputNetwork2[0].ackTimeoutMs = 10;//not used
	paramInputNetwork2[0].nbOfRetry = 5;//not used
	paramInputNetwork2[0].buffSize = 2;
	paramInputNetwork2[0].buffCellSize = sizeof(int);
	paramInputNetwork2[0].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramOutputNetwork2[0].id = ID_CHAR_DATA;
	paramOutputNetwork2[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork2[0].sendingWaitTime = 3;
	paramOutputNetwork2[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[0].nbOfRetry = 10;//not used
	paramOutputNetwork2[0].buffSize = 1;
	paramOutputNetwork2[0].buffCellSize = sizeof(char);
	paramOutputNetwork2[0].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
	paramOutputNetwork2[1].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramOutputNetwork2[1].sendingWaitTime = 3;//not used
	paramOutputNetwork2[1].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[1].nbOfRetry = 10;//not used
	paramOutputNetwork2[1].buffSize = 5;
	paramOutputNetwork2[1].buffCellSize = sizeof(int);
	paramOutputNetwork2[1].overwriting = 0;
	
	//-----------------------------
	
	pNetwork1 = newNetwork( 256, 256, 2, paramInputNetwork1, 1, paramOutputNetwork1);
							
	printf(" -pNetwork1->pSender connect error: %d \n", 
								NETWORK_SenderConnection(pNetwork1->pSender,"127.0.0.1", 5551) );
								
	printf(" -pNetwork1->pReceiver Bind  error: %d \n", 
								NETWORK_ReceiverBind(pNetwork1->pReceiver, 5552, 10) );

	pNetwork2 = newNetwork( 256, 256, 1, paramInputNetwork2, 2, paramOutputNetwork2);
							
	printf(" -pNetwork2->pReceiver Bind  error: %d \n",
								NETWORK_ReceiverBind(pNetwork2->pReceiver, 5551, 5) );
	printf(" -pNetwork2->pSender connect error: %d \n",
								NETWORK_SenderConnection(pNetwork2->pSender,"127.0.0.1", 5552) );
								
	sal_thread_t thread_send1 = NULL;
	sal_thread_t thread_recv1 = NULL;
	
	sal_thread_t thread_send2 = NULL;
	sal_thread_t thread_recv2 = NULL;
	
	printf("main start \n");
	
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) NETWORK_RunReceivingThread, pNetwork2->pReceiver);
	sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_RunReceivingThread, pNetwork1->pReceiver);
	
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_RunSendingThread, pNetwork1->pSender);
	sal_thread_create(&thread_send2, (sal_thread_routine) NETWORK_RunSendingThread, pNetwork2->pSender);

    for(ii = 0; ii < 5; ii++)
    {
		
		chData = ii;
		printf(" send char: %d \n",chData);
		error = networkSendData(pNetwork1, ID_CHAR_DATA, &chData);
		
		if( error )
		{
			printf(" error send char \n");
		}

		
		intData = ntohl (0x11223340 + ii);
		printf(" send int: %0x \n",0x11223340 + ii);		
		error = networkSendData(pNetwork1, ID_INT_DATA_WITH_ACK, &intData);
		
		if( error )
		{
			printf(" error send int ack \n");
		}
		
        usleep(50000);
    }
	
	printf(" -- stop-- \n");
	
	//stop all therad
	NETWORK_StopSender(pNetwork1->pSender);
	NETWORK_StopSender(pNetwork2->pSender);
	NETWORK_StopReceiver(pNetwork1->pReceiver);
	NETWORK_StopReceiver(pNetwork2->pReceiver);
	
	printf("\n the last char transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	printf("\n the integers transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	printf("\n");
	printf("wait ... \n");
	
	//kill all thread
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

	//delete
	deleteNetwork( &pNetwork1 );
	deleteNetwork( &pNetwork2 );

	printf("end \n");

	return 0;
}

