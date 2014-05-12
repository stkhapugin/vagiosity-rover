//
//  VCViewController.m
//  Vagiosity-Controller
//
//  Created by Khapugin Stepan on 26/02/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VCViewController.h"
#import "VCControllerClient.h"
#import "VCJoystickControl.h"

@interface VCViewController ()

@property (nonatomic, weak) IBOutlet VCJoystickControl * cameraJoystick;
@property (nonatomic, weak) IBOutlet VCJoystickControl * movementJoystick;

@property (nonatomic, strong) VCControllerClient * client;

@end

@implementation VCViewController

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
	// Do any additional setup after loading the view, typically from a nib.
    self.client = [VCControllerClient new];
    [self.client connectIfPossible];
    
    [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(tick) userInfo:nil repeats:YES];
}

- (void) tick {
    
    NSMutableArray * values = [NSMutableArray new];
    
    [values addObject:[self valueForJoystickValue:self.movementJoystick.xValue]];
    [values addObject:[self valueForJoystickValue:self.movementJoystick.yValue]];

    [values addObject:[self valueForJoystickValue:self.cameraJoystick.xValue]];
    [values addObject:[self valueForJoystickValue:self.cameraJoystick.yValue]];
    
    [self.client sendValues:values];
}

// converts -1..1 float to -127..127 integer
- (NSNumber *)valueForJoystickValue:(float)val {
    return @(-val*127);
}

@end
