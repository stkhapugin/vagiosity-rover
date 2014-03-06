//
//  VRVagiosityController.h
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface VRVagiosityController : NSObject

- (void) sendValues:(NSArray *)controlValues;
- (void) sendSynchroimpulses;

@property (nonatomic) int packetsPerSecond;


@end
