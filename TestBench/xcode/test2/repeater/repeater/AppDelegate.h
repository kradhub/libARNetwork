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

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
    network_t* pNetwork2;
    
    bool connected;
    int connectError;
    int bindError;
    
    sal_thread_t thread_send2;
	sal_thread_t thread_recv2;
}

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;


@property(nonatomic,assign)   bool connected;

- (void)startThreadRepeater;
- (void)stopThreadRepeater;
- (void)exit;
- (void) print;
- (bool) connection:(NSString * )ip ;


@end
