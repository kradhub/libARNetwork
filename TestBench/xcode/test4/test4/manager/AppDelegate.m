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
@synthesize netType;

- (void)netWorkConstructor
{
    pNetwork1 = NULL;
    
	network_paramNewInOutBuffer_t paramNetwork1[5];
    
    //--- network 1 ---
	
	// input ID_CHAR_DATA int
	paramNetwork1[0].id = ID_CHAR_DATA;
	paramNetwork1[0].dataType = CMD_TYPE_DATA;
	paramNetwork1[0].sendingWaitTime = 3;
	paramNetwork1[0].ackTimeoutMs = 10;//not used
	paramNetwork1[0].nbOfRetry = 5;//not used
	paramNetwork1[0].buffSize = 1;
	paramNetwork1[0].buffCellSize = sizeof(char);
	paramNetwork1[0].overwriting = 1;
	
	// input ID_INT_DATA_WITH_ACK char
	paramNetwork1[1].id = ID_INT_DATA_WITH_ACK;
	paramNetwork1[1].dataType = CMD_TYPE_DATA_WITH_ACK;
	paramNetwork1[1].sendingWaitTime = 2;
	paramNetwork1[1].ackTimeoutMs = 10;
	paramNetwork1[1].nbOfRetry = -1 /*5*/;
	paramNetwork1[1].buffSize = 5;
	paramNetwork1[1].buffCellSize = sizeof(int);
	paramNetwork1[1].overwriting = 0;
	
	// input ID_KEEP_ALIVE char
	paramNetwork1[2].id = ID_KEEP_ALIVE;
	paramNetwork1[2].dataType = CMD_TYPE_KEEP_ALIVE;
	paramNetwork1[2].sendingWaitTime = 100;
	paramNetwork1[2].ackTimeoutMs = 10;//not used
	paramNetwork1[2].nbOfRetry = 5;//not used
	paramNetwork1[2].buffSize = 1;
	paramNetwork1[2].buffCellSize = sizeof(int);
	paramNetwork1[2].overwriting = 1;
	
	//  ID_CHAR_DATA_2 int
	paramNetwork1[3].id = ID_CHAR_DATA_2;
	paramNetwork1[3].dataType = CMD_TYPE_DATA;
	paramNetwork1[3].sendingWaitTime = 3;
	paramNetwork1[3].ackTimeoutMs = 10;//not used
	paramNetwork1[3].nbOfRetry = 5;//not used
	paramNetwork1[3].buffSize = 1;
	paramNetwork1[3].buffCellSize = sizeof(char);
	paramNetwork1[3].overwriting = 1;
	
	//  ID_INT_DATA_WITH_ACK_2 char
	paramNetwork1[4].id = ID_INT_DATA_WITH_ACK_2;
	paramNetwork1[4].dataType = CMD_TYPE_DATA_WITH_ACK;
	paramNetwork1[4].sendingWaitTime = 2;
	paramNetwork1[4].ackTimeoutMs = 10;
	paramNetwork1[4].nbOfRetry = -1 /*5*/;
	paramNetwork1[4].buffSize = 5;
	paramNetwork1[4].buffCellSize = sizeof(int);
	paramNetwork1[4].overwriting = 0;
    
    switch(netType)
    {
        case 1:
            pNetwork1 = newNetwork( 256, 256, 2, 2/*3*/,
                                   paramNetwork1[3], paramNetwork1[4],
                                   paramNetwork1[0], paramNetwork1[1]/*, paramNetwork1[2]*/);
            
            pOutBuffChar = inOutBufferWithId(	pNetwork1->ppTabOutput, pNetwork1->numOfOutput, ID_CHAR_DATA_2);
            pOutBuffIntAck = inOutBufferWithId(	pNetwork1->ppTabOutput, pNetwork1->numOfOutput, ID_INT_DATA_WITH_ACK_2);
            
            pInputBuffChar  = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_CHAR_DATA);
            pInputBuffIntAck = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_INT_DATA_WITH_ACK);
            
             NSLog(@"pNetwork1 type 1");
            
        break;
			
        case 2:
            pNetwork1 = newNetwork( 256, 256, 2, 2,
                                   paramNetwork1[0], paramNetwork1[1],
                                   paramNetwork1[3], paramNetwork1[4]);
            pOutBuffChar = inOutBufferWithId(	pNetwork1->ppTabOutput, pNetwork1->numOfOutput, ID_CHAR_DATA);
            pOutBuffIntAck = inOutBufferWithId(	pNetwork1->ppTabOutput, pNetwork1->numOfOutput, ID_INT_DATA_WITH_ACK);
            
            pInputBuffChar  = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_CHAR_DATA_2);
            pInputBuffIntAck = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_INT_DATA_WITH_ACK_2);
            
            NSLog(@"pNetwork1 type 2");
            
        break;
			
        default:

        break;
    }
    
    
    
}

- (void)startThreadManager
{
    NSLog(@"startThreadManager");
    
	sal_thread_create(&(thread_recv1), (sal_thread_routine) runReceivingThread, pNetwork1->pReceiver);
	sal_thread_create(&thread_send1, (sal_thread_routine) runSendingThread, pNetwork1->pSender);
    
    if (timer == nil)
    {
        timer = [NSTimer scheduledTimerWithTimeInterval:0.01f target:self selector:@selector(print:) userInfo:nil repeats:YES];
    }
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
	deleteNetwork( &pNetwork1 );
    
    NSLog(@" end \n");
    
    //exit(0);
    
    
    [self applicationWillTerminate:[UIApplication sharedApplication]];
    //[[UIApplication sharedApplication] terminate];
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
    
    if(netType == 1)
    {
        bindError = receiverBind(pNetwork1->pReceiver, 5552, 10);
        connectError = senderConnection(pNetwork1->pSender,IpAddress, 5551);
        NSLog(@"bind 5552");
        NSLog(@"connected 5551");
    }
    else
    {
        bindError = receiverBind(pNetwork1->pReceiver, 5551, 10);
        connectError = senderConnection(pNetwork1->pSender,IpAddress, 5552);
        NSLog(@"bind 5551");
        NSLog(@"connected 5552");
    }
    
    connected = true;
     
    
    NSLog(@"bindError = %d connectError= %d", bindError, connectError);
    
    NSLog(@"connected = %d", connected);
    
    
    [ self.viewController.textViewInfo appendText: @" connected \n"];
    
    return connected;
}

- (bool) sendChar:(NSString * )data
{
    ++chData;
    
    NSLog(@"sendChar");
    [self.viewController.textViewInfo appendText:[ @"" stringByAppendingFormat: @"sendChar : %d \n", chData ] ];
    
    //return (bool)ringBuffPushBack(pInOutTemp->pBuffer, &chData);
    return (bool)ringBuffPushBack(pInputBuffChar->pBuffer, &chData);
}

- (bool) sendIntAck:(NSString * )data
{
    ++intData;
    NSLog(@"sendIntAck");
    
    [self.viewController.textViewInfo appendText: [ @"" stringByAppendingFormat: @"sendIntAck : %d \n",intData ] ];
    
    return (bool) ringBuffPushBack(pInputBuffIntAck->pBuffer, &intData);
}

- (void)print:(NSTimer *)_timer
{
    char chDataRecv = 0;
    int intDataRecv = 0;
    
    //while(alive)
	//{
    //usleep(MILLISECOND);
    
    while( !ringBuffPopFront(pOutBuffChar->pBuffer, &chDataRecv) )
    {
        NSLog(@"- char :%d \n",chDataRecv);
        
        
        [self.viewController.textViewInfoRecv appendText: [@"- char" stringByAppendingFormat:@"%d \n",chDataRecv ]];
    }
    
    while( !ringBuffPopFront(pOutBuffIntAck->pBuffer, &intDataRecv) )
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
    bindError = -1;
    pNetwork1 = NULL;
    
    chData = 0;
    intData = 0;
    
    thread_send1 = NULL;
    thread_recv1 = NULL;
    
    pOutBuffChar = NULL;
	pOutBuffIntAck = NULL;
    
    pInputBuffChar = NULL;
    pInputBuffIntAck = NULL;
    
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
