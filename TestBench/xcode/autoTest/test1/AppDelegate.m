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
//
//  AppDelegate.m
//  test1
//
//  Created by Nicolas Payot on 09/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate




/*****************************************
 *
 *             include file :
 *
 ******************************************/

#include <stdlib.h>
#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Thread.h>

#include <string.h>

#include <libNetwork/paramNewIoBuffer.h>
#include <libNetwork/frame.h>
#include <libNetwork/manager.h>

#include <unistd.h>

/*****************************************
 *
 *             define :
 *
 ******************************************/

#define NUMBER_DATA_SENT 5
#define SENDING_SLEEP_TIME_US 50000
#define RECEIVER_TIMEOUT_SEC 5
#define FIRST_CHAR_SENT 'A'
#define FIRST_INT_ACK_SENT 100
#define BASE_DEPORTED_DATA 0x5555555555555555LL
#define BASE_DEPORTED_DATA_ACK 0xffffffffffffffffLL

#define RECV_TIMEOUT_MS 10
#define PORT1 5551
#define PORT2 5552
#define ADRR_IP "127.0.0.1"

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define NB_OF_INPUT_NET1 4
#define NB_OF_OUTPUT_NET1 1
#define NB_OF_INPUT_NET2 1
#define NB_OF_OUTPUT_NET2 4

#define SENDDATA 1
#define SENDDATADEPORT 1

/** define of the ioBuffer identifiers */
typedef enum eID_BUFF
{
    ID_CHAR_DATA = 5,
    ID_INT_DATA_WITH_ACK,
    ID_INT_DATA,
    ID_DEPORT_DATA,
    ID_DEPORT_DATA_ACK
}eID_BUFF;

int callBackDepotData(int OutBufferId, void* pData, int status);

/*****************************************
 *
 *             implementation :
 *
 ******************************************/

- (void)test1
{
    
    /** local declarations */
    network_manager_t* pManager1= NULL;
    network_manager_t* pManager2= NULL;
    ARSAL_Thread_t thread_send1 = NULL;
    ARSAL_Thread_t thread_recv1 = NULL;
    ARSAL_Thread_t thread_send2 = NULL;
    ARSAL_Thread_t thread_recv2 = NULL;
    
    int ii = 0;
    int error = 0;
    char chData = 0;
    int intData = 0;
    
    void* pDataDeported = NULL;
    uint64_t orgDataDeported = BASE_DEPORTED_DATA;
    uint64_t dataDeportedRead = 0;
    int dataDeportSize = 0 ;
    
    void* pDataDeported_ack = NULL;
    uint64_t orgDataDeported_ack = BASE_DEPORTED_DATA_ACK;
    uint64_t dataDeportedRead_ack = 0;
    int dataDeportSize_ack = 0 ;
    
    network_paramNewIoBuffer_t paramInputNetwork1[NB_OF_INPUT_NET1];
    network_paramNewIoBuffer_t paramOutputNetwork1[NB_OF_OUTPUT_NET1];
    
    network_paramNewIoBuffer_t paramInputNetwork2[NB_OF_INPUT_NET2];
    network_paramNewIoBuffer_t paramOutputNetwork2[NB_OF_OUTPUT_NET2];
    
    /** initialization of the buffer parameters */
    /** --- network 1 --- */
    
    /** input ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[0]) );
    paramInputNetwork1[0].id = ID_CHAR_DATA;
    paramInputNetwork1[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramInputNetwork1[0].numberOfCell = 1;
    paramInputNetwork1[0].cellSize = sizeof(char);
    paramInputNetwork1[0].isOverwriting = 1;
    
    /** input ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[1]) );
    paramInputNetwork1[1].id = ID_INT_DATA_WITH_ACK;
    paramInputNetwork1[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramInputNetwork1[1].sendingWaitTimeMs = 2;
    paramInputNetwork1[1].ackTimeoutMs = 5;
    paramInputNetwork1[1].nbOfRetry = -1/*20*/;
    paramInputNetwork1[1].numberOfCell = 5;
    paramInputNetwork1[1].cellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[2]) );
    paramInputNetwork1[2].id = ID_DEPORT_DATA;
    paramInputNetwork1[2].dataType = NETWORK_FRAME_TYPE_DATA;
    paramInputNetwork1[2].sendingWaitTimeMs = 2;
    paramInputNetwork1[2].numberOfCell = 5;
    paramInputNetwork1[2].deportedData = 1;
    
    /** input ID_DEPORT_DATA_ACK */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[3]) );
    paramInputNetwork1[3].id = ID_DEPORT_DATA_ACK;
    paramInputNetwork1[3].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramInputNetwork1[3].sendingWaitTimeMs = 2;
    paramInputNetwork1[3].ackTimeoutMs = 5;
    paramInputNetwork1[3].nbOfRetry = -1/*20*/;
    paramInputNetwork1[3].numberOfCell = 5;
    paramInputNetwork1[3].deportedData = 1;
    
    /** output ID_INT_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork1[0]) );
    paramOutputNetwork1[0].id = ID_INT_DATA;
    paramOutputNetwork1[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramOutputNetwork1[0].numberOfCell = 5;
    paramOutputNetwork1[0].cellSize = sizeof(int);
    paramOutputNetwork1[0].isOverwriting = 1;
    
    /** ----------------------------- */
    
    /**--- network 2 --- */
    
    /** input ID_INT_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork2[0]) );
    paramInputNetwork2[0].id = ID_INT_DATA;
    paramInputNetwork2[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramInputNetwork2[0].sendingWaitTimeMs = 2;
    paramInputNetwork2[0].numberOfCell = 2;
    paramInputNetwork2[0].cellSize = sizeof(int);
    paramInputNetwork2[0].isOverwriting = 1;
    
    /**  output ID_CHAR_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[0]) );
    paramOutputNetwork2[0].id = ID_CHAR_DATA;
    paramOutputNetwork2[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramOutputNetwork2[0].sendingWaitTimeMs = 3;
    paramOutputNetwork2[0].numberOfCell = 1;
    paramOutputNetwork2[0].cellSize = sizeof(char);
    paramOutputNetwork2[0].isOverwriting = 1;
    
    /** output ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[1]) );
    paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
    paramOutputNetwork2[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramOutputNetwork2[1].numberOfCell = 5;
    paramOutputNetwork2[1].cellSize = sizeof(int);
    
    /** output ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[2]) );
    paramOutputNetwork2[2].id = ID_DEPORT_DATA;
    paramOutputNetwork2[2].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramOutputNetwork2[2].numberOfCell = 5;
    paramOutputNetwork2[2].deportedData = 1;
    
    /** output ID_DEPORT_DATA_ACK */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[3]) );
    paramOutputNetwork2[3].id = ID_DEPORT_DATA_ACK;
    paramOutputNetwork2[3].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramOutputNetwork2[3].numberOfCell = 5;
    paramOutputNetwork2[3].deportedData = 1;
    
    /** ----------------------------- */

    
    NSLog(@" -- libNetWork Test Bench auto -- \n");
    
    /** create the Manager1 */
    
    pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE,
                                    NB_OF_INPUT_NET1, paramInputNetwork1,
                                    NB_OF_OUTPUT_NET1, paramOutputNetwork1, NULL );
    
    error = NETWORK_ManagerSocketsInit(pManager1, ADRR_IP, PORT1, PORT2, RECEIVER_TIMEOUT_SEC);
   NSLog(@"pManager1 error initScoket = %d", error);
    
    /** create the Manager2 */
    pManager2 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE,
                                   NB_OF_INPUT_NET2, paramInputNetwork2,
                                   NB_OF_OUTPUT_NET2, paramOutputNetwork2, NULL);
    
    error = NETWORK_ManagerSocketsInit(pManager2, ADRR_IP, PORT2, PORT1, RECEIVER_TIMEOUT_SEC);
    NSLog(@"pManager2 error initScoket = %d", error);
    
    NSLog(@"main start \n");
    
    
    /** create the threads */
    ARSAL_Thread_Create(&(thread_recv2), (ARSAL_Thread_Routine_t) NETWORK_ManagerRunReceivingThread, pManager2);
    ARSAL_Thread_Create(&(thread_recv1), (ARSAL_Thread_Routine_t) NETWORK_ManagerRunReceivingThread, pManager1);
    
    ARSAL_Thread_Create(&thread_send1, (ARSAL_Thread_Routine_t) NETWORK_ManagerRunSendingThread, pManager1);
    ARSAL_Thread_Create(&thread_send2, (ARSAL_Thread_Routine_t) NETWORK_ManagerRunSendingThread, pManager2);
    
    
    /** loop sending data */
    for(ii = 0; ii < NUMBER_DATA_SENT; ii++)
    {
        chData = FIRST_CHAR_SENT + ii;
        NSLog(@" send char: %c \n",chData);
        error = NETWORK_ManagerSendData(pManager1, ID_CHAR_DATA, &chData);
        
        if( error )
        {
            printf(" error send char \n");
        }
        
        intData = FIRST_INT_ACK_SENT + ii;
        NSLog(@" send int: %d \n",intData);
        error = NETWORK_ManagerSendData(pManager1, ID_INT_DATA_WITH_ACK, &intData);
        
        if( error )
        {
            NSLog(@" error send int ack \n");
        }
        
        /** create DataDeported */
        dataDeportSize = ii+1;
        
        pDataDeported = malloc(dataDeportSize);
        memcpy(pDataDeported, &orgDataDeported, dataDeportSize);
        
        error = NETWORK_ManagerSendDeportedData( pManager1, ID_DEPORT_DATA,
                                                 pDataDeported, dataDeportSize,
                                                 &(callbackDepotData) );
        
        NSLog(@" send %d byte deportedData\n",dataDeportSize);
        if( error  )
        {
            NSLog(@" error send deported data ack :%d \n", error);
        }
        
        /** create DataDeported_ack */
        dataDeportSize_ack = ii+1;
        
        pDataDeported_ack = malloc(dataDeportSize_ack);
        memcpy(pDataDeported_ack, &orgDataDeported_ack, dataDeportSize_ack);
        
        error = NETWORK_ManagerSendDeportedData( pManager1, ID_DEPORT_DATA_ACK,
                                                pDataDeported_ack, dataDeportSize_ack,
                                                &(callbackDepotData) );
        
        NSLog(@" send %d byte deportedData\n",dataDeportSize_ack);
        if( error  )
        {
            NSLog(@" error send deported data ack :%d \n", error);
        }

        
        usleep(SENDING_SLEEP_TIME_US);
    }
    
    NSLog(@" -- stop-- \n");
    
    /** stop all therad */
    NETWORK_ManagerStop(pManager1);
    NETWORK_ManagerStop(pManager2);
    
    NSLog(@"wait ... \n");
    
    //kill all thread
    if(thread_send1 != NULL)
    {
        ARSAL_Thread_Join(&(thread_send1), NULL);
    }
    if(thread_send2 != NULL)
    {
        ARSAL_Thread_Join(&(thread_send2), NULL);
    }
    
    if(thread_recv1 != NULL)
    {
        ARSAL_Thread_Join(&(thread_recv1), NULL);
    }
    
    if(thread_recv2 != NULL)
    {
        ARSAL_Thread_Join(&(thread_recv2), NULL);
    }
    
    
    /** checking */
    ii = 0;
    NSLog(@"\n the last char transmitted:\n");
    while( ! NETWORK_ManagerReadData(pManager2, ID_CHAR_DATA, &chData) )
    {
        ++ii;
        printf("- %c \n", chData);
        /** check values */
        error = error || ( chData != ( FIRST_CHAR_SENT + (NUMBER_DATA_SENT - 1) ) );
    }
    /** check nb data */
    error = error || ( ii != 1) ;
    
    NSLog(@"\n the integers transmitted:\n");
    ii = 0;
    while( ! NETWORK_ManagerReadData(pManager2, ID_INT_DATA_WITH_ACK, &intData) )
    {
        NSLog(@"- %d \n", intData);
        /** check values */
        error = error || ( intData != FIRST_INT_ACK_SENT + ii);
        ++ii;
    }
    /** check nb data */
    error = error || ( ii != 5) ;
    
    /** chech deported data */
    printf("\n the deported data transmitted:\n");
    ii = 0;
    dataDeportedRead = 0;
    
    
    while( ! NETWORK_ManagerReadDeportedData(pManager2, ID_DEPORT_DATA, &dataDeportedRead, ii+1, NULL) )
    {
        dataDeportSize = ii+1;
        
        printf("- %08x %08x \n",  (uint32_t) (dataDeportedRead >> 32), (uint32_t) dataDeportedRead );
        
        /** create DataDeported */
        pDataDeported = malloc(dataDeportSize);
        memcpy(pDataDeported, &orgDataDeported, dataDeportSize);
        
        /** check values */
        error = error || memcmp(&dataDeportedRead, pDataDeported ,dataDeportSize);
        ++ii;
        free(pDataDeported);
        dataDeportedRead = 0;
    }
    /** check nb data */
    error = error || ( ii != 5) ;
    
    printf("\n the deported data ACK transmitted:\n");
    ii = 0;
    dataDeportedRead_ack = 0;
    
    while( ! NETWORK_ManagerReadDeportedData(pManager2, ID_DEPORT_DATA_ACK, &dataDeportedRead_ack, ii+1, NULL) )
    {
        dataDeportSize_ack = ii+1;
        
        printf("- %08x %08x \n",  (uint32_t) (dataDeportedRead_ack >> 32), (uint32_t) dataDeportedRead_ack );
        
        /** create DataDeported */
        pDataDeported_ack = malloc(dataDeportSize_ack);
        memcpy(pDataDeported_ack, &orgDataDeported_ack, dataDeportSize_ack);
        
        /** check values */
        error = error || memcmp(&dataDeportedRead_ack, pDataDeported_ack ,dataDeportSize_ack);
        ++ii;
        free(pDataDeported_ack);
        dataDeportedRead_ack = 0;
    }
    /** check nb data */
    error = error || ( ii != 5) ;
    
    
    NSLog(@"\n");
    
    if(error)
    {
        NSLog(@"Bad result of the test bench \n");
    }
    else
    {
        NSLog(@"Good result of the test bench \n");
    }
    
    
    
    NSLog(@"end \n");
    
    /** delete */
    ARSAL_Thread_Destroy(&thread_send1);
    ARSAL_Thread_Destroy(&thread_send2);
    ARSAL_Thread_Destroy(&thread_recv1);
    ARSAL_Thread_Destroy(&thread_recv2);
    NETWORK_DeleteManager( &pManager1 );
    NETWORK_DeleteManager( &pManager2 );
    
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];
    
    [self test1];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

int callbackDepotData(int OutBufferId, void* pData, int status)
{
    /** local declarations */
    int retry = 0;
    
    printf(" -- callbackDepotData status: %d ",status);
    
    switch(status)
    {
        case NETWORK_CALLBACK_STATUS_SENT :
        case NETWORK_CALLBACK_STATUS_SENT_WITH_ACK :
        case NETWORK_CALLBACK_STATUS_FREE :
            retry = 0;
            free(pData);
            printf(" free --\n");
            break;
            
        case NETWORK_CALLBACK_STATUS_TIMEOUT :
            retry = 1;
            printf(" retry --\n");
            break;
            
        default:
            printf(" default --\n");
            break;
    }
    
    return retry;
}


@end
