/**
 *	@file main.h
 *  @brief Test
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#include <stdlib.h> 
#include <libSAL/print.h>
#include <libSAL/thread.h>

#include <string.h>

//#include <libSAL/mutex.h>
#include <libNetWork/common.h>//!! modif
#include <libNetWork/inOutBuffer.h>
#include <libNetWork/sender.h>
#include <libNetWork/receiver.h>
#include <libNetWork/netWork.h>
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
	
	netWork_t* pNetWork1= NULL;
	netWork_t* pNetWork2= NULL;
	
	int i=0;
	char chData = 0;
	
	int intData = 0;
	
	netWork_inOutBuffer_t* pInOutTemp = NULL;
	
	netWork_paramNewInOutBuffer_t paramNetWork1[3];
	
	netWork_paramNewInOutBuffer_t paramNetWork2[3];
	
	
	mkfifo("wifi", 0666);
	
	//--- netWork 1 ---
	
	
	
	// input ID_CHAR_DATA int
	paramNetWork1[0].id = ID_CHAR_DATA;
	paramNetWork1[0].dataType = CMD_TYPE_DATA;
	paramNetWork1[0].sendingWaitTime = 3;//not used
	paramNetWork1[0].ackTimeoutMs = 10;//not used
	paramNetWork1[0].nbOfRetry = 5;//not used
	paramNetWork1[0].buffSize = 1;
	paramNetWork1[0].buffCellSize = sizeof(char);
	paramNetWork1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
	paramNetWork1[1].id = ID_INT_DATA_WITH_ACK;
	paramNetWork1[1].dataType = CMD_TYPE_DATA_WITH_ACK;
	paramNetWork1[1].sendingWaitTime = 2;
	paramNetWork1[1].ackTimeoutMs = 10;
	paramNetWork1[1].nbOfRetry = 5;
	paramNetWork1[1].buffSize = 5;
	paramNetWork1[1].buffCellSize = sizeof(int);
	paramNetWork1[1].overwriting = 0;
	
	// output ID_INT_DATA int
	paramNetWork1[2].id = ID_INT_DATA;
	paramNetWork1[2].dataType = CMD_TYPE_DATA;
	paramNetWork1[2].sendingWaitTime = 3;//not used
	paramNetWork1[2].ackTimeoutMs = 10;//not used
	paramNetWork1[2].nbOfRetry = 5;//not used
	paramNetWork1[2].buffSize = 10;
	paramNetWork1[2].buffCellSize = sizeof(int);
	paramNetWork1[2].overwriting = 1;
	
	//-----------------------------
	
	
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
	paramNetWork2[1].nbOfRetry = 10;//not used
	paramNetWork2[1].buffSize = 10;
	paramNetWork2[1].buffCellSize = sizeof(char);
	paramNetWork2[1].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramNetWork2[2].id = ID_INT_DATA_WITH_ACK;
	paramNetWork2[2].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramNetWork2[2].sendingWaitTime = 3;//not used
	paramNetWork2[2].ackTimeoutMs = 10;//not used
	paramNetWork2[2].nbOfRetry = 10;//not used
	paramNetWork2[2].buffSize = 5;
	paramNetWork2[2].buffCellSize = sizeof(int);
	paramNetWork2[2].overwriting = 0;
	
	//-----------------------------
						
						
	pNetWork1 = newNetWork( 256, 256, 1, 2,
							paramNetWork1[2],
							paramNetWork1[0], paramNetWork1[1]);
							
	sal_print( PRINT_WARNING," ------pNetWork1->pSender connect error: %d \n", 
								senderConnection(pNetWork1->pSender,"127.0.0.1", 5551) );
								
	sal_print( PRINT_WARNING," ------pNetWork1->pReceiver Bind  error: %d \n", 
								receiverBind(pNetWork1->pReceiver, 5552, 10) );
	
	pNetWork2 = newNetWork( 256, 256, 2, 1,
							paramNetWork2[1], paramNetWork2[2],
							paramNetWork2[0]);
							
	sal_print( PRINT_WARNING,	" ------pNetWork2->pReceiver Bind  error: %d \n",
								receiverBind(pNetWork2->pReceiver, 5551, 5) );
	sal_print( PRINT_WARNING,	" ------pNetWork2->pSender connect error: %d \n",
								senderConnection(pNetWork2->pSender,"127.0.0.1", 5552) );
								
	sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	
	sal_print(PRINT_WARNING,"main start \n");
	
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetWork2->pReceiver);
	sal_thread_create(&(thread_recv1), (sal_thread_routine) runReceivingThread, pNetWork1->pReceiver);

	usleep(50000);
	
	sal_print(PRINT_WARNING,"after wait 1s \n");
	
	sal_thread_create(&thread_send1, (sal_thread_routine) runSendingThread, pNetWork1->pSender);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetWork2->pSender);

    for(i=0 ; i<5 ; i++)
    {
		sal_print(PRINT_WARNING," \n --tic boucle i; %d \n",i);
		
		/*
		chData = ntohl (0x55 + i);
		pInOutTemp = inOutBufferWithId(	pNetWork1->ppTabInput, pNetWork1->numOfInput, ID_CHAR_DATA);

		ringBuffPushBack(pInOutTemp->pBuffer, &chData);
		
		sal_print(PRINT_WARNING," \n -- ID_CHAR_DATA data send : \n");
		ringBuffPrint(pInOutTemp->pBuffer);*/
		
		
		intData = ntohl (0x11223340 +i);
		pInOutTemp = inOutBufferWithId(	pNetWork1->ppTabInput,
										pNetWork1->numOfInput, ID_INT_DATA_WITH_ACK);
										
		ringBuffPushBack(pInOutTemp->pBuffer, &intData);
		
		sal_print(PRINT_WARNING," \n -- ID_INT_DATA_WITH_ACK data send : \n");
		//ringBuffPrint(pInOutTemp->pBuffer);
		
		
        usleep(50000);
    }
	
	sal_print(PRINT_WARNING," #### stop thread:\n");
	
	//stop all therad
	stopSender(pNetWork1->pSender);
	stopSender(pNetWork2->pSender);
	stopReceiver(pNetWork1->pReceiver);
	stopReceiver(pNetWork2->pReceiver);
	
	/*
	sal_print(PRINT_WARNING,"\n ID_CHAR_DATA transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetWork2->ppTabOutput, pNetWork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
	*/
	
	sal_print(PRINT_WARNING,"\n ID_INT_DATA_WITH_ACK transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetWork2->ppTabOutput, pNetWork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	
	sal_print(PRINT_WARNING,"\n");
	
	//kill all thread
	
	sal_thread_join(&(thread_send1), NULL);
	sal_thread_join(&(thread_send2), NULL);
	
	sal_thread_join(&(thread_recv1), NULL);
	sal_thread_join(&(thread_recv2), NULL);

	//delete
	deleteNetWork( &pNetWork1 );
	deleteNetWork( &pNetWork2 );
	
	remove("wifi");
	
	sal_print(PRINT_WARNING,"fin\n");

}

