//
//  VCJoystickControl.m
//  Vagiosity-Controller
//
//  Created by Stepan Khapugin on 5/12/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VCJoystickControl.h"

@implementation VCJoystickControl

- (void) layoutSubviews {
    CAShapeLayer * rectLayer = [CAShapeLayer new];
    CGRect rect = CGRectMake(self.bounds.size.width/4, self.bounds.size.height/4,
                             self.bounds.size.width/2, self.bounds.size.height/2);
    rectLayer.path = CGPathCreateWithRect(rect, NULL);
    rectLayer.strokeColor = [UIColor whiteColor].CGColor;
    rectLayer.fillColor = [UIColor clearColor].CGColor;
    rectLayer.lineWidth = 3.0;
    
    [self.layer addSublayer:rectLayer];
}

- (void) setXValue:(NSInteger)xValue {
    
    if (xValue > 1.0){
        xValue = 1.0;
    }
    
    if (xValue < 1.0){
        xValue = -1.0;
    }
    
    _xValue = xValue;
    NSLog(@"x:%d", xValue);
}

- (void) setYValue:(NSInteger)yValue {
    
    if (yValue > 1.0){
        yValue = 1.0;
    }
    
    if (yValue < 1.0){
        yValue = -1.0;
    }
    
    _yValue = yValue;
    NSLog(@"y:%d", yValue);
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event{
    
    if (touches.count > 1) {
        [self resetValue];
        return;
    }
    
    UITouch * touch = [touches anyObject];
    
    CGPoint pt = [touch locationInView:self];
    
    [self updateValuesWithPoint:pt];
}

- (void) updateValuesWithPoint:(CGPoint) pt {
    
    CGFloat xOffset = pt.x - self.bounds.size.width/2;
    CGFloat yOffset = pt.y - self.bounds.size.height/2;
    self.xValue = xOffset/(self.bounds.size.width/2);
    self.yValue = yOffset/(self.bounds.size.height/2);

    [self sendActionsForControlEvents:UIControlEventValueChanged];
}

- (void) resetValue{
    self.xValue = 0;
    self.yValue = 0;
    [self sendActionsForControlEvents:UIControlEventValueChanged];
}

@end
