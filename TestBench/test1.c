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
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/network.h>
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
	
	int i=0;
	char chData = 0;
	
	int intData = 0;
	
	network_inOutBuffer_t* pInOutTemp = NULL;
	
	network_paramNewInOutBuffer_t paramNetwork1[3];
	
	network_paramNewInOutBuffer_t paramNetwork2[3];
	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
	paramNetwork1[0].id = ID_CHAR_DATA;
	paramNetwork1[0].dataType = CMD_TYPE_DATA;
	paramNetwork1[0].sendingWaitTime = 3;//not used
	paramNetwork1[0].ackTimeoutMs = 10;//not used
	paramNetwork1[0].nbOfRetry = 5;//not used
	paramNetwork1[0].buffSize = 1;
	paramNetwork1[0].buffCellSize = sizeof(char);
	paramNetwork1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
	paramNetwork1[1].id = ID_INT_DATA_WITH_ACK;
	paramNetwork1[1].dataType = CMD_TYPE_DATA_WITH_ACK;
	paramNetwork1[1].sendingWaitTime = 2;
	paramNetwork1[1].ackTimeoutMs = 10;
	paramNetwork1[1].nbOfRetry = -1/*20*/;
	paramNetwork1[1].buffSize = 5;
	paramNetwork1[1].buffCellSize = sizeof(int);
	paramNetwork1[1].overwriting = 0;
	
	// output ID_INT_DATA int
	paramNetwork1[2].id = ID_INT_DATA;
	paramNetwork1[2].dataType = CMD_TYPE_DATA;
	paramNetwork1[2].sendingWaitTime = 3;//not used
	paramNetwork1[2].ackTimeoutMs = 10;//not used
	paramNetwork1[2].nbOfRetry = 5;//not used
	paramNetwork1[2].buffSize = 10;
	paramNetwork1[2].buffCellSize = sizeof(int);
	paramNetwork1[2].overwriting = 1;
	
	//-----------------------------
	
	
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
	paramNetwork2[1].nbOfRetry = 10;//not used
	paramNetwork2[1].buffSize = 1;
	paramNetwork2[1].buffCellSize = sizeof(char);
	paramNetwork2[1].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramNetwork2[2].id = ID_INT_DATA_WITH_ACK;
	paramNetwork2[2].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramNetwork2[2].sendingWaitTime = 3;//not used
	paramNetwork2[2].ackTimeoutMs = 10;//not used
	paramNetwork2[2].nbOfRetry = 10;//not used
	paramNetwork2[2].buffSize = 5;
	paramNetwork2[2].buffCellSize = sizeof(int);
	paramNetwork2[2].overwriting = 0;
	
	//-----------------------------
						
						
	pNetwork1 = newNetwork( 256, 256, 1, 2,
							paramNetwork1[2],
							paramNetwork1[0], paramNetwork1[1]);
							
	printf(" -pNetwork1->pSender connect error: %d \n", 
								senderConnection(pNetwork1->pSender,"127.0.0.1", 5551) );
								
	printf(" -pNetwork1->pReceiver Bind  error: %d \n", 
								receiverBind(pNetwork1->pReceiver, 5552, 10) );
	
	pNetwork2 = newNetwork( 256, 256, 2, 1,
							paramNetwork2[1], paramNetwork2[2],
							paramNetwork2[0]);
							
	printf(" -pNetwork2->pReceiver Bind  error: %d \n",
								receiverBind(pNetwork2->pReceiver, 5551, 5) );
	printf(" -pNetwork2->pSender connect error: %d \n",
								senderConnection(pNetwork2->pSender,"127.0.0.1", 5552) );
								
	sal_thread_t thread_send1 = NULL;
	sal_thread_t thread_recv1 = NULL;
	
	sal_thread_t thread_send2 = NULL;
	sal_thread_t thread_recv2 = NULL;
	
	printf("main start \n");
	
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetwork2->pReceiver);
	sal_thread_create(&(thread_recv1), (sal_thread_routine) runReceivingThread, pNetwork1->pReceiver);
	
	sal_thread_create(&thread_send1, (sal_thread_routine) runSendingThread, pNetwork1->pSender);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetwork2->pSender);

    for(i=0 ; i<5 ; i++)
    {
		
		//chData = ntohl (0x50 + i);
		chData =  i;
		printf(" send char: %d \n",chData);
		pInOutTemp = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_CHAR_DATA);
		ringBuffPushBack(pInOutTemp->pBuffer, &chData);

		
		intData = ntohl (0x11223340 +i);
		printf(" send int: %0x \n",0x11223340 +i);
		pInOutTemp = inOutBufferWithId(	pNetwork1->ppTabInput,
										pNetwork1->numOfInput, ID_INT_DATA_WITH_ACK);						
		ringBuffPushBack(pInOutTemp->pBuffer, &intData);
		
		
        usleep(50000);
    }
	
	printf(" -- stop-- \n");
	
	//stop all therad
	stopSender(pNetwork1->pSender);
	stopSender(pNetwork2->pSender);
	stopReceiver(pNetwork1->pReceiver);
	stopReceiver(pNetwork2->pReceiver);
	
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

