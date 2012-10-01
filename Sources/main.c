/**
 *	@file main.h
 *  @brief TEST
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#include <stdio.h> //!!sup 
#include <stdlib.h> //!!sup 
//#include <LibSAL.h>
#include "../Includes/LibNetWork.h"

int main(int argc, char *argv[])
{
	int i = 0;
	AR_CMD_ACK cmd;
	int error = 1;
	printf("debut \n");
	
	bufCmdAckInit();
	
	for(i=0; i<10 ; ++i)
	{
		cmd.CMDType = i%7;
		error = sendCmdWithAck(&cmd);
		printf("send %d \n", error );	
	}
	
	for(i=0; i<10 ; ++i)
	{
		bufferPush();
	}
	
	for(i=0; i<10 ; ++i)
	{
		cmd.CMDType = i%7;
		error = sendCmdWithAck(&cmd);
		printf("send %d \n", error );	
	}
	
	for(i=0; i<10 ; ++i)
	{
		bufferPush();
	}
	
	printf("fin\n");
	
}
