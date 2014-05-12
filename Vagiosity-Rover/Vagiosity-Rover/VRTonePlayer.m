//
//  VRTonePlayer.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 27/04/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRTonePlayer.h"
#import <AudioToolbox/AudioToolbox.h>

@interface VRTonePlayer()

- (OSStatus) renderToneWithAudioBufferList:(AudioBufferList **)ioDataStar inNumberFrames:(UInt32)inNumberFrames;
- (void) stop;

@property (atomic) Float32 * leftChannelBuffer;
@property (atomic) Float32 * rightChannelBuffer;
@property (nonatomic) NSUInteger signalLength;
@property (nonatomic) NSUInteger lastBufferedFrame;
@property (nonatomic) AudioComponentInstance toneUnit;
@property (nonatomic) int sampleRate;
@property (nonatomic) BOOL playsOnce;
@property (nonatomic) BOOL writesZeroes;
@property (nonatomic, retain) NSLock * audioLock;

@end
    
#pragma mark -
#pragma mark Audio Callbacks

OSStatus RenderTone(
                    void *inRefCon,
                    AudioUnitRenderActionFlags 	*ioActionFlags,
                    const AudioTimeStamp 		*inTimeStamp,
                    UInt32 						inBusNumber,
                    UInt32 						inNumberFrames,
                    AudioBufferList 			*ioData)

{
    // we're gonna handle everything in our method so that we don't have to struggle with properties
    VRTonePlayer *player = (__bridge VRTonePlayer *)inRefCon;
    return [player renderToneWithAudioBufferList:&ioData inNumberFrames:inNumberFrames];
}

void ToneInterruptionListener(void *inClientData, UInt32 inInterruptionState)
{
    VRTonePlayer *player = (__bridge VRTonePlayer *)inClientData;
    [player stop];
}

@implementation VRTonePlayer

- (void)dealloc {
	AudioSessionSetActive(false);
}

- (id) init {
    self = [super init];
    if (self){
        [self initializeAudioSession];
        self.audioLock = [NSLock new];
    }
    
    return self;
}

- (OSStatus) renderToneWithAudioBufferList:(AudioBufferList **)ioDataStar inNumberFrames:(UInt32)inNumberFrames{
    
    [self.audioLock lock];
    if (!self.toneUnit){
        return noErr;
    }
    
    AudioBufferList * ioData = *ioDataStar;
        
	Float32 *lBuffer = (Float32 *)ioData->mBuffers[0].mData;
    Float32 *rBuffer = (Float32 *)ioData->mBuffers[1].mData;

    NSAssert((lBuffer != NULL)&&(rBuffer != NULL), @"You have to have a stereo audio session initialized");

    UInt32 framesPerNumber = 8;
	// Generate the samples
	for (UInt32 frame = 0; frame < inNumberFrames; frame++)
	{
        if (!self.writesZeroes){
            lBuffer[frame] = self.leftChannelBuffer[self.lastBufferedFrame];
            rBuffer[frame] = self.rightChannelBuffer[self.lastBufferedFrame];
        } else {
            lBuffer[frame] = 0;
            rBuffer[frame] = 0;
        }
        
        if (frame%framesPerNumber == 0)
            self.lastBufferedFrame ++;
        
        if (self.lastBufferedFrame > self.signalLength){
            self.lastBufferedFrame = 0;

            if (self.playsOnce){
 
                self.playsOnce = NO;
                self.writesZeroes = YES;
            }
        }
    }
    [self.audioLock unlock];
    return noErr;
}

- (void)createToneUnit
{
	// Configure the search parameters to find the default playback output unit
	// (called the kAudioUnitSubType_RemoteIO on iOS but
	// kAudioUnitSubType_DefaultOutput on Mac OS X)
	AudioComponentDescription defaultOutputDescription;
	defaultOutputDescription.componentType = kAudioUnitType_Output;
	defaultOutputDescription.componentSubType = kAudioUnitSubType_RemoteIO;
	defaultOutputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
	defaultOutputDescription.componentFlags = 0;
	defaultOutputDescription.componentFlagsMask = 0;
	
	// Get the default playback output unit
	AudioComponent defaultOutput = AudioComponentFindNext(NULL, &defaultOutputDescription);
	NSAssert(defaultOutput, @"Can't find default output");
	
	// Create a new unit based on this that we'll use for output
	OSErr err = AudioComponentInstanceNew(defaultOutput, &_toneUnit);
	NSAssert1(self.toneUnit, @"Error creating unit: %hd", err);
	
	// Set our tone rendering function on the unit
	AURenderCallbackStruct input;
	input.inputProc = RenderTone;
	input.inputProcRefCon = (__bridge void *)(self);
	err = AudioUnitSetProperty(self.toneUnit,
                               kAudioUnitProperty_SetRenderCallback,
                               kAudioUnitScope_Input,
                               0,
                               &input,
                               sizeof(input));
	NSAssert1(err == noErr, @"Error setting callback: %hd", err);
	
	// Set the format to 32 bit, single channel, floating point, linear PCM
	const int four_bytes_per_float = 4;
	const int eight_bits_per_byte = 8;
	AudioStreamBasicDescription streamFormat;
	streamFormat.mSampleRate = self.sampleRate;
	streamFormat.mFormatID = kAudioFormatLinearPCM;
	streamFormat.mFormatFlags =
    kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
	streamFormat.mBytesPerPacket = four_bytes_per_float;
	streamFormat.mFramesPerPacket = 1;
	streamFormat.mBytesPerFrame = four_bytes_per_float;
	streamFormat.mChannelsPerFrame = 2;
	streamFormat.mBitsPerChannel = four_bytes_per_float * eight_bits_per_byte;
	err = AudioUnitSetProperty (self.toneUnit,
                                kAudioUnitProperty_StreamFormat,
                                kAudioUnitScope_Input,
                                0,
                                &streamFormat,
                                sizeof(AudioStreamBasicDescription));
	NSAssert1(err == noErr, @"Error setting stream format: %hd", err);
}

- (void)togglePlay
{
    [self.audioLock lock];
    
	if (self.toneUnit)
	{
		AudioOutputUnitStop(self.toneUnit);
		AudioUnitUninitialize(self.toneUnit);
		AudioComponentInstanceDispose(self.toneUnit);
		self.toneUnit = nil;
	}
	else
	{
		[self createToneUnit];
		
		// Stop changing parameters on the unit
		OSErr err = AudioUnitInitialize(self.toneUnit);
		NSAssert1(err == noErr, @"Error initializing unit: %hd", err);
		
		// Start playback
		err = AudioOutputUnitStart(self.toneUnit);
		NSAssert1(err == noErr, @"Error starting unit: %hd", err);
    }
    
    [self.audioLock unlock];
}

- (void)stop
{
	if (self.toneUnit)
	{
		[self togglePlay];
	}
}

- (void) start {
    if (!self.toneUnit){
        [self togglePlay];
    }
}

- (void)initializeAudioSession {
	self.sampleRate = 44100;
    
	OSStatus result = AudioSessionInitialize(NULL, NULL, ToneInterruptionListener, (__bridge void *)self);
	if (result == kAudioSessionNoError)
	{
		UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
		AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
	}
	AudioSessionSetActive(true);
    
}

- (void) setLeftChannelBits:(Float32 *)leftChannelBits
           rightChannelBits:(Float32 *)rightChannelBits
               signalLength:(NSUInteger)length{
    
    
    NSAssert(length > 0, @"Cannot play an empty sequence");

    if (![NSThread isMainThread]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setLeftChannelBits:leftChannelBits
                    rightChannelBits:rightChannelBits
                        signalLength:length];
        });
        
        return;
    }
    
    //[self stop];
    [self.audioLock lock];
    self.leftChannelBuffer = leftChannelBits;
    self.rightChannelBuffer = rightChannelBits;
    self.signalLength = length;
    [self.audioLock unlock];
}

- (void) playOnce{
    //[self stop];
    self.playsOnce = YES;
    self.writesZeroes = NO;
    //[self start];
}

- (void) playContinously{
    //[self stop];
    self.playsOnce = NO;
    self.writesZeroes = NO;
    //[self start];
}


@end
