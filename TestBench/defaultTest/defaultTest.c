/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 * @file main.h
 * @brief libARNetwork TestBench automatic
 * @date 05/18/2012
 * @author maxime.maitre@parrot.com
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <stdio.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Thread.h>

#include <libARNetwork/ARNETWORK_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define TEST_PING_DELAY (0) // Use default value

#define PORT1 12345
#define PORT2 54321

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define RECV_TIMEOUT_MS 10

/** terminal setting */
struct termios initial_settings, new_settings;

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

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
    ARNETWORK_Manager_t *managerPtr = NULL;
    ARNETWORKAL_Manager_t *networkALManagerPtr = NULL;

    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORKAL_ERROR specificError = ARNETWORKAL_OK;
    eARNETWORKAL_ERROR connectError = ARNETWORKAL_OK;
    char IpAddress[16];

    ARSAL_Thread_t thread_send1;
    ARSAL_Thread_t thread_recv1;

    /** save terminal setting */
    tcgetattr(0,&initial_settings);
    /** call fixTerminal when the terminal kill the program */
    signal (SIGINT, fixTerminal);

    printf(" -- libARNetwork TestBench default -- \n");

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

    networkALManagerPtr = ARNETWORKAL_Manager_New(&specificError);

    if(specificError == ARNETWORKAL_OK)
    {
        do
        {
            printf("repeater IP address ? : ");
            scanf("%s",IpAddress);

            connectError = ARNETWORKAL_Manager_InitWifiNetwork(networkALManagerPtr, IpAddress, sendPort, recvPort, RECV_TIMEOUT_MS);
            printf("    - connect error: %d \n", connectError );
            printf("\n");
        } while( connectError != ARNETWORKAL_OK);

    }

    if(specificError == ARNETWORKAL_OK)
    {
        managerPtr = ARNETWORK_Manager_New(networkALManagerPtr, 0, NULL, 0, NULL, TEST_PING_DELAY, NULL, NULL, &error);
    }
    else
    {
        error = ARNETWORK_ERROR;
    }

    /** start threads */
    ARSAL_Thread_Create(&(thread_recv1), (ARSAL_Thread_Routine_t) ARNETWORK_Manager_ReceivingThreadRun, managerPtr);
    ARSAL_Thread_Create(&thread_send1, (ARSAL_Thread_Routine_t) ARNETWORK_Manager_SendingThreadRun, managerPtr);

    while ( ((scanChar = getchar()) != '\n') && scanChar != EOF )
    {

    }

    /** set the terminal on nonBloking mode */
    setupNonBlockingTerm ();

    if(error == ARNETWORK_OK )
    {
        printf("press q to quit: \n");

        while(scanChar != 'q')
        {
            scanf("%c",&scanChar);
        }
    }

    /** stop all therad */
    ARNETWORK_Manager_Stop(managerPtr);

    printf("wait ... \n");

    /** kill all thread */
    ARSAL_Thread_Join(&(thread_send1), NULL);
    ARSAL_Thread_Join(&(thread_recv1), NULL);

    /** delete */
    ARSAL_Thread_Destroy(&thread_send1);
    ARSAL_Thread_Destroy(&thread_recv1);
    ARNETWORK_Manager_Delete( &managerPtr );

    printf("end \n");

    fixTerminal (0);

    return 0;
}
