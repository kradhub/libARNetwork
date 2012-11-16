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
    
    sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
    
}

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;

@property(nonatomic,assign)   bool connected;

- (void)startThreadManager;
- (void)stopThreadManager;
- (void)exit;

- (bool) connection:(NSString * )ip ;

- (bool) sendChar:(NSString * )data ;
- (bool) sendIntAck:(NSString * )data ;


@end
