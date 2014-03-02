//
//  VRToneGenerator.h
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface VRToneGenerator : NSObject

- (OSStatus) renderToneWithAudioBufferList:(AudioBufferList **)ioDataStar inNumberFrames:(UInt32)inNumberFrames;
- (void)stop;
- (void)togglePlay;

/// Use this method to play a sequence of frequencies. It will play for one second, then repeat.
- (void) generateTonesForSequence:(NSArray *)sequence;

@end
