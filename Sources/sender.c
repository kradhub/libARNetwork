/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libNetWork/circularBuffer.h>
#include <libNetWork/singleBuffer.h> //rename
#include <libNetWork/sender.h>
#include <libNetWork/common.h>// !! modif

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_Sender_t* newSender()
{
	sal_print(PRINT_WARNING,"newSender \n");//!! sup
	
	netWork_Sender_t* pSender =  malloc( sizeof(netWork_Sender_t));
	
	if(pSender)
	{
		// !! alloc the buffer //???
		pSender->pNavCmdBuff = NULL;
		pSender->pPiliotCmsBuff = NULL;

		// !!! sal_mutex_init( &(pSender->mutex) ); //??
	}
	
	return pSender;
}

void deleteSender(netWork_Sender_t** ppSender)
{
	
}

void* runSendingThread(void* data)
{
	netWork_Sender_t* pSend = data;
	
	while( pSend->isAlive  )
	{
		sal_print(PRINT_WARNING," send \n");
		
		// !!! see the strat 
		// !!! temp:
		
		if(pSend->pNavCmdBuff)
		{
			sendAckCmd(pSend->pNavCmdBuff);
		}
		
		if(pSend->pPiliotCmsBuff)
		{
			sendAckCmd(pSend->pPiliotCmsBuff);
		}
		
		usleep(pSend->sleepTime);
	}
        
    return NULL;
}

