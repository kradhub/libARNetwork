//
//  AppDelegate.h
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <libSAL/thread.h>

#include <libNetwork/manager.h>

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>{

    network_manager_t* pManager1;
	
	int netType;
	
	char cmdType;
	char chData;
	
	int intData;
	
	char IpAddress[16];
    int sendPort;
	int recvPort;
    
	int connectError;
	
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    
    int id_print_ioBuff_char;
    int id_print_ioBuff_intAck;
	
	sal_thread_t thread_send1;
	sal_thread_t thread_recv1;
    
    NSTimer *timer;
    
    bool connected;
    
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
