//
//  ViewController.h
//  repeater
//
//  Created by Nicolas Payot on 14/11/12.
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
    
    IBOutlet UIButton* buttonConnection;

    IBOutlet UIButton* buttonExit;
    
    /*IBOutlet UITextView* */
    IBOutlet TextViewConsole* textViewInfo;
    
    AppDelegate* pAppDelegate;
}

@property (nonatomic, retain) AppDelegate* pAppDelegate;
@property (nonatomic, readonly) /*IBOutlet UITextView**/IBOutlet TextViewConsole* textViewInfo;
//@property(nonatomic,assign)   id  delegate;

- (IBAction)clickConnection;

- (IBAction)clcikStop;

- (IBAction)clcikExit;



@end
