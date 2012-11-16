//
//  ViewController.h
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <libNetwork/common.h>
#include <libNetwork/inOutBuffer.h>
#include <libNetwork/sender.h>
#include <libNetwork/receiver.h>

#include <libNetwork/network.h>

#import "TextViewConsole.h"

#import "AppDelegate.h"

@interface ViewController : UIViewController
{

    IBOutlet UITextField* textfieldIP;
    //IBOutlet UITextField* textfieldChar;
    //IBOutlet UITextField* textfieldIntAck;

    IBOutlet UIButton* buttonConnection;
    IBOutlet UIButton* buttonSendChar;
    IBOutlet UIButton* buttonSendIntAck;
    IBOutlet UIButton* buttonExit;
    
    IBOutlet TextViewConsole* textViewInfo;
    
    //id delegate;
    
    AppDelegate* pAppDelegate;

}

@property (nonatomic, retain) AppDelegate* pAppDelegate;
@property (nonatomic, readonly) IBOutlet TextViewConsole* textViewInfo;
//@property(nonatomic,assign)   id  delegate;

- (IBAction)clickConnection;


- (IBAction)clcikSendChar;
- (IBAction)clcikSendIntAck;

- (IBAction)clcikExit;


@end
