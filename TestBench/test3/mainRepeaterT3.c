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
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/network.h>

#include <unistd.h>

#include "Includes/networkDef.h"



#define MILLISECOND 1000

typedef struct printThread
{
	network_inOutBuffer_t* pOutBuffChar;
	network_inOutBuffer_t* pOutBuffIntAck;
	int alive;
	
}printThread;


void* printBuff(void* data);






int main(int argc, char *argv[])
{
	
	
	network_t* pNetwork2= NULL;
	
	char IpAddress[16];
	int scanfReturn = 0;
	int bindError = -1;
	int connectError = -1;
	
	printThread printThread1;
	
	network_paramNewInOutBuffer_t paramNetwork2[3];
	
	
	//--- network 2 ---
	
	// input ID_INT_DATA char
	paramNetwork2[0].id = ID_INT_DATA;
	paramNetwork2[0].dataType = CMD_TYPE_DATA;
	paramNetwork2[0].sendingWaitTime = 2;
	paramNetwork2[0].ackTimeoutMs = 10;//not used
	paramNetwork2[0].nbOfRetry = 5;//not used
	paramNetwork2[0].buffSize = 2;
	paramNetwork2[0].buffCellSize = sizeof(int);
	paramNetwork2[1].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramNetwork2[1].id = ID_CHAR_DATA;
	paramNetwork2[1].dataType = CMD_TYPE_DATA;
	paramNetwork2[1].sendingWaitTime = 3;
	paramNetwork2[1].ackTimeoutMs = 10;//not used
	paramNetwork2[1].nbOfRetry = 5;//not used
	paramNetwork2[1].buffSize = 1;
	paramNetwork2[1].buffCellSize = sizeof(char);
	paramNetwork2[1].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramNetwork2[2].id = ID_INT_DATA_WITH_ACK;
	paramNetwork2[2].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramNetwork2[2].sendingWaitTime = 2;//not used
	paramNetwork2[2].ackTimeoutMs = 10;//not used
	paramNetwork2[2].nbOfRetry = 5;//not used
	paramNetwork2[2].buffSize = 5;
	paramNetwork2[2].buffCellSize = sizeof(int);
	paramNetwork2[2].overwriting = 0;
	
	//-----------------------------

	pNetwork2 = newNetwork( 256, 256, 2, 1,
							paramNetwork2[1], paramNetwork2[2],
							paramNetwork2[0]);
							
					
	printThread1.alive=1;		
	printThread1.pOutBuffChar = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA);
	printThread1.pOutBuffIntAck = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	
	printf("\n~~ This soft receives data sent by the manager soft ~~ \n \n");
						
	while(scanfReturn != 1 || bindError != 0 || connectError != 0)
	{
		printf("manager IP address ? :");
		scanfReturn = scanf("%s",IpAddress);
		
		if(bindError != 0)
		{
			bindError = receiverBind(pNetwork2->pReceiver, 5551, 10);
		}
		
		if(connectError != 0)
		{
			connectError = senderConnection(pNetwork2->pSender,IpAddress, 5552);
		}
		
		printf("	- Sender connect error: %d \n", connectError );			
		printf("	- Receiver Bind  error: %d \n", bindError );
		printf("\n");
	}				
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	sal_thread_t thread_printBuff;
	
	sal_thread_create(&thread_printBuff, (sal_thread_routine) printBuff, &printThread1 );
	sal_thread_create(&thread_recv2, (sal_thread_routine) runReceivingThread, pNetwork2->pReceiver);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetwork2->pSender);

	printf("press 'q' to quit : \n");
	
    while(getchar() != 'q')
	{
		
    }
	
	//stop all therad
	printThread1.alive = 0;
	stopSender(pNetwork2->pSender);
	stopReceiver(pNetwork2->pReceiver);
	
	printf("wait ... \n");
	
	//kill all thread
	sal_thread_join(&(thread_send2), NULL);
	sal_thread_join(&(thread_recv2), NULL);
	sal_thread_join(&(thread_printBuff), NULL);
	

	//delete
	deleteNetwork( &pNetwork2 );
	
	printf("end \n");
	
	
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
