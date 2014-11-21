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
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//


/*****************************************
 *
 *             include file :
 *
 ******************************************/

#import "ViewController.h"

#include <stdlib.h>

#include <libARSAL/ARSAL_Thread.h>

#include <libNetwork/deportedData.h>
#include <libNetwork/frame.h>
#include <libNetwork/manager.h>

#include <unistd.h>

#include "../../Includes/networkDef.h"

#import "AppDelegate.h"


/*****************************************
 *
 *             define :
 *
 ******************************************/

#define MILLISECOND 1000
#define RECV_TIMEOUT_MS 10
#define PORT1 12345
#define PORT2 54321

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define NB_OF_INPUT_L1 4
#define NB_OF_INPUT_L2 4

#define FIRST_DEPORTED_DATA 'A'
#define LIMIT_SIZE_DEPORT_DATA 27


/*****************************************
 *
 *             implementation :
 *
 ******************************************/

@implementation AppDelegate


@synthesize connected;
@synthesize netType;

- (void)netWorkConstructor
{
    pManager1 = NULL;
    eNETWORK_Error error = NETWORK_OK;
    
    network_paramNewIoBuffer_t paramNetworkL1[5];
    network_paramNewIoBuffer_t paramNetworkL2[4];
    
    
    /** --- network 1 --- */
    
    /** input ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[0]) );
    paramNetworkL1[0].id = ID_CHAR_DATA;
    paramNetworkL1[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL1[0].sendingWaitTimeMs = 3;
    paramNetworkL1[0].numberOfCell = 1;
    paramNetworkL1[0].cellSize = sizeof(char);
    paramNetworkL1[0].isOverwriting = 1;
    
    /** input ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[1]) );
    paramNetworkL1[1].id = ID_INT_DATA_WITH_ACK;
    paramNetworkL1[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL1[1].sendingWaitTimeMs = 2;
    paramNetworkL1[1].ackTimeoutMs = 10;
    paramNetworkL1[1].nbOfRetry = 5;
    paramNetworkL1[1].numberOfCell = 5;
    paramNetworkL1[1].cellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA_ACK  */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[2]) );
    paramNetworkL1[2].id = ID_DEPORT_DATA_ACK;
    paramNetworkL1[2].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL1[2].sendingWaitTimeMs = 2;
    paramNetworkL1[2].ackTimeoutMs = 10;
    paramNetworkL1[2].nbOfRetry = 5;
    paramNetworkL1[2].numberOfCell = 5;
    paramNetworkL1[2].deportedData = 1;
    
    /** input ID_DEPORT_DATA */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[3]) );
    paramNetworkL1[3].id = ID_DEPORT_DATA;
    paramNetworkL1[3].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL1[3].sendingWaitTimeMs = 2;
    paramNetworkL1[3].numberOfCell = 5;
    paramNetworkL1[3].deportedData = 1;
    
    /** input ID_KEEP_ALIVE char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[4]) );
    paramNetworkL1[4].id = ID_KEEP_ALIVE;
    paramNetworkL1[4].dataType = NETWORK_FRAME_TYPE_KEEP_ALIVE;
    paramNetworkL1[4].sendingWaitTimeMs = 100;
    paramNetworkL1[4].numberOfCell = 1;
    paramNetworkL1[4].cellSize = sizeof(int);
    paramNetworkL1[4].isOverwriting = 1;
    
    /**  ID_CHAR_DATA_2 int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[0]) );
    paramNetworkL2[0].id = ID_CHAR_DATA_2;
    paramNetworkL2[0].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL2[0].sendingWaitTimeMs = 3;
    paramNetworkL2[0].numberOfCell = 1;
    paramNetworkL2[0].cellSize = sizeof(char);
    paramNetworkL2[0].isOverwriting = 1;
    
    /**  ID_INT_DATA_WITH_ACK_2 char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[1]) );
    paramNetworkL2[1].id = ID_INT_DATA_WITH_ACK_2;
    paramNetworkL2[1].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL2[1].sendingWaitTimeMs = 2;
    paramNetworkL2[1].ackTimeoutMs = 10;
    paramNetworkL2[1].nbOfRetry = -1 /*5*/;
    paramNetworkL2[1].numberOfCell = 5;
    paramNetworkL2[1].cellSize = sizeof(int);
    paramNetworkL2[1].isOverwriting = 0;
    
    /** input ID_DEPORT_DATA_ACK_2  */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[2]) );
    paramNetworkL2[2].id = ID_DEPORT_DATA_ACK_2;
    paramNetworkL2[2].dataType = NETWORK_FRAME_TYPE_DATA_WITH_ACK;
    paramNetworkL2[2].sendingWaitTimeMs = 2;
    paramNetworkL2[2].ackTimeoutMs = 10;
    paramNetworkL2[2].nbOfRetry = -1 /*5*/;
    paramNetworkL2[2].numberOfCell = 5;
    paramNetworkL2[2].deportedData = 1;
    
    /** input ID_DEPORT_DATA_2 */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[3]) );
    paramNetworkL2[3].id = ID_DEPORT_DATA_2;
    paramNetworkL2[3].dataType = NETWORK_FRAME_TYPE_DATA;
    paramNetworkL2[3].sendingWaitTimeMs = 2;
    paramNetworkL2[3].numberOfCell = 5;
    paramNetworkL2[3].deportedData = 1;

    
    NSLog(@"\n ~~ This soft sends data and repeater ack ~~ \n \n");
    
    switch(netType)
    {
        case 1:
            pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, NB_OF_INPUT_L1, paramNetworkL1, NB_OF_INPUT_L2,paramNetworkL2, &error);
            
            id_ioBuff_char = ID_CHAR_DATA;
            id_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
            id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK;
            id_ioBuff_deportData = ID_DEPORT_DATA;
            
            id_print_ioBuff_char = ID_CHAR_DATA_2;
            id_print_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
            id_print_ioBuff_deportDataAck =ID_DEPORT_DATA_ACK_2;
            id_print_ioBuff_deportData=ID_DEPORT_DATA_2;
            
            
            sendPort = PORT1;
            recvPort = PORT2;
            
             NSLog(@"pNetwork1 type 1");
            
        break;
            
        case 2:
            pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, NB_OF_INPUT_L2, paramNetworkL2, NB_OF_INPUT_L1 ,paramNetworkL1, &error);
            
            id_ioBuff_char = ID_CHAR_DATA_2;
            id_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
            id_ioBuff_deportDataAck = ID_DEPORT_DATA_ACK_2;
            id_ioBuff_deportData = ID_DEPORT_DATA_2;
            
            id_print_ioBuff_char = ID_CHAR_DATA;
            id_print_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
            id_print_ioBuff_deportDataAck =ID_DEPORT_DATA_ACK;
            id_print_ioBuff_deportData=ID_DEPORT_DATA;
            
            sendPort = PORT2;
            recvPort = PORT1;
            
            NSLog(@"pNetwork1 type 2");
            
        break;
            
        default:

        break;
    }
    
    
    
}

- (void)startThreadManager
{
    NSLog(@"startThreadManager");
    ARSAL_Thread_Create(&(thread_recv1), (ARSAL_Thread_Routine_t) NETWORK_ManagerRunReceivingThread, pManager1);
    ARSAL_Thread_Create(&thread_send1, (ARSAL_Thread_Routine_t) NETWORK_ManagerRunSendingThread, pManager1);
    
    if (timer == nil)
    {
        timer = [NSTimer scheduledTimerWithTimeInterval:0.01f target:self selector:@selector(print:) userInfo:nil repeats:YES];
    }
}
  
- (void)stopThreadManager
{
    
    NSLog(@"stopThreadManager");
    
    //stop all therad
    if(pManager1 != NULL)
    {
        NETWORK_ManagerStop(pManager1);
    }
    
    //kill all thread
    if(thread_send1 != NULL)
    {
        ARSAL_Thread_Join(&(thread_send1), NULL);
    }
    
    if(thread_recv1 != NULL)
    {
        ARSAL_Thread_Join(&(thread_recv1), NULL);
    }
    
    //stop the timer hiding the navbar
    if(timer != nil)
    {
        NSLog(@"timer invalidate\n");
        [timer invalidate];
        timer = nil;
    }
    
    
}

- (void)exit
{
    NSLog(@" wait ...\n");
    
    [self.viewController.textViewInfo appendText:@" wait ...\n" ];
    
    [self stopThreadManager];
    
    NSLog(@" ThreadManager stoped \n");
    
    //delete
    ARSAL_Thread_Destroy(&thread_send1);
    ARSAL_Thread_Destroy(&thread_recv1);
    NETWORK_DeleteManager( &pManager1 );
    
    NSLog(@" end \n");
    
    exit(0);
}

- (bool) connection:(NSString  *)ip
{
    const char* IpAddress = [ip UTF8String ];
    
    connectError = NETWORK_ManagerSocketsInit( pManager1, IpAddress, sendPort,
                                              recvPort, RECV_TIMEOUT_MS );
    
    connected = true;
     
    
    NSLog(@"connectError= %d", connectError);
    
    NSLog(@"connected = %d", connected);
    
    
    [ self.viewController.textViewInfo appendText: @" connected \n"];
    
    return connected;
}

- (bool) sendChar:(NSString * )data
{
    ++chData;
    
    NSLog(@"sendChar");
    [self.viewController.textViewInfo appendText:[ @"" stringByAppendingFormat: @"sendChar: %d \n", chData ] ];
    
    return (bool)NETWORK_ManagerSendData(pManager1, id_ioBuff_char, &chData) ;
}

- (bool) sendIntAck:(NSString * )data
{
    ++intData;
    NSLog(@"sendIntAck");
    
    [self.viewController.textViewInfo appendText: [ @"" stringByAppendingFormat: @"sendIntAck: %d \n",intData ] ];
    
    return (bool) NETWORK_ManagerSendData(pManager1, id_ioBuff_intAck, &intData);
}

- (bool) sendStrAck:(NSString * )data
{
    if( dataDeportSize_ack < LIMIT_SIZE_DEPORT_DATA )
    {
        ++dataDeportSize_ack;
    }
    else
    {
        dataDeportSize_ack = 2;
    }
    
    pDataDeported_ack = malloc(dataDeportSize_ack);
    
    /** write data */
    for( int ii = 0; ii < dataDeportSize_ack - 1 ; ++ii)
    {
        pDataDeported_ack[ii] = (FIRST_DEPORTED_DATA + ii) ;
    }
    /** end the string */
    pDataDeported_ack[dataDeportSize_ack - 1] = '\0' ;
    NSLog(@"send str :%s ",pDataDeported_ack);
    
    [self.viewController.textViewInfo appendText: [ @"" stringByAppendingFormat: @"send str : %s \n",pDataDeported_ack ] ];
    
    return (bool) NETWORK_ManagerSendDeportedData( pManager1,
                                                  id_ioBuff_deportDataAck,
                                                  pDataDeported_ack, dataDeportSize_ack,
                                                  &(callBackDepotData) );
}

- (bool) sendStr:(NSString * )data
{
    if( dataDeportSize < LIMIT_SIZE_DEPORT_DATA )
    {
        ++dataDeportSize;
    }
    else
    {
        dataDeportSize = 2;
    }
    
    pDataDeported = malloc(dataDeportSize);
    
    /** write data */
    for( int ii = 0; ii < dataDeportSize - 1 ; ++ii)
    {
        pDataDeported[ii] = (FIRST_DEPORTED_DATA + ii) ;
    }
    /** end the string */
    pDataDeported[dataDeportSize - 1] = '\0' ;
    NSLog(@"send str :%s ",pDataDeported);
    
    [self.viewController.textViewInfo appendText: [ @"" stringByAppendingFormat: @"send str : %s \n",pDataDeported ] ];
    
    return (bool) NETWORK_ManagerSendDeportedData( pManager1,
                                                  id_ioBuff_deportData,
                                                  pDataDeported, dataDeportSize,
                                                  &(callBackDepotData) );
}

- (void)print:(NSTimer *)_timer
{
    char chDataRecv = 0;
    int intDataRecv = 0;
    char deportData[LIMIT_SIZE_DEPORT_DATA];
    int error = NETWORK_OK;
    
    error =  NETWORK_ManagerReadData(pManager1, id_print_ioBuff_char, &chDataRecv);
    if( error ==  NETWORK_OK )
    {
        NSLog(@"- char: %d \n",chDataRecv);
        
        
        [self.viewController.textViewInfoRecv appendText: [@"- char: " stringByAppendingFormat:@"%d \n",chDataRecv ]];
    }
    
    error = NETWORK_ManagerReadData(pManager1, id_print_ioBuff_intAck, &intDataRecv) ;
    if( error ==  NETWORK_OK )
    {
        NSLog(@"- int ack: %d \n",intDataRecv);
        
        [self.viewController.textViewInfoRecv  appendText: [@"- int ack: " stringByAppendingFormat:@"%d \n",intDataRecv ]];
    }
    
    error = NETWORK_ManagerReadDeportedData(pManager1,
                                            id_print_ioBuff_deportDataAck,
                                            &deportData, LIMIT_SIZE_DEPORT_DATA, NULL );
    
    if( error ==  NETWORK_OK )
    {
        NSLog(@"- str ack: %s \n", deportData);
        [self.viewController.textViewInfoRecv appendText: [@"- str ack: " stringByAppendingFormat:@"%s \n",deportData ]];
    }
    
    
    error = NETWORK_ManagerReadDeportedData(pManager1,
                                            id_print_ioBuff_deportData,
                                            &deportData, LIMIT_SIZE_DEPORT_DATA, NULL );
    
    if( error ==  NETWORK_OK )
    {
        NSLog(@"- str : %s \n", deportData);
        [self.viewController.textViewInfoRecv appendText: [@"- str : " stringByAppendingFormat:@"%s \n",deportData ]];
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        self.viewController = [[ViewController alloc] initWithNibName:@"ViewController_iPhone" bundle:nil];
    } else {
        self.viewController = [[ViewController alloc] initWithNibName:@"ViewController_iPad" bundle:nil];
    }
    
    connected = false;
    connectError = -1;
    pManager1 = NULL;
    
    chData = 0;
    intData = 0;
    
    thread_send1 = NULL;
    thread_recv1 = NULL;

    
    timer = nil;
    
    
    //[self.viewController setDelegate:self];
    [self.viewController setPAppDelegate:self];
    
    
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    

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
    [self stopThreadManager];
}

@end

int callBackDepotData(int OutBufferId, void* pData, int status)
{
    /** local declarations */
    int error = 0;
    
    printf(" -- callBackDepotData -- \n");
    
    free(pData);
    
    return error;
}
