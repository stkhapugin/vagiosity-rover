//
//  VRTonePlayer.h
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 27/04/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>

/// Plays stereo sound which is provided to it through audio session. The sound loops automatically.
@interface VRTonePlayer : NSObject

- (void) setLeftChannelBits:(Float32 *)leftChannelBits rightChannelBits:(Float32 *)rightChannelBits signalLength:(NSUInteger)length;

- (void)stop;
- (void)togglePlay;

@end
