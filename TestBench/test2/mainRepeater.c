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

#include <libNetwork/frame.h>
#include <libNetwork/ioBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/manager.h>

#include <unistd.h>

#include "Includes/networkDef.h"

int main(int argc, char *argv[])
{
	network_manager_t* pManager2= NULL;
	
	char chData = 0;
	int intData = 0;
	
	char IpAddress[16];
	int scanfReturn = 0;
	int bindError = -1;
	int connectError = -1;
	
	network_ioBuffer_t* pInOutTemp = NULL;
	
	//network_paramNewIoBuffer_t paramNetwork2[3];
	network_paramNewIoBuffer_t paramInputNetwork2[1];
	network_paramNewIoBuffer_t paramOutputNetwork2[2];
	
	
	//--- network 2 ---
	
	// input ID_INT_DATA char
	paramInputNetwork2[0].id = ID_INT_DATA;
	paramInputNetwork2[0].dataType = network_frame_t_TYPE_DATA;
	paramInputNetwork2[0].sendingWaitTime = 2;
	paramInputNetwork2[0].ackTimeoutMs = 10;//not used
	paramInputNetwork2[0].nbOfRetry = 5;//not used
	paramInputNetwork2[0].buffSize = 2;
	paramInputNetwork2[0].buffCellSize = sizeof(int);
	paramInputNetwork2[1].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramOutputNetwork2[0].id = ID_CHAR_DATA;
	paramOutputNetwork2[0].dataType = network_frame_t_TYPE_DATA;
	paramOutputNetwork2[0].sendingWaitTime = 3;
	paramOutputNetwork2[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[0].nbOfRetry = 5;//not used
	paramOutputNetwork2[0].buffSize = 1;
	paramOutputNetwork2[0].buffCellSize = sizeof(char);
	paramOutputNetwork2[0].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
	paramOutputNetwork2[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;//not used
	paramOutputNetwork2[1].sendingWaitTime = 2;//not used
	paramOutputNetwork2[1].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[1].nbOfRetry = 5;//not used
	paramOutputNetwork2[1].buffSize = 5;
	paramOutputNetwork2[1].buffCellSize = sizeof(int);
	paramOutputNetwork2[1].overwriting = 0;
	
	//-----------------------------
	
	pManager2 = NETWORK_NewManager( 256, 256, 1, paramInputNetwork2, 2, paramOutputNetwork2);
							
	printf("\n~~ This soft receives data sent by the manager soft ~~ \n \n");
						
	while(scanfReturn != 1 || bindError != 0 || connectError != 0)
	{
		printf("manager IP address ? :");
		scanfReturn = scanf("%s",IpAddress);
		
		if(bindError != 0)
		{
			bindError = NETWORK_ReceiverBind(pManager2->pReceiver, 5551, 10);
		}
		
		if(connectError != 0)
		{
			connectError = NETWORK_SenderConnection(pManager2->pSender,IpAddress, 5552);
		}
		
		printf("	- Sender connect error: %d \n", connectError );			
		printf("	- Receiver Bind  error: %d \n", bindError );
		printf("\n");
	}				
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) NETWORK_RunReceivingThread, pManager2->pReceiver);
	sal_thread_create(&thread_send2, (sal_thread_routine) NETWORK_RunSendingThread, pManager2->pSender);

	printf("press 'q' to quit and to see the date received : \n");
	
    while(getchar() != 'q')
	{
		
    }
	
	//stop all therad
	NETWORK_StopSender(pManager2->pSender);
	NETWORK_StopReceiver(pManager2->pReceiver);
	
	printf("\n the last char transmited:\n");
	pInOutTemp = NETWORK_IoBufferWithId(	pManager2->ppTabOutput, pManager2->numOfOutput, ID_CHAR_DATA);
	//ringBuffPrint(pInOutTemp->pBuffer);

	
	while( !ringBuffPopFront(pInOutTemp->pBuffer, &chData) )
	{
		printf("- %c \n",chData);
	}
	
	printf("\n the integers transmited:\n");
	pInOutTemp = NETWORK_IoBufferWithId(	pManager2->ppTabOutput, pManager2->numOfOutput, ID_INT_DATA_WITH_ACK);
	//ringBuffPrint(pInOutTemp->pBuffer);
	while( !ringBuffPopFront(pInOutTemp->pBuffer, &intData) )
	{
		printf("- %d \n",intData);
	}
	
	printf("\n");
	
	printf("wait ... \n");
	
	//kill all thread
	sal_thread_join(&(thread_send2), NULL);
	sal_thread_join(&(thread_recv2), NULL);

	//delete
	NETWORK_DeleteManager( &pManager2 );
	
	printf("end \n");
	
	return 0;
}

