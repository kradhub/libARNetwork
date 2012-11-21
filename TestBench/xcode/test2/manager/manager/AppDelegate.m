//
//  AppDelegate.m
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//



#import "ViewController.h"

#include <libSAL/print.h>
#include <libSAL/thread.h>

#include <libNetwork/common.h>
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>
#include <libNetwork/network.h>

#include <unistd.h>

#include "../../Includes/networkDef.h"

#import "AppDelegate.h"

@implementation AppDelegate


@synthesize connected;

- (void)netWorkConstructor
{
    pNetwork1 = NULL;
    
    
	//network_paramNewInOutBuffer_t paramNetwork1[4];
	network_paramNewInOutBuffer_t paramInputNetwork1[3];
	network_paramNewInOutBuffer_t paramOutputNetwork1[1];
	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
	paramInputNetwork1[0].id = ID_CHAR_DATA;
	paramInputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork1[0].sendingWaitTime = 3;
	paramInputNetwork1[0].ackTimeoutMs = 10;//not used
	paramInputNetwork1[0].nbOfRetry = 5;//not used
	paramInputNetwork1[0].buffSize = 1;
	paramInputNetwork1[0].buffCellSize = sizeof(char);
	paramInputNetwork1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
	paramInputNetwork1[1].id = ID_INT_DATA_WITH_ACK;
	paramInputNetwork1[1].dataType = CMD_TYPE_DATA_WITH_ACK;
	paramInputNetwork1[1].sendingWaitTime = 2;
	paramInputNetwork1[1].ackTimeoutMs = 10;
	paramInputNetwork1[1].nbOfRetry = -1 /*5*/;
	paramInputNetwork1[1].buffSize = 5;
	paramInputNetwork1[1].buffCellSize = sizeof(int);
	paramInputNetwork1[1].overwriting = 0;
	
	// input ID_KEEP_ALIVE char
	paramInputNetwork1[2].id = ID_KEEP_ALIVE;
	paramInputNetwork1[2].dataType = CMD_TYPE_KEEP_ALIVE;
	paramInputNetwork1[2].sendingWaitTime = 100;
	paramInputNetwork1[2].ackTimeoutMs = 10;//not used
	paramInputNetwork1[2].nbOfRetry = 5;//not used
	paramInputNetwork1[2].buffSize = 1;
	paramInputNetwork1[2].buffCellSize = sizeof(int);
	paramInputNetwork1[2].overwriting = 1;
	
	// output ID_INT_DATA int
	paramOutputNetwork1[0].id = ID_INT_DATA;
	paramOutputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork1[0].sendingWaitTime = 3;//not used
	paramOutputNetwork1[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork1[0].nbOfRetry = 5;//not used
	paramOutputNetwork1[0].buffSize = 10;
	paramOutputNetwork1[0].buffCellSize = sizeof(int);
	paramOutputNetwork1[0].overwriting = 1;
    
    
	//pNetwork1 = newNetworkWithVarg( 256, 256, 1, 2/*3*/,
	//						paramNetwork1[3],
	//						paramNetwork1[0], paramNetwork1[1]/*, paramNetwork1[2]*/);
	
	pNetwork1 = newNetwork( 256, 256, 2/*3*/,paramInputNetwork1, 1,paramOutputNetwork1);
    
}

- (void)startThreadManager
{
    NSLog(@"startThreadManager");
    
	sal_thread_create(&(thread_recv1), (sal_thread_routine) runReceivingThread, pNetwork1->pReceiver);
	sal_thread_create(&thread_send1, (sal_thread_routine) runSendingThread, pNetwork1->pSender);
}
  
- (void)stopThreadManager
{
    
    NSLog(@"stopThreadManager");
    
	//stop all therad
    if(pNetwork1 != NULL)
    {
        stopSender(pNetwork1->pSender);
        stopReceiver(pNetwork1->pReceiver);
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
    
	
    
}

- (void)exit
{
    NSLog(@" wait ...");
    
    [self.viewController.textViewInfo appendText:@" wait ..." ];
    
    [self stopThreadManager];
    
    //delete
	deleteNetwork( &pNetwork1 );
    
    exit(0);
}

- (bool) connection:(NSString  *)ip
{
    const char* IpAddress = [ip UTF8String /*cStringUsingEncoding:ASCIIEncoding*/];
    
    /*
     
    if( !connected)
    {
        
        if(bindError != 0)
        {
            bindError = receiverBind(pNetwork1->pReceiver, 5552, 10);
        }
    
        if(connectError != 0)
        {
            connectError = senderConnection(pNetwork1->pSender,IpAddress, 5551);
        }
    
        if( !connectError && !bindError )
        {
            connected = true;
            NSLog(@"connected");
        }
        
        
    }
     
     */
     
     bindError = receiverBind(pNetwork1->pReceiver, 5552, 10);
     connectError = senderConnection(pNetwork1->pSender,IpAddress, 5551);
     connected = true;
     
    
     NSLog(@"bindError = %d connectError= %d", bindError, connectError);
    
     NSLog(@"connected = %d", connected);
    
    
    [ self.viewController.textViewInfo appendText: @" connected \n"];
    
    return connected;
}

- (bool) sendChar:(NSString * )data
{
    //char chData = [data intValue];
    
    NSLog(@"sendChar");
    [self.viewController.textViewInfo appendText:[ @"" stringByAppendingFormat: @"sendChar : %c \n", *[data UTF8String] ] ];
    
    network_inOutBuffer_t* pInOutTemp = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_CHAR_DATA);
    //return (bool)ringBuffPushBack(pInOutTemp->pBuffer, &chData);
    return (bool)ringBuffPushBack(pInOutTemp->pBuffer, [data UTF8String]);
}

- (bool) sendIntAck:(NSString * )data
{
    int intData = [data intValue];
    NSLog(@"sendIntAck");
    
    [self.viewController.textViewInfo appendText: [ @"" stringByAppendingFormat: @"sendIntAck : %d \n",intData ] ];
    
    network_inOutBuffer_t*  pInOutTemp = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_INT_DATA_WITH_ACK);
    return (bool) ringBuffPushBack(pInOutTemp->pBuffer, &intData);
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
    bindError = -1;
    pNetwork1 = NULL;
    
    thread_send1 = NULL;
    thread_recv1 = NULL;
    
    [self netWorkConstructor];
    //[self.viewController setDelegate:self];
    [self.viewController setPAppDelegate:self];
    
    [self startThreadManager];
    
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
