//
//  VCJoystickControl.h
//  Vagiosity-Controller
//
//  Created by Stepan Khapugin on 5/12/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface VCJoystickControl : UIControl

@property (nonatomic) CGFloat xValue; // -1 .. 1
@property (nonatomic) CGFloat yValue; // -1 .. 1

@end
