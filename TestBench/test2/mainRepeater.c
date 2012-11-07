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

#include "Includes/netWorkDef.h"

int main(int argc, char *argv[])
{
	netWork_t* pNetWork2= NULL;
	
	char chData = 0;
	
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

	pNetWork2 = newNetWork( 256, 256, 2, 1,
							paramNetWork2[1], paramNetWork2[2],
							paramNetWork2[0]);
							
	sal_print( PRINT_WARNING,	" ------pNetWork2->pReceiver Bind  error: %d \n",
								receiverBind(pNetWork2->pReceiver, 5551, 10) );
	sal_print( PRINT_WARNING,	" ------pNetWork2->pSender connect error: %d \n",
								senderConnection(pNetWork2->pSender,"127.0.0.1", 5552) );
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	
	sal_print(PRINT_WARNING,"main start \n");
	
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetWork2->pReceiver);

	usleep(50000);

	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetWork2->pSender);

	sal_print(PRINT_WARNING," q to quit :\n");

    while(getchar() != 'q')
	{
		
    }
	
	sal_print(PRINT_WARNING," #### stop thread:\n");
	
	//stop all therad
	stopSender(pNetWork2->pSender);
	stopReceiver(pNetWork2->pReceiver);
	
	sal_print(PRINT_WARNING,"\n ID_CHAR_DATA transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetWork2->ppTabOutput, pNetWork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	sal_print(PRINT_WARNING,"\n ID_INT_DATA_WITH_ACK transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetWork2->ppTabOutput, pNetWork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	
	sal_print(PRINT_WARNING,"\n");
	
	//kill all thread
	
	sal_thread_join(&(thread_send2), NULL);
	sal_thread_join(&(thread_recv2), NULL);

	//delete
	deleteNetWork( &pNetWork2 );
	
	sal_print(PRINT_WARNING,"fin\n");
}

