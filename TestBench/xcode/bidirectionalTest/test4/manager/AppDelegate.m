//
//  AppDelegate.m
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//


/*****************************************
 *
 * 			include file :
 *
 ******************************************/

#import "ViewController.h"

#include <stdlib.h>

#include <libSAL/thread.h>

#include <libNetwork/frame.h>
#include <libNetwork/manager.h>

#include <unistd.h>

#include "../../Includes/networkDef.h"

#import "AppDelegate.h"


/*****************************************
 *
 * 			define :
 *
 ******************************************/

#define MILLISECOND 1000
#define RECV_TIMEOUT_MS 10
#define PORT1 5551
#define PORT2 5552

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256


/*****************************************
 *
 * 			implementation :
 *
 ******************************************/

@implementation AppDelegate


@synthesize connected;
@synthesize netType;

- (void)netWorkConstructor
{
    pManager1 = NULL;
    
    
    network_paramNewIoBuffer_t paramNetworkL1[3];
	network_paramNewIoBuffer_t paramNetworkL2[2];
    
	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[0]) );
	paramNetworkL1[0].id = ID_CHAR_DATA;
	paramNetworkL1[0].dataType = network_frame_t_TYPE_DATA;
	paramNetworkL1[0].sendingWaitTime = 3;
	paramNetworkL1[0].buffSize = 1;
	paramNetworkL1[0].buffCellSize = sizeof(char);
	paramNetworkL1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[1]) );
	paramNetworkL1[1].id = ID_INT_DATA_WITH_ACK;
	paramNetworkL1[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL1[1].sendingWaitTime = 2;
	paramNetworkL1[1].ackTimeoutMs = 10;
	paramNetworkL1[1].nbOfRetry = -1 /*5*/;
	paramNetworkL1[1].buffSize = 5;
	paramNetworkL1[1].buffCellSize = sizeof(int);
	paramNetworkL1[1].overwriting = 0;
	
	// input ID_KEEP_ALIVE char
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL1[2]) );
	paramNetworkL1[2].id = ID_KEEP_ALIVE;
	paramNetworkL1[2].dataType = network_frame_t_TYPE_KEEP_ALIVE;
	paramNetworkL1[2].sendingWaitTime = 100;
	paramNetworkL1[2].buffSize = 1;
	paramNetworkL1[2].buffCellSize = sizeof(int);
	paramNetworkL1[2].overwriting = 1;
	
	//  ID_CHAR_DATA_2 int
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[0]) );
	paramNetworkL2[0].id = ID_CHAR_DATA_2;
	paramNetworkL2[0].dataType = network_frame_t_TYPE_DATA;
	paramNetworkL2[0].sendingWaitTime = 3;
	paramNetworkL2[0].buffSize = 1;
	paramNetworkL2[0].buffCellSize = sizeof(char);
	paramNetworkL2[0].overwriting = 1;
	
	//  ID_INT_DATA_WITH_ACK_2 char
    NETWORK_ParamNewIoBufferDefaultInit( &(paramNetworkL2[1]) );
	paramNetworkL2[1].id = ID_INT_DATA_WITH_ACK_2;
	paramNetworkL2[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramNetworkL2[1].sendingWaitTime = 2;
	paramNetworkL2[1].ackTimeoutMs = 10;
	paramNetworkL2[1].nbOfRetry = -1 /*5*/;
	paramNetworkL2[1].buffSize = 5;
	paramNetworkL2[1].buffCellSize = sizeof(int);
	paramNetworkL2[1].overwriting = 0;

    
	NSLog(@"\n ~~ This soft sends data and repeater ack ~~ \n \n");
    
    switch(netType)
    {
        case 1:
            pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, 2/*3*/, paramNetworkL1, 2 ,paramNetworkL2);
            
            id_ioBuff_char = ID_CHAR_DATA;
            id_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
            
            id_print_ioBuff_char = ID_CHAR_DATA_2;
            id_print_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
            
            sendPort = PORT1;
            recvPort = PORT2;
            
             NSLog(@"pNetwork1 type 1");
            
        break;
			
        case 2:
            pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE, 2, paramNetworkL2, 2 ,paramNetworkL1);
            
            id_ioBuff_char = ID_CHAR_DATA_2;
            id_ioBuff_intAck = ID_INT_DATA_WITH_ACK_2;
            
            id_print_ioBuff_char = ID_CHAR_DATA;
            id_print_ioBuff_intAck = ID_INT_DATA_WITH_ACK;
            
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
    sal_thread_create(&(thread_recv1), (sal_thread_routine) NETWORK_ManagerRunReceivingThread, pManager1);
    sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_ManagerRunSendingThread, pManager1);
    
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
        sal_thread_join(&(thread_send1), NULL);
    }
    
    if(thread_recv1 != NULL)
    {
        sal_thread_join(&(thread_recv1), NULL);
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
	NETWORK_DeleteManager( &pManager1 );
    
    NSLog(@" end \n");
    
    exit(0);
}

- (bool) connection:(NSString  *)ip
{
    const char* IpAddress = [ip UTF8String ];
    
    connectError = NETWORK_ManagerScoketsInit( pManager1, IpAddress, sendPort,
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
    [self.viewController.textViewInfo appendText:[ @"" stringByAppendingFormat: @"sendChar : %d \n", chData ] ];
    
    return (bool)NETWORK_ManagerSendData(pManager1, id_ioBuff_char, &chData) ;
}

- (bool) sendIntAck:(NSString * )data
{
    ++intData;
    NSLog(@"sendIntAck");
    
    [self.viewController.textViewInfo appendText: [ @"" stringByAppendingFormat: @"sendIntAck : %d \n",intData ] ];
    
    return (bool) NETWORK_ManagerSendData(pManager1, id_ioBuff_intAck, &intData);
}

- (void)print:(NSTimer *)_timer
{
    char chDataRecv = 0;
    int intDataRecv = 0;
    
    
    while( ! NETWORK_ManagerReadData(pManager1, id_print_ioBuff_char, &chDataRecv) )
    {
        NSLog(@"- char :%d \n",chDataRecv);
        
        
        [self.viewController.textViewInfoRecv appendText: [@"- char" stringByAppendingFormat:@"%d \n",chDataRecv ]];
    }
    
    while( ! NETWORK_ManagerReadData(pManager1, id_print_ioBuff_intAck, &intDataRecv) )
    {
        NSLog(@"- int ack :%d \n",intDataRecv);
        
        [self.viewController.textViewInfoRecv  appendText: [@"- int ack :" stringByAppendingFormat:@"%d \n",intDataRecv ]];
    }
	//}
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
