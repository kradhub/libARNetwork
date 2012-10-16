/**
 *	@file singleBuffer.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libNetWork/singleBuffer.h>
#include <libNetWork/common.h>//!! modif

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

netWork_buffPilotCmd_t* newBuffPilotCmd()
{
	netWork_buffPilotCmd_t* buffPilotCmd = malloc( sizeof(netWork_buffPilotCmd_t));
	sal_print(PRINT_WARNING,"buffPilotCmdInit \n"); //!! sup
	
	if(buffPilotCmd)
	{
	
		buffPilotCmd->pilotCmd.x = 0;//!!
		buffPilotCmd->pilotCmd.y = 0;//!!
		buffPilotCmd->pilotCmd.z = 0;//!!
		buffPilotCmd->isUpDated = 0;
		sal_mutex_init( &(buffPilotCmd->mutex) );
    }
    
    return buffPilotCmd;
}

void deleteBuffPilotCmd(netWork_buffPilotCmd_t** ppBuffPilotCmd)
{	
	free(*ppBuffPilotCmd);
	*ppBuffPilotCmd = NULL;
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
	sal_print(PRINT_WARNING,"sendPilotingCmd isUpDated:%d" ,buffPilotCmd->isUpDated ); //!! sup
	
	if(buffPilotCmd->isUpDated)
	{
		sal_print(PRINT_WARNING,"send  x:%d, y:%d z:%d ",buffPilotCmd->pilotCmd.x,
		buffPilotCmd->pilotCmd.y, buffPilotCmd->pilotCmd.z); //!! sup
		
		buffPilotCmd->isUpDated = 0;
	}
	
	sal_print(PRINT_WARNING," \n "); //!! sup
}
