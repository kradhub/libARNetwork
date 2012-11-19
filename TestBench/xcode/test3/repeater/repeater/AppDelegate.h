//
//  AppDelegate.h
//  repeater
//
//  Created by Nicolas Payot on 14/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <libSAL/thread.h>

#include <libNetwork/common.h>
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>

#include <libNetwork/network.h>

#import "TextViewConsole.h"

/*

@interface printThread:NSObject
{
	network_inOutBuffer_t* pOutBuffChar;
	network_inOutBuffer_t* pOutBuffIntAck;
	int alive;
    TextViewConsole* pprintOut;
	
};
@property ( nonatomic , assign) network_inOutBuffer_t* pOutBuffChar;
@property ( nonatomic , assign) network_inOutBuffer_t* pOutBuffIntAck;
@property ( nonatomic , assign) int alive;
@property ( nonatomic , retain) TextViewConsole* pprintOut;

-(id) initWith:(network_inOutBuffer_t*)apOutBuffChar:(network_inOutBuffer_t*) apOutBuffIntAck;
-(void) stop;

@end

void* printBuff(void* data);
*/


@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
    network_t* pNetwork2;
    
    bool connected;
    int connectError;
    int bindError;
    
    //printThread* printThread1;
    //int alive;
    NSTimer *timer;
    
    network_inOutBuffer_t* pOutBuffChar;
	network_inOutBuffer_t* pOutBuffIntAck;
    
    sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
}

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;


@property(nonatomic,assign)   bool connected;

- (void)startThreadRepeater;
- (void)stopThreadRepeater;
- (void)exit;
//- (void) print;
- (void)print:(NSTimer *)_timer;
- (bool) connection:(NSString * )ip ;




@end
