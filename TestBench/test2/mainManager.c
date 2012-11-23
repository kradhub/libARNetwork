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

#include <libNetwork/common.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/manager.h>

#include <unistd.h>

#include "Includes/networkDef.h"

int main(int argc, char *argv[])
{
	
	network_manager_t* pManager1= NULL;
	
	int cmdType = -1;
	char chData = 0;
	
	int intData = 0;
	
	char IpAddress[16];
	int scanfReturn = 0;
	int bindError = -1;
	int connectError = -1;
	
	network_ioBuffer_t* pInOutTemp = NULL;
	
	//network_paramNewInOutBuffer_t paramNetwork1[4];
	network_paramNewInOutBuffer_t paramInputNetwork1[3];
	network_paramNewInOutBuffer_t paramOutputNetwork1[1];
	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
	paramInputNetwork1[0].id = ID_CHAR_DATA;
	paramInputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork1[0].sendingWaitTime = 3;
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
	paramInputNetwork1[1].nbOfRetry = -1 /*5*/;
	paramInputNetwork1[1].buffSize = 5;
	paramInputNetwork1[1].buffCellSize = sizeof(int);
	paramInputNetwork1[1].overwriting = 0;
	
	// input ID_KEEP_ALIVE char
	paramInputNetwork1[2].id = ID_KEEP_ALIVE;
	paramInputNetwork1[2].dataType = CMD_TYPE_KEEP_ALIVE;
	paramInputNetwork1[2].sendingWaitTime = 100;
	paramInputNetwork1[2].ackTimeoutMs = 10;//not used
	paramInputNetwork1[2].nbOfRetry = 5;//not used
	paramInputNetwork1[2].buffSize = 1;
	paramInputNetwork1[2].buffCellSize = sizeof(int);
	paramInputNetwork1[2].overwriting = 1;
	
	// output ID_INT_DATA int
	paramOutputNetwork1[0].id = ID_INT_DATA;
	paramOutputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork1[0].sendingWaitTime = 3;//not used
	paramOutputNetwork1[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork1[0].nbOfRetry = 5;//not used
	paramOutputNetwork1[0].buffSize = 10;
	paramOutputNetwork1[0].buffCellSize = sizeof(int);
	paramOutputNetwork1[0].overwriting = 1;		
		
				
	//pManager1 = NETWORK_NewManagerWithVarg( 256, 256, 1, 2/*3*/,
	//						paramNetwork1[3],
	//						paramNetwork1[0], paramNetwork1[1]/*, paramNetwork1[2]*/);
	
	pManager1 = NETWORK_NewManager( 256, 256, 2/*3*/,paramInputNetwork1, 1,paramOutputNetwork1);
	
	
	printf("\n ~~ This soft sends data to the repeater soft ~~ \n \n");
	
	while(scanfReturn != 1 || bindError != 0 || connectError != 0)
	{
		sal_print(PRINT_WARNING,"repeater IP address ? : ");
		scanfReturn = scanf("%s",IpAddress);
		
		if(bindError != 0)
		{
			bindError = NETWORK_ReceiverBind(pManager1->pReceiver, 5552, 10);
		}
		
		if(connectError != 0)
		{
			connectError = NETWORK_SenderConnection(pManager1->pSender,IpAddress, 5551);
		}
		
		printf("	- Sender connect error: %d \n", connectError );			
		printf("	- Receiver Bind  error: %d \n", bindError );
		printf("\n");
	}

	sal_thread_t thread_send1;
	sal_thread_t thread_recv1;

	sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_RunReceivingThread, pManager1->pReceiver);
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_RunSendingThread, pManager1->pSender);
	
	while(cmdType != 0)
	{
		printf("press: 	1 - send char \n \
		2 - send int acknowledged \n \
		0 - quit \n");
	
		scanfReturn = scanf("%d",&cmdType);
		
		while ( ((chData = getchar()) != '\n') && chData != EOF)
		{
		};
		
		switch(cmdType)
		{
			case 0 :
				cmdType = 0; 
				printf("wait ... \n");
			break;
			
			case 1 :
			
				printf("char data ? ");
				scanfReturn = scanf("%c", &chData);
				printf("\n");
				
				pInOutTemp = inOutBufferWithId(	pManager1->ppTabInput, pManager1->numOfInput,
												ID_CHAR_DATA);
				ringBuffPushBack(pInOutTemp->pBuffer, &chData);
				
			break;
			
			case 2 :
				printf("int data acknowledged ?");
				scanfReturn = scanf("%d", &intData);
				printf("\n");
				
				pInOutTemp = inOutBufferWithId(	pManager1->ppTabInput,
										pManager1->numOfInput, ID_INT_DATA_WITH_ACK);
										
				ringBuffPushBack(pInOutTemp->pBuffer, &intData);
			break;
				
			default:
				cmdType = -1;
			break;
		}
	}
	
	//stop all therad
	NETWORK_StopSender(pManager1->pSender);
	NETWORK_StopReceiver(pManager1->pReceiver);
	
	//kill all thread
	
	sal_thread_join(&(thread_send1), NULL);
	sal_thread_join(&(thread_recv1), NULL);

	//delete
	NETWORK_DeleteManager( &pManager1 );
	
	printf("end\n");

	return 0;
}

