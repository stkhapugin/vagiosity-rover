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
#import "VRCameraCapturer.h"

@interface VRViewController ()<VRCameraCapturerDelegate>
@property (nonatomic, strong) VRVagiosityController * controller;
@property (nonatomic, strong) VRControllerServer * server;
@property (nonatomic, strong) VRCameraCapturer * capturer;
@end

@implementation VRViewController

- (void) dealloc {
    self.controller = nil;
    [self.server removeObserver:self forKeyPath:@"lastReceivedValues"];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSAssert(self.valueLabels.count == self.valueSliders.count, @"Number of values should match number of sliders");
    
    self.controller = [VRVagiosityController new];
    [self sendCurrentValues];
    self.server = [VRControllerServer new];
    [self.server startServer];
    
    self.capturer = [VRCameraCapturer new];
    self.capturer.delegate = self;
    [self.capturer startCameraCapture];
    
    [self.server addObserver:self forKeyPath:@"lastReceivedValues" options:0 context:nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath
                       ofObject:(id)object
                         change:(NSDictionary *)change
                        context:(void *)context{
    
    if ([keyPath isEqualToString:@"lastReceivedValues"] && object == self.server){
        [self receiveValues];
    }
}

- (void) receiveValues{
    if (![NSThread isMainThread]){
        [self performSelectorOnMainThread:@selector(receiveValues)
                               withObject:nil
                            waitUntilDone:NO];
        return;
    }
    
    NSAssert(self.server.lastReceivedValues.count == self.valueSliders.count, @"");
    
    for (int i = 0; i < self.valueSliders.count; i++){
        UISlider * slider = self.valueSliders[i];
        NSNumber * val = self.server.lastReceivedValues[i];
        
        slider.value = [val floatValue] + 128;
    }
    
    [self updateLabels];
    [self sendCurrentValues];
    [self.controller playValuesOnce];
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
        label.text = [NSString stringWithFormat:@"%2.0x", (int)slider.value];
    }
}

- (IBAction)switchValueChanged:(id)sender{
    [self updateLabels];
    [self sendCurrentValues];
}

- (IBAction)playOncePressed:(id)sender{
    [self.controller playValuesOnce];
}

- (IBAction)playPressed:(id)sender{
    [self.controller playValuesContinously];
}

- (void) receiveNewFrame:(UIImage *)frame{
    dispatch_async(dispatch_get_main_queue(), ^{
        self.imageView.image = frame;
    });
}
@end
