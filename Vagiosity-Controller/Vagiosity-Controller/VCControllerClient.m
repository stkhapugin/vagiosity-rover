//
//  VCControllerClient.m
//  Vagiosity-Controller
//
//  Created by Khapugin Stepan on 27/04/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VCControllerClient.h"
#import <MultipeerConnectivity/MultipeerConnectivity.h>

@interface VCControllerClient () <MCNearbyServiceBrowserDelegate, MCSessionDelegate>

@property (nonatomic, strong) MCPeerID * ownPeerID;
@property (nonatomic, strong) MCSession * session;
@property (nonatomic, strong) MCNearbyServiceBrowser* browser;
@end

@implementation VCControllerClient

/// lazy getter
- (MCPeerID *) ownPeerID
{
    if (!_ownPeerID){
        self.ownPeerID = [[MCPeerID alloc] initWithDisplayName:[[UIDevice currentDevice] name]];
    }
    
    return _ownPeerID;
}

- (void) connectIfPossible
{
    if (!self.browser){
        MCNearbyServiceBrowser *browser =
        [[MCNearbyServiceBrowser alloc] initWithPeer:self.ownPeerID
                                         serviceType:@"VAGINA"];
        browser.delegate = self;
        self.browser = browser;
    }
    
    if (!self.session){
        self.session = [[MCSession alloc] initWithPeer:self.ownPeerID];
        self.session.delegate = self;
    }
    
    [browser startBrowsingForPeers];
    NSLog(@"started browsing");
}

- (void) sendValues:(NSArray *)values {
    
    if (!self.session){
        return;
    }
    
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:values];
    NSError *error = nil;
    
    if (![self.session sendData:data
                        toPeers:self.session.connectedPeers
                       withMode:MCSessionSendDataReliable
                          error:&error]) {
        NSLog(@"[Error] %@", error);
    }}

#pragma mark - MCNearbyServiceBrowserDelegate

- (void) browser:(MCNearbyServiceBrowser *)browser didNotStartBrowsingForPeers:(NSError *)error{
    NSLog(@"%@", error);
}

- (void) browser:(MCNearbyServiceBrowser *)browser
       foundPeer:(MCPeerID *)peerID
withDiscoveryInfo:(NSDictionary *)info{
    
    [browser invitePeer:peerID toSession:self.session withContext:nil timeout:60.0];
    NSLog(@"found peer");
}

- (void) browser:(MCNearbyServiceBrowser *)browser lostPeer:(MCPeerID *)peerID{
    [browser startBrowsingForPeers];
    NSLog(@"lost peer");
}

#pragma mark - MCSessionDelegate

- (void) session:(MCSession *)session peer:(MCPeerID *)peerID didChangeState:(MCSessionState)state
{
    if (state==MCSessionStateNotConnected)
    {
        NSLog(@"Disconnected!");
        [session disconnect];
        self.session = nil;
        
        [self connectIfPossible];
    }
    
    if (state == MCSessionStateConnected)
    {
        NSLog(@"Connected!");
        if (self.connectionBlock){
            self.connectionBlock();
        }
        
        [self.browser stopBrowsingForPeers];
        self.browser = nil;
    }
}

- (void) session:(MCSession *)session didReceiveStream:(NSInputStream *)stream withName:(NSString *)streamName fromPeer:(MCPeerID *)peerID{
    
}

- (void) session:(MCSession *)session didStartReceivingResourceWithName:(NSString *)resourceName fromPeer:(MCPeerID *)peerID withProgress:(NSProgress *)progress{
    
}

- (void) session:(MCSession *)session didReceiveData:(NSData *)data fromPeer:(MCPeerID *)peerID{
    
}

- (void) session:(MCSession *)session didFinishReceivingResourceWithName:(NSString *)resourceName fromPeer:(MCPeerID *)peerID atURL:(NSURL *)localURL withError:(NSError *)error{
    
}
@end
