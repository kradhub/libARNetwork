//
//  AppDelegate.h
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <libSAL/thread.h>

#include <libNetwork/common.h>
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>

#include <libNetwork/network.h>

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>{

    network_t* pNetwork1;
    
    bool connected;
    int connectError;
    int bindError;
    
    
    char chData;
    int intData;
    
    int netType;
    
    sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
    
    network_inOutBuffer_t* pOutBuffChar;
	network_inOutBuffer_t* pOutBuffIntAck;
    
    network_inOutBuffer_t* pInputBuffChar ;
    network_inOutBuffer_t* pInputBuffIntAck;
    
    NSTimer *timer;
    
}

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;

@property(nonatomic,assign)   bool connected;
@property(nonatomic,assign)  int netType;

- (void)startThreadManager;
- (void)stopThreadManager;
- (void)exit;

- (bool) connection:(NSString * )ip;

- (bool) sendChar:(NSString * )data ;
- (bool) sendIntAck:(NSString * )data ;

- (void)print:(NSTimer *)_timer;

- (void) netWorkConstructor;

@end
