//
//  VRVagiosityController.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRVagiosityController.h"
#import "VRToneGenerator.h"
#import "VRTonePlayer.h"

@interface VRVagiosityController()

@property (nonatomic, strong) VRToneGenerator * toneGenerator;
@property (nonatomic, strong) VRTonePlayer * tonePlayer;

@end

@implementation VRVagiosityController

- (id) init {
    if (self = [super init]){
        self.toneGenerator = [VRToneGenerator new];
        self.tonePlayer = [VRTonePlayer new];
    }
    
    return self;
}

- (void) sendValues:(NSArray *)controlValues{
    NSAssert(controlValues.count == 4, @"You have to provide exactly 4 values for vagiosity controller to send!");
    
    for (NSNumber * num in controlValues){
        NSAssert([num intValue] >= 0 && [num intValue] <= 255, @"You must supply control values in 0..255 range!");
    }
    
    Float32 *leftChannel, *rightChannel;
    NSUInteger length;
    [self.toneGenerator generateTonesForSequence:controlValues
                                  leftChannelPtr:&leftChannel
                                 rightChannelPtr:&rightChannel
                             audioSequenceLength:&length];
    
    [self.tonePlayer setLeftChannelBits:leftChannel
                       rightChannelBits:rightChannel
                           signalLength:length];
}

- (void) playValuesOnce{
    [self.tonePlayer playOnce];
}
- (void) playValuesContinously{
    [self.tonePlayer playContinously];
}

@end
