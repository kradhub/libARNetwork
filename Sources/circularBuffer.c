/**
 *	@file circularBuffer.c
 *  @brief circular buffer for the commands to send
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libNetWork/circularBuffer.h>
#include <libNetWork/common.h>//!! modif

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


netWork_buffSend_t* newBuffCmdAck()
{
	sal_print(PRINT_WARNING,"newBuffCmdAck \n");//!! sup
	
	netWork_buffSend_t* pBuffsend =  malloc( sizeof(netWork_buffSend_t));
	
	if(pBuffsend)
	{
		pBuffsend->buffCmdAckNbData = 0;
		pBuffsend->buffIndexInput = 0;
		pBuffsend->buffIndexOutput = 0;
		sal_mutex_init( &(pBuffsend->mutex) );
		pBuffsend->buffCmdAck = malloc( sizeof(AR_CMD_ACK) * BUFFER_CMD_SIZE );
		
		if( pBuffsend->buffCmdAck == NULL)
		{
			free(pBuffsend);
		}
	}
	
	return pBuffsend;
}

void deleteBuffCmdAck(netWork_buffSend_t** ppBuffsend)
{
	netWork_buffSend_t* pBuffsend = NULL;
	
	if(ppBuffsend)
	{
		pBuffsend = *ppBuffsend;
		
		sal_print(PRINT_WARNING,"deleteBuffCmdAck \n");//!! sup
	
		pBuffsend->buffCmdAckNbData = 0;
		pBuffsend->buffIndexOutput = 0;
		sal_mutex_destroy(&(pBuffsend->mutex));
		free(pBuffsend->buffCmdAck);
	
		free(pBuffsend);
		*ppBuffsend = NULL;
	}
}

int addAckCmd(netWork_buffSend_t* pBuffsend, AR_CMD_ACK* cmd)
{
	int error = 1; //!!
	AR_CMD_ACK* buffPointor = NULL;
	
	sal_print(PRINT_WARNING,"addAckCmd ");//!! sup
	
	sal_mutex_lock(&(pBuffsend->mutex));
	
	if( pBuffsend->buffCmdAckNbData < BUFFER_CMD_SIZE)
	{
		buffPointor = pBuffsend->buffCmdAck + (pBuffsend->buffIndexInput % BUFFER_CMD_SIZE);
		++(pBuffsend->buffIndexInput);
		++(pBuffsend->buffCmdAckNbData);
		
		buffPointor->CMDType = cmd->CMDType;
		
		sal_print(PRINT_WARNING," cmd :%d",cmd->CMDType);//!! sup
		
		error = 0; //!!
	}
	
	sal_mutex_unlock(&(pBuffsend->mutex));
	
	sal_print(PRINT_WARNING,"\n");//!! sup

	return error;
}

void sendAckCmd(netWork_buffSend_t* pBuffsend)
{
	AR_CMD_ACK* buffPointor = NULL;
	
	sal_mutex_lock(&(pBuffsend->mutex));
	
	if( pBuffsend->buffCmdAckNbData > 0)
	{
		buffPointor = pBuffsend->buffCmdAck + (pBuffsend->buffIndexOutput % BUFFER_CMD_SIZE);
		++(pBuffsend->buffIndexOutput);
		--(pBuffsend->buffCmdAckNbData);
		sal_print(PRINT_WARNING,"send data : %d \n", buffPointor->CMDType); //!! sup
	}
	
	sal_mutex_unlock(&(pBuffsend->mutex));
}


