//
//  VRViewController.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 26/02/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRViewController.h"
#import "VRVagiosityController.h"


@interface VRViewController ()
@property (nonatomic, strong) VRVagiosityController * controller;
@property (nonatomic, weak) IBOutlet UISwitch * freqSwitch;
@end

@implementation VRViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Override point for customization after application launch.
    self.controller = [VRVagiosityController new];
    [self switchPositionChanged:self.freqSwitch];
}

- (IBAction)switchPositionChanged:(UISwitch *)sender{
    if (sender.isOn){
        [self.controller sendValues:@[@(0), @(100), @(255), @(42)]];
    } else {
        [self.controller sendSynchroimpulses];
    }
}

- (void) dealloc {
    self.controller = nil;
}

- (IBAction)segmentedControlValueChanged:(UISegmentedControl *)sender{
    self.controller.packetsPerSecond = (int)pow(2, sender.selectedSegmentIndex);
}

@end
