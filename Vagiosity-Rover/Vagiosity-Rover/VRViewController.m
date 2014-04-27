//
//  VRViewController.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 26/02/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRViewController.h"
#import "VRVagiosityController.h"
#import "VRControllerServer.h"

@interface VRViewController ()
@property (nonatomic, strong) VRVagiosityController * controller;
@property (nonatomic, strong) VRControllerServer * server;
@end

@implementation VRViewController

- (void) dealloc {
    self.controller = nil;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSAssert(self.valueLabels.count == self.valueSliders.count, @"Number of values should match number of sliders");
    
    self.controller = [VRVagiosityController new];
    [self sendCurrentValues];
    self.server = [VRControllerServer new];
    [self.server startServer];
}

- (void) viewWillAppear:(BOOL)animated{
    [super viewWillAppear:animated];
    [self updateLabels];
}

- (void) sendCurrentValues {
    [self.controller sendValues:[self.valueSliders valueForKey:@"value"]];
}

- (void) updateLabels {
    for (int i = 0; i<self.valueLabels.count; i++){
        UILabel * label = self.valueLabels[i];
        UISlider * slider = self.valueSliders[i];
        label.text = [NSString stringWithFormat:@"%2.0f", slider.value];
    }
}

- (IBAction)switchValueChanged:(id)sender{
    [self updateLabels];
    [self sendCurrentValues];
}

@end
