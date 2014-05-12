//
//  VRCameraCapturer.h
//  Vagiosity-Rover
//
//  Created by Stepan Khapugin on 5/12/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol VRCameraCapturer <NSObject>

- (void) receiveNewFrame:(UIImage *)frame;

@end

@interface VRCameraCapturer : NSObject

- (void) startCameraCapture;

@end
