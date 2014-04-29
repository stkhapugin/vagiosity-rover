//
//  VCViewController.m
//  Vagiosity-Controller
//
//  Created by Khapugin Stepan on 26/02/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VCViewController.h"
#import "VCControllerClient.h"

@interface VCViewController ()

@property (nonatomic, strong) VCControllerClient * client;

@end

@implementation VCViewController

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
	// Do any additional setup after loading the view, typically from a nib.
    self.client = [VCControllerClient new];
    [self.client connectIfPossible];
    self.client.connectionBlock = ^
    {
        [self.client sendValues:@[@(1), @(2), @(3), @(4)]];
    };
}

@end
