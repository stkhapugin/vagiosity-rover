//
//  VRControllerServer.h
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 27/04/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface VRControllerServer : NSObject

- (void) startServer;
- (void) stopServer;

/// KVO-compliant
@property (nonatomic, readonly) NSArray * lastReceivedValues;

@end
