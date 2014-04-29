//
//  VRToneGenerator.h
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface VRToneGenerator : NSObject

/** Use this method to generate a playable audio sequence from an integer sequence.
 It takes numbers from first argument and interpret them as 8-bit integer, which are then converted into levels on the right channel. Every second signal generated is a zero. The left channel consists only from 1s and 0s, with 1s corresponding to bits where the right channel has data in it.
 */
- (void) generateTonesForSequence:(NSArray *)sequence
                   leftChannelPtr:(Float32 **)leftChannelPtr
                  rightChannelPtr:(Float32 **)rightChannelPtr
              audioSequenceLength:(NSUInteger *)lengthPtr;

@end
