//
//  AppDelegate.m
//  test1
//
//  Created by Nicolas Payot on 09/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate





#import <libSAL/print.h>
#import <libSAL/thread.h>

#import <libNetwork/common.h>
#import <libNetwork/inOutBuffer.h>
#import <libNetwork/sender.h>
#import <libNetwork/receiver.h>
#import <libNetwork/network.h>
#import <libSAL/socket.h>

#import <unistd.h>

#define RING_BUFFER_SIZE 256
#define RING_BUFFER_CELL_SIZE 10

#define ID_SEND_RING_BUFF 1
#define ID_SEND_SING_BUFF 2

#define ID_RECV_RING_BUFF 1

typedef enum eID_BUFF
{
	ID_CHAR_DATA = 5,
	ID_INT_DATA_WITH_ACK,
	ID_INT_DATA
}eID_BUFF;

- (void)test1
{
    NSLog(@"toto");
    
    
	network_t* pNetwork1= NULL;
	network_t* pNetwork2= NULL;
	
	int i=0;
	char chData = 0;
	
	int intData = 0;
	
	network_inOutBuffer_t* pInOutTemp = NULL;
	
	//network_paramNewInOutBuffer_t paramNetwork1[3];
	network_paramNewInOutBuffer_t paramInputNetwork1[2];
	network_paramNewInOutBuffer_t paramOutputNetwork1[1];
	
	//network_paramNewInOutBuffer_t paramNetwork2[3];
	network_paramNewInOutBuffer_t paramInputNetwork2[1];
	network_paramNewInOutBuffer_t paramOutputNetwork2[2];
	
	//--- network 1 ---
	
	// input ID_CHAR_DATA int
	paramInputNetwork1[0].id = ID_CHAR_DATA;
	paramInputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork1[0].sendingWaitTime = 3;//not used
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
	paramInputNetwork1[1].nbOfRetry = -1/*20*/;
	paramInputNetwork1[1].buffSize = 5;
	paramInputNetwork1[1].buffCellSize = sizeof(int);
	paramInputNetwork1[1].overwriting = 0;
	
	// output ID_INT_DATA int
	paramOutputNetwork1[0].id = ID_INT_DATA;
	paramOutputNetwork1[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork1[0].sendingWaitTime = 3;//not used
	paramOutputNetwork1[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork1[0].nbOfRetry = 5;//not used
	paramOutputNetwork1[0].buffSize = 10;
	paramOutputNetwork1[0].buffCellSize = sizeof(int);
	paramOutputNetwork1[0].overwriting = 1;
	
	//-----------------------------
	
	
	//--- network 2 ---
	
	// input ID_INT_DATA char
	paramInputNetwork2[0].id = ID_INT_DATA;
	paramInputNetwork2[0].dataType = CMD_TYPE_DATA;
	paramInputNetwork2[0].sendingWaitTime = 2;
	paramInputNetwork2[0].ackTimeoutMs = 10;//not used
	paramInputNetwork2[0].nbOfRetry = 5;//not used
	paramInputNetwork2[0].buffSize = 2;
	paramInputNetwork2[0].buffCellSize = sizeof(int);
	paramInputNetwork2[0].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramOutputNetwork2[0].id = ID_CHAR_DATA;
	paramOutputNetwork2[0].dataType = CMD_TYPE_DATA;
	paramOutputNetwork2[0].sendingWaitTime = 3;
	paramOutputNetwork2[0].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[0].nbOfRetry = 10;//not used
	paramOutputNetwork2[0].buffSize = 1;
	paramOutputNetwork2[0].buffCellSize = sizeof(char);
	paramOutputNetwork2[0].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
	paramOutputNetwork2[1].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramOutputNetwork2[1].sendingWaitTime = 3;//not used
	paramOutputNetwork2[1].ackTimeoutMs = 10;//not used
	paramOutputNetwork2[1].nbOfRetry = 10;//not used
	paramOutputNetwork2[1].buffSize = 5;
	paramOutputNetwork2[1].buffCellSize = sizeof(int);
	paramOutputNetwork2[1].overwriting = 0;
	
	//-----------------------------
    
	/*
     pNetwork1 = newNetworkWithVarg( 256, 256, 1, 2,
                                    paramNetwork1[2],
                                    paramNetwork1[0], paramNetwork1[1]);
     */
	
	pNetwork1 = newNetwork( 256, 256, 2, paramInputNetwork1, 1, paramOutputNetwork1);
    
	sal_print( PRINT_WARNING," -pNetwork1->pSender connect error: %d \n",
              senderConnection(pNetwork1->pSender,"127.0.0.1", 5551) );
    
	sal_print( PRINT_WARNING," -pNetwork1->pReceiver Bind  error: %d \n",
              receiverBind(pNetwork1->pReceiver, 5552, 10) );
	
	/*
     pNetwork2 = newNetworkWithVarg( 256, 256, 2, 1,
                                    paramNetwork2[1], paramNetwork2[2],
                                    paramNetwork2[0]);
     */
	pNetwork2 = newNetwork( 256, 256, 1, paramInputNetwork2, 2, paramOutputNetwork2);
    
	sal_print( PRINT_WARNING,	" -pNetwork2->pReceiver Bind  error: %d \n",
              receiverBind(pNetwork2->pReceiver, 5551, 5) );
	sal_print( PRINT_WARNING,	" -pNetwork2->pSender connect error: %d \n",
              senderConnection(pNetwork2->pSender,"127.0.0.1", 5552) );
    
	sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
	
	sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
	
	sal_print(PRINT_WARNING,"main start \n");
	
	
	sal_thread_create(&(thread_recv2), (sal_thread_routine) runReceivingThread, pNetwork2->pReceiver);
	sal_thread_create(&(thread_recv1), (sal_thread_routine) runReceivingThread, pNetwork1->pReceiver);
	
	sal_thread_create(&thread_send1, (sal_thread_routine) runSendingThread, pNetwork1->pSender);
	sal_thread_create(&thread_send2, (sal_thread_routine) runSendingThread, pNetwork2->pSender);
    
    for(i=0 ; i<5 ; i++)
    {
		
		//chData = ntohl (0x50 + i);
		chData =  i;
		NSLog(@" send char: %d \n",chData);
		pInOutTemp = inOutBufferWithId(	pNetwork1->ppTabInput, pNetwork1->numOfInput, ID_CHAR_DATA);
		ringBuffPushBack(pInOutTemp->pBuffer, &chData);
        
		
		intData = ntohl (0x11223340 +i);
		NSLog(@" send int: %0x \n",0x11223340 +i);
		pInOutTemp = inOutBufferWithId(	pNetwork1->ppTabInput,
                                       pNetwork1->numOfInput, ID_INT_DATA_WITH_ACK);
		ringBuffPushBack(pInOutTemp->pBuffer, &intData);
		
		
        usleep(50000);
    }
	
	NSLog(@" -- stop-- \n");
	
	//stop all therad
	stopSender(pNetwork1->pSender);
	stopSender(pNetwork2->pSender);
	stopReceiver(pNetwork1->pReceiver);
	stopReceiver(pNetwork2->pReceiver);
	
	NSLog(@"\n the last char transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	NSLog(@"\n the integers transmited:\n");
	pInOutTemp = inOutBufferWithId(	pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK);
	ringBuffPrint(pInOutTemp->pBuffer);
	
	NSLog(@"\n");
	NSLog(@"wait ... \n");
	
	//kill all thread
	
	sal_thread_join(&(thread_send1), NULL);
	sal_thread_join(&(thread_send2), NULL);
	
	sal_thread_join(&(thread_recv1), NULL);
	sal_thread_join(&(thread_recv2), NULL);
    
	//delete
	deleteNetwork( &pNetwork1 );
	deleteNetwork( &pNetwork2 );
    
	NSLog(@"end \n");
    
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

@end
