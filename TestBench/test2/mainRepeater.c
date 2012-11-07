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

#include <libNetWork/common.h>
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/sender.h>
#include <libNetWork/receiver.h>
#include <libNetWork/netWork.h>

#include <unistd.h>

#include "Includes/netWorkDef.h"

int main(int argc, char *argv[])
{
	netWork_t* pNetWork2= NULL;
	
	char chData = 0;
	
	char IpAddress[16];
	int scanfReturn = 0;
	int bindError = -1;
	int connectError = -1;
	
	netWork_inOutBuffer_t* pInOutTemp = NULL;
	
	netWork_paramNewInOutBuffer_t paramNetWork2[3];
	
	
	//--- netWork 2 ---
	
	// input ID_INT_DATA char
	paramNetWork2[0].id = ID_INT_DATA;
	paramNetWork2[0].dataType = CMD_TYPE_DATA;
	paramNetWork2[0].sendingWaitTime = 2;
	paramNetWork2[0].ackTimeoutMs = 10;//not used
	paramNetWork2[0].nbOfRetry = 5;//not used
	paramNetWork2[0].buffSize = 2;
	paramNetWork2[0].buffCellSize = sizeof(int);
	paramNetWork2[1].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramNetWork2[1].id = ID_CHAR_DATA;
	paramNetWork2[1].dataType = CMD_TYPE_DATA;
	paramNetWork2[1].sendingWaitTime = 3;
	paramNetWork2[1].ackTimeoutMs = 10;//not used
	paramNetWork2[1].nbOfRetry = 5;//not used
	paramNetWork2[1].buffSize = 1;
	paramNetWork2[1].buffCellSize = sizeof(char);
	paramNetWork2[1].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramNetWork2[2].id = ID_INT_DATA_WITH_ACK;
	paramNetWork2[2].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramNetWork2[2].sendingWaitTime = 2;//not used
	paramNetWork2[2].ackTimeoutMs = 10;//not used
	paramNetWork2[2].nbOfRetry = 5;//not used
	paramNetWork2[2].buffSize = 5;
	paramNetWork2[2].buffCellSize = sizeof(int);
	paramNetWork2[2].overwriting = 0;
	
	//-----------------------------

	pNetWork2 = newNetWork( 256, 256, 2, 1,
							paramNetWork2[1], paramNetWork2[2],
							paramNetWork2[0]);
	
	printf("\n~~ This soft receives data sent by the manager soft ~~ \n \n");
						
	while(scanfReturn != 1 || bindError != 0 || connectError != 0)
	{
		printf("manager IP address ? : ");
		scanfReturn = scanf("%s",&IpAddress);
		
		if(bindError != 0)
		{
			bindError = receiverBind(pNetWork2->pReceiver, 5551, 10);
		}
		
		if(connectError != 0)
		{
			connectError = senderConnection(pNetWork2->pSender,&IpAddress, 5552);
		}
		
		printf("	- Sender connect error: %d \n", connectError );			
		printf("	- Receiver Bind  error: %d \n", bindError );
		printf("\n ");
	}				
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetWork2->pReceiver);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetWork2->pSender);

	printf("press 'q' to quit and to see the date received : \n");

    while(getchar() != 'q')
	{
		
    }
	
	printf("wait ... \n");
	
	//stop all therad
	stopSender(pNetWork2->pSender);
	stopReceiver(pNetWork2->pReceiver);
	
	printf("\n the last char transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetWork2->ppTabOutput, pNetWork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	printf("\n the integers transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetWork2->ppTabOutput, pNetWork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	sal_print(PRINT_WARNING,"\n");
	
	//kill all thread
	sal_thread_join(&(thread_send2), NULL);
	sal_thread_join(&(thread_recv2), NULL);

	//delete
	deleteNetWork( &pNetWork2 );
	
	printf("end \n");
}

