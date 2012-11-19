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



#define MILLISECOND 1000


- (void)netWorkConstructor
{
    pNetwork2 = NULL;
	
	network_paramNewInOutBuffer_t paramNetwork2[3];
	
	
	//--- network 2 ---
	
	// input ID_INT_DATA char
	paramNetwork2[0].id = ID_INT_DATA;
	paramNetwork2[0].dataType = CMD_TYPE_DATA;
	paramNetwork2[0].sendingWaitTime = 2;
	paramNetwork2[0].ackTimeoutMs = 10;//not used
	paramNetwork2[0].nbOfRetry = 5;//not used
	paramNetwork2[0].buffSize = 2;
	paramNetwork2[0].buffCellSize = sizeof(int);
	paramNetwork2[1].overwriting = 1;
	
	// output ID_CHAR_DATA int
	paramNetwork2[1].id = ID_CHAR_DATA;
	paramNetwork2[1].dataType = CMD_TYPE_DATA;
	paramNetwork2[1].sendingWaitTime = 3;
	paramNetwork2[1].ackTimeoutMs = 10;//not used
	paramNetwork2[1].nbOfRetry = 5;//not used
	paramNetwork2[1].buffSize = 1;
	paramNetwork2[1].buffCellSize = sizeof(char);
	paramNetwork2[1].overwriting = 1;
	
	// output ID_INT_DATA_WITH_ACK int
	paramNetwork2[2].id = ID_INT_DATA_WITH_ACK;
	paramNetwork2[2].dataType = CMD_TYPE_DATA_WITH_ACK;//not used
	paramNetwork2[2].sendingWaitTime = 2;//not used
	paramNetwork2[2].ackTimeoutMs = 10;//not used
	paramNetwork2[2].nbOfRetry = 5;//not used
	paramNetwork2[2].buffSize = 5;
	paramNetwork2[2].buffCellSize = sizeof(int);
	paramNetwork2[2].overwriting = 0;
	
	//-----------------------------
    
	pNetwork2 = newNetwork( 256, 256, 2, 1,
                           paramNetwork2[1], paramNetwork2[2],
                           paramNetwork2[0]);
    
    
}

- (void)startThreadRepeater
{
    NSLog(@"startThreadRepeater");
    
    pOutBuffChar = inOutBufferWithId( pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA) ;
    pOutBuffIntAck =  inOutBufferWithId( pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK) ;
    
    /*
    printThread1 = [ [printThread alloc] initWith:
        inOutBufferWithId( pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_CHAR_DATA) :
        inOutBufferWithId( pNetwork2->ppTabOutput, pNetwork2->numOfOutput, ID_INT_DATA_WITH_ACK) ];
    
    [printThread1 setPprintOut: self.viewController.textViewInfo ];
    
    NSLog(@"pre printThread1 %@",printThread1);
    NSLog(@"pre alive %d",printThread1.alive);
    NSLog(@"pre outchar %p",printThread1.pOutBuffChar);
    NSLog(@"pre outint %p",printThread1.pOutBuffIntAck);
    NSLog(@"pre pprintOut %@",printThread1.pprintOut);
    
    sal_thread_create(&thread_printBuff, (sal_thread_routine) printBuff, (__bridge void*) printThread1 );
    */
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
        //[printThread1 stop];
        
        //stop the timer hiding the navbar
        if(timer != nil)
        {
            NSLog(@"timer invalidate");
            [timer invalidate];
            timer = nil;
        }
        
    }
    
    //[self print];
	
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
    /*
    
    if(thread_printBuff != NULL)
    {
        sal_thread_join(&(thread_printBuff), NULL);
        thread_printBuff = NULL;
    }
     */
    
}

- (void)exit
{
    NSLog(@" wait ...");
    
    [self.viewController.textViewInfo appendText:@" wait ..." ];
    
    [self stopThreadRepeater];
    
    NSLog(@"deleteNetwork");
    
    //delete
	deleteNetwork( &pNetwork2 );
    
    NSLog(@"end");
    
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
    
    bindError = receiverBind(pNetwork2->pReceiver, 5551, 10);
    connectError = senderConnection(pNetwork2->pSender,IpAddress, 5552);
    connected = true;
    
    
    NSLog(@"bindError = %d connectError= %d", bindError, connectError);
    
    NSLog(@"connected = %d", connected);
    
    
    [self.viewController.textViewInfo appendText: @" connected \n"];
    
    return connected;
}


//- (void) print
- (void)print:(NSTimer *)_timer
{
    char chData = 0;
    int intData = 0;
    
    //while(alive)
	//{
		//usleep(MILLISECOND);
        
		while( !ringBuffPopFront(pOutBuffChar->pBuffer, &chData) )
		{
			NSLog(@"- char :%d \n",chData);
            
            
            [self.viewController.textViewInfo appendText: [@"- char" stringByAppendingFormat:@"%d \n",chData ]];
		}
		
		while( !ringBuffPopFront(pOutBuffIntAck->pBuffer, &intData) )
		{
			NSLog(@"- int ack :%d \n",intData);
            
            [self.viewController.textViewInfo  appendText: [@"- int ack :" stringByAppendingFormat:@"%d \n",intData ]];
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
    pNetwork2 = NULL;
    
    thread_send2 = NULL;
    thread_recv2 = NULL;
    
    timer = nil;
    
    
    [self netWorkConstructor];
    //[self.viewController setDelegate:self];
    [self.viewController setPAppDelegate:self];
    
    
    [self startThreadRepeater];
    
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    
    //[self print];
    if (timer == nil)
    {
        timer = [NSTimer scheduledTimerWithTimeInterval:0.01f target:self selector:@selector(print:) userInfo:nil repeats:YES];
    }
    
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






/*


@implementation printThread

@synthesize pOutBuffChar;
@synthesize pOutBuffIntAck;
@synthesize pprintOut;
@synthesize alive;


-(id) initWith:(network_inOutBuffer_t*)apOutBuffChar:(network_inOutBuffer_t*) apOutBuffIntAck
{
    self = [super init];
    self.pOutBuffChar = apOutBuffChar;
    self.pOutBuffIntAck = apOutBuffIntAck;
    alive = 1;
    return self;
}

-(void)stop
{
    alive = 0;
    
    [self setPprintOut:nil];
}

@end

void* printBuff(void* data)
{
	char chData = 0;
	int intData = 0;
	
	printThread *printThread1 = (__bridge printThread*) data;
    
    NSLog(@"wazaaaaaa");
    
    
    NSLog(@"pos printThread1 %@",printThread1);
    NSLog(@"pos alive %d",printThread1.alive);
    NSLog(@"pos outchar %p",printThread1.pOutBuffChar);
    NSLog(@"pos outint %p",printThread1.pOutBuffIntAck);
    NSLog(@"pos pprintOut %@",printThread1.pprintOut);
    
	while(printThread1.alive)
	{
		usleep(MILLISECOND);
        
		while( !ringBuffPopFront(printThread1.pOutBuffChar->pBuffer, &chData) )
		{
			NSLog(@"- char :%d \n",chData);
            
            
            [printThread1.pprintOut appendText: [@"- char" stringByAppendingFormat:@"%d \n",chData ]];
		}
		
		while( !ringBuffPopFront(printThread1.pOutBuffIntAck->pBuffer, &intData) )
		{
			NSLog(@"- int ack :%d \n",intData);
            
            [printThread1.pprintOut  appendText: [@"- int ack :" stringByAppendingFormat:@"%d \n",intData ]];
		}
	}
	
	return NULL;
}
 
 */
