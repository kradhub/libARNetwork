//
//  ViewController.m
//  repeater
//
//  Created by Nicolas Payot on 14/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import "TextViewConsole.h"
#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

@synthesize pAppDelegate;
@synthesize textViewInfo;

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    [textfieldIP setDelegate:self];
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}



- (IBAction)clickConnection
{
    /*
     if( ![pAppDelegate connected] )
     {
        [pAppDelegate connection:textfieldIP.text] ;
     }
     else
     {
     
     }*/
    
    [pAppDelegate connection:textfieldIP.text] ;
    
}

-(void)touchesBegan :(NSSet *) touches withEvent:(UIEvent *)event
{
    [textfieldIP resignFirstResponder];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    
	[textField resignFirstResponder];
    
	return YES;
}


- (IBAction)clcikExit
{
    [pAppDelegate exit];
}


- (IBAction)clcikStop
{
   [pAppDelegate stopThreadRepeater];
}


@end
