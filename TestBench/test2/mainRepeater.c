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

int main(int argc, char *argv[])
{
	network_t* pNetwork2= NULL;
	
	char chData = 0;
	
	char IpAddress[16];
	int scanfReturn = 0;
	int bindError = -1;
	int connectError = -1;
	
	network_inOutBuffer_t* pInOutTemp = NULL;
	
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
	
	printf("\n~~ This soft receives data sent by the manager soft ~~ \n \n");
						
	while(scanfReturn != 1 || bindError != 0 || connectError != 0)
	{
		printf("manager IP address ? :");
		scanfReturn = scanf("%s",&IpAddress);
		
		if(bindError != 0)
		{
			bindError = receiverBind(pNetwork2->pReceiver, 5551, 10);
		}
		
		if(connectError != 0)
		{
			connectError = senderConnection(pNetwork2->pSender,&IpAddress, 5552);
		}
		
		printf("	- Sender connect error: %d \n", connectError );			
		printf("	- Receiver Bind  error: %d \n", bindError );
		printf("\n");
	}				
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetwork2->pReceiver);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetwork2->pSender);

	printf("press 'q' to quit and to see the date received : \n");

    while(getchar() != 'q')
	{
		
    }
	
	//stop all therad
	stopSender(pNetwork2->pSender);
	stopReceiver(pNetwork2->pReceiver);
	
	printf("\n the last char transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	printf("\n the integers transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	sal_print(PRINT_WARNING,"\n");
	
	printf("wait ... \n");
	
	//kill all thread
	sal_thread_join(&(thread_send2), NULL);
	sal_thread_join(&(thread_recv2), NULL);

	//delete
	deleteNetwork( &pNetwork2 );
	
	printf("end \n");
}

