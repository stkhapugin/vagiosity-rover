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
@property (nonatomic, strong) MCNearbyServiceAdvertiser * advertiser;

@end

@implementation VRControllerServer

#pragma mark - setters and getters

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
    self.ownPeerID = [[MCPeerID alloc] initWithDisplayName:[[UIDevice currentDevice] name]];
    NSAssert(self.ownPeerID, @"");
    
    MCNearbyServiceAdvertiser *advertiser =
    [[MCNearbyServiceAdvertiser alloc] initWithPeer:self.ownPeerID
                                      discoveryInfo:nil
                                        serviceType:@"VAGINA"];
    
    NSAssert(advertiser, @"");
    
    advertiser.delegate = self;
    [advertiser startAdvertisingPeer];
    self.advertiser = advertiser; //< we are responsible to retain it
    NSLog(@"started advertising");
}

- (void) stopServer
{
    NSLog(@"stopping server");
    [self.session disconnect];
    self.session = nil;
}

- (void) sendData:(NSData *)data{
    
    if (self.session.connectedPeers.count == 0){
        return;
    }
    
    [self.session sendData:data
                   toPeers:self.session.connectedPeers
                  withMode:MCSessionSendDataUnreliable
                     error:nil];
}

#pragma mark - MCNearbyAdvertiserDelegate

- (void) advertiser:(MCNearbyServiceAdvertiser *)advertiser
didNotStartAdvertisingPeer:(NSError *)error{
    NSLog(@"%@", error);
}

- (void) advertiser:(MCNearbyServiceAdvertiser *)advertiser
didReceiveInvitationFromPeer:(MCPeerID *)peerID
        withContext:(NSData *)context
  invitationHandler:(void (^)(BOOL, MCSession *))invitationHandler
{
    self.session = [[MCSession alloc] initWithPeer:self.ownPeerID
                                  securityIdentity:nil
                              encryptionPreference:MCEncryptionNone];
    
    self.session.delegate = self;
    NSLog(@"Accepted peer: %@", [peerID displayName]);

    invitationHandler(YES, self.session);
}

#pragma mark - MCSessionDelegate

- (void) session:(MCSession *)session
            peer:(MCPeerID *)peerID
  didChangeState:(MCSessionState)state{
    
}

- (void) session:(MCSession *)session
didStartReceivingResourceWithName:(NSString *)resourceName
        fromPeer:(MCPeerID *)peerID
    withProgress:(NSProgress *)progress{
    
}

- (void) session:(MCSession *)session didReceiveStream:(NSInputStream *)stream withName:(NSString *)streamName fromPeer:(MCPeerID *)peerID{
    
}

- (void) session:(MCSession *)session didFinishReceivingResourceWithName:(NSString *)resourceName fromPeer:(MCPeerID *)peerID atURL:(NSURL *)localURL withError:(NSError *)error{
    
}

- (void)session:(MCSession *)session
 didReceiveData:(NSData *)data
       fromPeer:(MCPeerID *)peerID
{
    
    NSArray * values = [NSKeyedUnarchiver unarchiveObjectWithData:data];
    self.lastReceivedValues = values;
}
@end
