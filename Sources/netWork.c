/**
 *	@file netWork.c
 *  @brief single buffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

//include :

#include <stdlib.h>
#include <libSAL/print.h>
#include <libSAL/mutex.h>
#include <libSAL/socket.h>
#include <libNetWork/sender.h>
#include <libNetWork/receiver.h>
#include <libNetWork/common.h>//!! modif
#include <libNetWork/netWork.h>

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

netWork_t* newNetWork()
{
	netWork_t* pNetWork = malloc( sizeof(netWork_t));
	sal_print(PRINT_WARNING,"newNetWork \n"); //!! sup
	
	if(pNetWork)
	{
		/*
		buffPilotCmd->pilotCmd.x = 0;//!!
		buffPilotCmd->pilotCmd.y = 0;//!!
		buffPilotCmd->pilotCmd.z = 0;//!!
		buffPilotCmd->isUpDated = 0;
		sal_mutex_init( &(buffPilotCmd->mutex) );
		* */
    }
    
    return pNetWork;
}

void deleteNetWork(netWork_t** ppNetWork)
{	
	free(*ppNetWork);
	*ppNetWork = NULL;
}


