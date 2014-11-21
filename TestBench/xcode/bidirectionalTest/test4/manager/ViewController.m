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
//  ViewController.m
//  manager
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import "ViewController.h"

//#import "AppDelegate.h"

@interface ViewController ()

@end

@implementation ViewController

@synthesize pAppDelegate;
@synthesize textViewInfo;
@synthesize textViewInfoRecv;

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    [textfieldIP setDelegate:self];
    [textfieldType setDelegate:self];
    //[textfieldChar setDelegate:self];
    //[textfieldIntAck setDelegate:self];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self setPAppDelegate:nil];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)clickCreate
{
    int type = [textfieldType.text intValue];
    
    if(type == 1 || type == 2)
    {
        [pAppDelegate setNetType: type];
        [pAppDelegate netWorkConstructor];
        
        [pAppDelegate startThreadManager];
        
        [buttonCreate setEnabled:NO];
        
        [textfieldIP setEnabled:YES];
        [buttonConnection setEnabled:YES];
        
        [textViewInfo appendText:@"Created \n"];
        
    }
    else
    {
        textfieldType.text = @"";
    }
}


- (IBAction)clickConnection
{
    [pAppDelegate connection:textfieldIP.text] ;
    
    if( [pAppDelegate connected] )
    {

        [buttonSendChar setEnabled:YES];
        [buttonSendIntAck setEnabled:YES];
        [buttonSendStrAck setEnabled:YES];
    }
    
}

-(void) touchesBegan :(NSSet *) touches withEvent:(UIEvent *)event
{
    [textfieldIP resignFirstResponder];
    [textfieldType resignFirstResponder];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    
	[textField resignFirstResponder];
    
	return YES;
}

- (IBAction)clcikSendChar
{
    [pAppDelegate sendChar:@""];
    
    
}

- (IBAction)clcikSendIntAck
{
    if( [pAppDelegate sendIntAck:@""] )
    {
        [textViewInfo appendText:@" buffer full !!!!!!!!!!! \n"];
    }
}

- (IBAction)clcikSendStrAck
{
    if( [pAppDelegate sendStrAck:@""] )
    {
        [textViewInfo appendText:@" buffer full !!!!!!!!!!! \n"];
    }
}


- (IBAction)clcikExit
{
    [pAppDelegate exit];
}


@end
