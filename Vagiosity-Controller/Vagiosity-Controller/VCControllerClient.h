//
//  VCControllerClient.h
//  Vagiosity-Controller
//
//  Created by Khapugin Stepan on 27/04/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface VCControllerClient : NSObject

- (void) connectIfPossible;
- (void) sendValues:(NSArray *)values;

@property (nonatomic, copy) void(^connectionBlock)();
@property (nonatomic, copy) void(^frameReceivedBlock)(UIImage * frame);

@end
