/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
//
//  AppDelegate.h
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <libARSAL/ARSAL_Thread.h>

#include <libNetwork/manager.h>

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>{

    network_manager_t* pManager1;
    
    int netType;
    
    char cmdType;
    char chData;
    
    int intData;
    int dataDeportSize_ack;
    char* pDataDeported_ack;
    
    int dataDeportSize;
    char* pDataDeported;
    
    char IpAddress[16];
    int sendPort;
    int recvPort;
    
    int connectError;
    
    int id_ioBuff_char;
    int id_ioBuff_intAck;
    int id_ioBuff_deportDataAck;
    int id_ioBuff_deportData;
    
    int id_print_ioBuff_char;
    int id_print_ioBuff_intAck;
    int id_print_ioBuff_deportDataAck;
    int id_print_ioBuff_deportData;
    
    ARSAL_Thread_t thread_send1;
    ARSAL_Thread_t thread_recv1;
    
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
- (bool) sendStrAck:(NSString * )data ;

- (void)print:(NSTimer *)_timer;

- (void) netWorkConstructor;


@end

int callBackDepotData(int OutBufferId, void* pData, int status);
