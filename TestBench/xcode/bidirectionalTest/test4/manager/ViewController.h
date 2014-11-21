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
//  ViewController.h
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <libNetwork/manager.h>

#import "TextViewConsole.h"

#import "AppDelegate.h"

@interface ViewController : UIViewController
{

    IBOutlet UITextField* textfieldIP;
    //IBOutlet UITextField* textfieldChar;
    //IBOutlet UITextField* textfieldIntAck;
    IBOutlet UITextField* textfieldType;

    IBOutlet UIButton* buttonCreate;
    IBOutlet UIButton* buttonConnection;
    IBOutlet UIButton* buttonSendChar;
    IBOutlet UIButton* buttonSendIntAck;
    IBOutlet UIButton* buttonSendStrAck;
    IBOutlet UIButton* buttonExit;
    
    IBOutlet TextViewConsole* textViewInfo;
    IBOutlet TextViewConsole* textViewInfoRecv;
    
    //id delegate;
    
    AppDelegate* pAppDelegate;

}

@property (nonatomic, retain) AppDelegate* pAppDelegate;
@property (nonatomic, readonly) IBOutlet TextViewConsole* textViewInfo;
@property (nonatomic, readonly) IBOutlet TextViewConsole* textViewInfoRecv;
//@property(nonatomic,assign)   id  delegate;


- (IBAction)clickCreate;
- (IBAction)clickConnection;

- (IBAction)clcikSendChar;
- (IBAction)clcikSendIntAck;
- (IBAction)clcikSendStrAck;

- (IBAction)clcikExit;


@end
