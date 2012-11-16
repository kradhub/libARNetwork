//
//  TextViewConsole.m
//  
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import "TextViewConsole.h"

@implementation TextViewConsole

-(void) appendText:(NSString *)text
{
    [super setText: [ self.text stringByAppendingString: text ] ];
    
    [self goToEndOfNote];
    
}

- (void) goToEndOfNote
{
    NSUInteger length = self.text.length;
    self.selectedRange = NSMakeRange(length, 0);
    //[self setContentOffset:CGPointMake(0, length) animated:YES];
}

@end
