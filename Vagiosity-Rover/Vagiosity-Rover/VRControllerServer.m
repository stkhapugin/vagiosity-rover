//
//  VRControllerServer.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 27/04/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRControllerServer.h"
#import <MultipeerConnectivity/MultipeerConnectivity.h>

@interface VRControllerServer () <MCNearbyServiceAdvertiserDelegate, MCSessionDelegate>

@property (nonatomic, strong) MCPeerID * ownPeerID;
@property (nonatomic, strong) MCSession * session;
@property (nonatomic, strong) NSArray * lastReceivedValues;

@end

@implementation VRControllerServer

#pragma mark - setters and getters

- (MCPeerID *) ownPeerID
{
    if (!_ownPeerID){
        self.ownPeerID = [[MCPeerID alloc] initWithDisplayName:[[UIDevice currentDevice] name]];
    }
    
    return _ownPeerID;
}

/// KVO setter
- (void) setLastReceivedValues:(NSArray *)lastReceivedValues
{
    if ([_lastReceivedValues isEqualToArray:lastReceivedValues]){
        return;
    }

    [self willChangeValueForKey:@"lastReceivedValues"];
    _lastReceivedValues = lastReceivedValues;
    [self didChangeValueForKey:@"lastReceivedValues"];
}

#pragma mark - public methods

- (void) startServer
{
    MCNearbyServiceAdvertiser *advertiser =
    [[MCNearbyServiceAdvertiser alloc] initWithPeer:self.ownPeerID
                                      discoveryInfo:nil
                                        serviceType:@"vagiosityctrl"];
    advertiser.delegate = self;
    [advertiser startAdvertisingPeer];
}

- (void) stopServer
{
    [self.session disconnect];
    self.session = nil;
}

#pragma mark - MCNearbyAdvertiserDelegate

- (void) advertiser:(MCNearbyServiceAdvertiser *)advertiser didNotStartAdvertisingPeer:(NSError *)error{
    NSLog(@"%@", error);
}

- (void) advertiser:(MCNearbyServiceAdvertiser *)advertiser
didReceiveInvitationFromPeer:(MCPeerID *)peerID
        withContext:(NSData *)context
  invitationHandler:(void (^)(BOOL, MCSession *))invitationHandler
{
    if (self.session != nil){
        NSLog(@"Blocked duplicate peer: %@", [peerID displayName]);
        invitationHandler(NO, nil);
        return;
    }
    
    self.session = [[MCSession alloc] initWithPeer:self.ownPeerID
                                  securityIdentity:nil
                              encryptionPreference:MCEncryptionNone];
    
    self.session.delegate = self;
    invitationHandler(YES, self.session);
    NSLog(@"Accepted peer: %@", [peerID displayName]);
}

#pragma mark - MCSessionDelegate

- (void)session:(MCSession *)session
 didReceiveData:(NSData *)data
       fromPeer:(MCPeerID *)peerID
{
    NSKeyedUnarchiver * unarch = [[NSKeyedUnarchiver alloc] initForReadingWithData:data];
    NSArray * values = [unarch decodeObject];
    [unarch finishDecoding];
    self.lastReceivedValues = values;
    
    NSLog(@"Received values: %@", values);
}
@end
