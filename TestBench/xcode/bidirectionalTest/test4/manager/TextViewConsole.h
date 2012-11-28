//
//  TextViewConsole.h
//  
//
//  Created by Nicolas Payot on 12/11/12.
//  Copyright (c) 2012 Parrot SA. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface TextViewConsole : UITextView
{
    
}
-(void) appendText:(NSString *)text;
- (void) goToEndOfNote;
@end
