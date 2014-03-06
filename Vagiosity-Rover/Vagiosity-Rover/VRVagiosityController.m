//
//  VRVagiosityController.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRVagiosityController.h"
#import "VRToneGenerator.h"

#define VAGIOSITY_ZERO_FREQ 2500
#define VAGIOSITY_SYNC_FREQ 2000

@interface VRVagiosityController()

@property (nonatomic, strong) VRToneGenerator * toneGenerator;

@end

@implementation VRVagiosityController

- (id) init {
    if (self = [super init]){
        self.toneGenerator = [VRToneGenerator new];
        self.packetsPerSecond = 1;
        [self.toneGenerator generateTonesForSequence:@[@(0),@(100),@(50),@(255)]];
        [self.toneGenerator togglePlay];
    }
    
    return self;
}

- (void) sendSynchroimpulses{
    [self.toneGenerator generateTonesForSequence:@[@(VAGIOSITY_SYNC_FREQ)]];
}

- (void) sendValues:(NSArray *)controlValues{
    NSAssert(controlValues.count == 4, @"You have to provide exactly 4 values for vagiosity controller to send!");
    
    for (NSNumber * num in controlValues){
        NSAssert([num intValue] >= 0 && [num intValue] <= 255, @"You must supply control values in 0..255 range!");
    }
    
    NSMutableArray * sentTones = [NSMutableArray new];
    for (int i = 0; i < self.packetsPerSecond; i++) {
        [sentTones addObject:@(VAGIOSITY_SYNC_FREQ)];
        for (int j = 0; j < 4; j++){
            [sentTones addObject:@([controlValues[j] intValue]*6 + VAGIOSITY_ZERO_FREQ)];
        }
    }
    
    [self.toneGenerator generateTonesForSequence:sentTones];
}

@end
