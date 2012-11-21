//
//  AppDelegate.m
//  repeater
//
//  Created by Nicolas Payot on 14/11/12.
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
    pNetwork2 = NULL;
	
	//network_paramNewInOutBuffer_t paramNetwork2[3];
	network_paramNewInOutBuffer_t paramInputNetwork2[1];
	network_paramNewInOutBuffer_t paramOutputNetwork2[2];
	
	
	//--- network 2 ---
	
	// input ID_INT_DATA char
	paramInputNetwork2[0].id = ID_INT_DATA;
	paramInputNetwork2[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork2[0].sendingWaitTime = 2;
	paramInputNetwork2[0].ackTimeoutMs = 10;//not used
	paramInputNetwork2[0].nbOfRetry = 5;//not used
	paramInputNetwork2[0].buffSize = 2;
	paramInputNetwork2[0].buffCellSize = sizeof(int);
	paramInputNetwork2[1].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramOutputNetwork2[0].id = ID_CHAR_DATA;
	paramOutputNetwork2[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork2[0].sendingWaitTime = 3;
	paramOutputNetwork2[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[0].nbOfRetry = 5;//not used
	paramOutputNetwork2[0].buffSize = 1;
	paramOutputNetwork2[0].buffCellSize = sizeof(char);
	paramOutputNetwork2[0].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
	paramOutputNetwork2[1].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramOutputNetwork2[1].sendingWaitTime = 2;//not used
	paramOutputNetwork2[1].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[1].nbOfRetry = 5;//not used
	paramOutputNetwork2[1].buffSize = 5;
	paramOutputNetwork2[1].buffCellSize = sizeof(int);
	paramOutputNetwork2[1].overwriting = 0;
	
	//-----------------------------
    
	/*
     pNetwork2 = newNetworkWithVarg( 256, 256, 2, 1,
     paramNetwork2[1], paramNetwork2[2],
     paramNetwork2[0]);
     */
	
	pNetwork2 = newNetwork( 256, 256, 1, paramInputNetwork2, 2, paramOutputNetwork2);
    
}

- (void)startThreadRepeater
{
    NSLog(@"startThreadRepeater");
    
    sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetwork2->pReceiver);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetwork2->pSender);
}

- (void)stopThreadRepeater
{
    //network_inOutBuffer_t* pInOutTemp = NULL;
    
    NSLog(@"stopThreadRepeater");
    
    [self.viewController.textViewInfo appendText:@" stopThreadRepeater \n" ];
    
	//stop all therad
    if(pNetwork2 != NULL)
    {
        stopSender(pNetwork2->pSender);
        stopReceiver(pNetwork2->pReceiver);
    }
    
    [self print];
	
	//kill all thread
	if(thread_send2 != NULL)
    {
        sal_thread_join(&(thread_send2), NULL);
        thread_send2 = NULL;
    }
    
    if(thread_recv2 != NULL)
    {
        sal_thread_join(&(thread_recv2), NULL);
        thread_recv2 = NULL;
    }
    
    
}

- (void)exit
{
    NSLog(@" wait ...");
    
    [self.viewController.textViewInfo appendText:@" wait ..." ];
    
    [self stopThreadRepeater];
    
    //delete
	deleteNetwork( &pNetwork2 );
    
    exit(0);
}


- (void) print
{
    network_inOutBuffer_t* pInOutTemp = NULL;
    
	NSLog(@"\n the last char transmited:\n");

    [self.viewController.textViewInfo appendText: @"\n the last char transmited:\n"];
    
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
    
    char chdata = 0;
    while( !ringBuffPopFront(pInOutTemp->pBuffer, &chdata) )
    {
        NSLog(@" - %c \n",chdata);
        
        
        [self.viewController.textViewInfo appendText: [@"- " stringByAppendingFormat:@"%c \n",chdata ]];
        
    }
	
    
    
	NSLog(@"\n the integers transmited:\n");
    
    [self.viewController.textViewInfo appendText:@"\n the integers transmited:\n"];
    
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
    
    int intdata = 0;
    while( !ringBuffPopFront(pInOutTemp->pBuffer, &intdata) )
    {
        NSLog(@" - %d \n",intdata);
        
        [self.viewController.textViewInfo appendText:[@"- " stringByAppendingFormat:@"%d \n",intdata ]];
    }
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
    
    bindError = receiverBind(pNetwork2->pReceiver, 5551, 10);
    connectError = senderConnection(pNetwork2->pSender,IpAddress, 5552);
    connected = true;
    
    
    NSLog(@"bindError = %d connectError= %d", bindError, connectError);
    
    NSLog(@"connected = %d", connected);
    
    
    [self.viewController.textViewInfo appendText: @" connected \n"];
    
    return connected;
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
    pNetwork2 = NULL;
    
    thread_send2 = NULL;
    thread_recv2 = NULL;

    
    
    [self netWorkConstructor];
    //[self.viewController setDelegate:self];
    [self.viewController setPAppDelegate:self];
    
    [self startThreadRepeater];
    
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
}

@end
