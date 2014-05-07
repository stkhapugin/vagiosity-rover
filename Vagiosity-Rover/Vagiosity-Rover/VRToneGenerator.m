//
//  VRToneGenerator.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRToneGenerator.h"

#pragma mark -
#pragma mark VRToneGenerator implementation

@interface VRToneGenerator()

@property (nonatomic) double amplitude;
@property (nonatomic) double theta;
@property (nonatomic) int sampleRate;
@property (nonatomic) int lastFrame;

@property (nonatomic) NSArray * audioSequence;
@property (nonatomic) int currentFrame;


@end

@implementation VRToneGenerator

#define amplitude 1.1

- (void) generateTonesForSequence:(NSArray *)sequence
                   leftChannelPtr:(Float32 **)leftChannelPtr
                  rightChannelPtr:(Float32 **)rightChannelPtr
              audioSequenceLength:(NSUInteger *)lengthPtr{
    
    // data length
    NSUInteger len = sequence.count * 8 * 2; //< 8 bit per number; one sync bit per data bit
    
    Float32* leftChannel  = malloc(sizeof(Float32)*len);
    Float32* rightChannel = malloc(sizeof(Float32)*len);
    
    NSUInteger offset = 0;

    for (NSNumber * number in sequence){
        
        NSUInteger val = [number unsignedIntegerValue];
        
        if (val > 255){
            val = 255;
        }
        
        for (int bit = 7; bit >= 0; bit --){
            BOOL bitval = (val & (1 << bit)) >> bit;
            NSAssert((bitval == 0) || (bitval == 1), @"");
            
            // sync bit
            leftChannel[offset] = 0;
            rightChannel[offset] = 0;
            offset ++;
            
            // data bit
            leftChannel[offset] = amplitude;
            rightChannel[offset] = amplitude * bitval;
            offset ++;
        }
    }
    
    // returning results
    *leftChannelPtr = leftChannel;
    *rightChannelPtr = rightChannel;
    *lengthPtr = len;
}


@end
