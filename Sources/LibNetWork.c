/**
 *	@file LibNetWork.c
 *  @brief AR commands processing, manage video stream reception,
 *	 photo reception and essential navdata demux
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libNetWork/LibNetWork.h>
#include <libNetWork/commun.h>//!! modif

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


void buffCmdAckInit(netWork_buffSend_t* pBuffsend)
{
	sal_print(PRINT_WARNING,"bufCmdAckInit \n");//!! sup
	
	pBuffsend->buffCmdAckNbData = 0;
	pBuffsend->buffIndexInput = 0;
	pBuffsend->buffIndexOutput = 0;
    sal_mutex_init( &(pBuffsend->mutex) );
	pBuffsend->buffCmdAck = malloc( sizeof(AR_CMD_ACK) * BUFFER_CMD_SIZE );
}

void buffCmdAckDelete(netWork_buffSend_t* pBuffsend)
{
	sal_print(PRINT_WARNING,"bufCmdAckDelete \n");//!! sup
	
	pBuffsend->buffCmdAckNbData = 0;
	pBuffsend->buffIndexOutput = 0;
	sal_mutex_destroy(&(pBuffsend->mutex));
	free(pBuffsend->buffCmdAck);
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

void buffPilotCmdInit(netWork_buffPilotCmd_t* buffPilotCmd)
{
	sal_print(PRINT_WARNING,"buffPilotCmdInit \n"); //!! sup
	
	buffPilotCmd->pilotCmd.x=0;//!!
	buffPilotCmd->pilotCmd.y=0;//!!
	buffPilotCmd->pilotCmd.z=0;//!!
    buffPilotCmd->isUpDated = 0;
    sal_mutex_init( &(buffPilotCmd->mutex) );
}

void updatePilotingCmd(netWork_buffPilotCmd_t* buffPilotCmd, AR_PILOT_CMD* cmd)
{
	sal_print(PRINT_WARNING,"updatePilotingCmd"); //!! sup
	
	sal_mutex_lock( &(buffPilotCmd->mutex) );
	
	buffPilotCmd->pilotCmd.x = cmd->x;//!!
	buffPilotCmd->pilotCmd.y = cmd->y;//!!
	buffPilotCmd->pilotCmd.z = cmd->z;//!!
	
	sal_print(PRINT_WARNING," ok x:%d, y:%d z:%d ",buffPilotCmd->pilotCmd.x,
		buffPilotCmd->pilotCmd.y, buffPilotCmd->pilotCmd.z); //!! sup
	
	buffPilotCmd->isUpDated = 1;
	sal_mutex_unlock( &(buffPilotCmd->mutex) );
	
	sal_print(PRINT_WARNING," \n "); //!! sup
}

void sendPilotingCmd(netWork_buffPilotCmd_t* buffPilotCmd)
{
	if(buffPilotCmd->isUpDated)
	{
		sal_print(PRINT_WARNING,"send \n"); //!! sup
	}
}
