/**
 *	@file main.h
 *  @brief libNetWork TestBench automatic
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdio.h> 
#include <stdlib.h> 

#include <libSAL/thread.h>

#include <libNetwork/manager.h>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

/*****************************************
 * 
 * 			define :
 *
******************************************/

#define PORT1 12345
#define PORT2 54321

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define RECV_TIMEOUT_MS 10

/** terminal setting */
struct termios initial_settings, new_settings;

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

void setupNonBlockingTerm ()
{
    /** set the terminal on nonBloking mode */
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
 
  tcsetattr(0, TCSANOW, &new_settings);
}

void fixTerminal (int sig)
{
    /** reload terminal setting */
	tcsetattr(0, TCSANOW, &initial_settings);
	
	exit (0);
}

int main(int argc, char *argv[])
{
	/** local declarations */
	char scanChar = 0;
	int sendPort = 0;
	int recvPort = 0;
    network_manager_t* pManager = NULL;
    eNETWORK_Error error = NETWORK_OK;
    int connectError = 1;
    char IpAddress[16];
    
    sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
    
    /** save terminal setting */
	tcgetattr(0,&initial_settings);	
    /** call fixTerminal when the terminal kill the program */
	signal (SIGINT, fixTerminal);
	
	printf(" -- libNetWork TestBench default -- \n");
	
	while(scanChar == 0)
	{
		printf("type 1 or 2 ? : ");
		scanf("%c",&scanChar);
		
		switch(scanChar)
		{
			case '1':
                sendPort = PORT1;
	            recvPort = PORT2;
			break;
			
			case '2':
				sendPort = PORT2;
	            recvPort = PORT1;
			break;
			
			default:
				scanChar = 0;
			break;
		}
	}
	
    pManager = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, 0, NULL, 0,NULL, &error);
    
    if(error == NETWORK_OK )
    {
        while( connectError != 0)
        {
            printf("repeater IP address ? : ");
            scanf("%s",IpAddress);
            
            connectError = NETWORK_ManagerSocketsInit( pManager, IpAddress, sendPort,
                                                        recvPort, RECV_TIMEOUT_MS );
            
            printf("    - connect error: %d \n", connectError );			
            printf("\n");
        }
    }
    
    /** start threads */
    sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_ManagerRunReceivingThread, pManager);
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_ManagerRunSendingThread, pManager);
    
	while ( ((scanChar = getchar()) != '\n') && scanChar != EOF )
	{
        
	}
	
    /** set the terminal on nonBloking mode */
	setupNonBlockingTerm ();
    
    if(error == NETWORK_OK )
    {
        printf("press q to quit: \n");
        
        while(scanChar != 'q')
        {
            scanf("%c",&scanChar);
        }
    }
	
    /** stop all therad */
    NETWORK_ManagerStop(pManager);
	
    printf("wait ... \n");
    
	/** kill all thread */
	sal_thread_join(&(thread_send1), NULL);
	sal_thread_join(&(thread_recv1), NULL);

	/** delete */
    sal_thread_destroy(&thread_send1);
    sal_thread_destroy(&thread_recv1);
    NETWORK_DeleteManager( &pManager );
    
    printf("end \n");
    
    fixTerminal (0);
    
    return 0;
}

