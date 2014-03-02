//
//  VRToneGenerator.m
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 02/03/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import "VRToneGenerator.h"

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
    VRToneGenerator *generator = (__bridge VRToneGenerator *)inRefCon;
    return [generator renderToneWithAudioBufferList:&ioData inNumberFrames:inNumberFrames];
}

void ToneInterruptionListener(void *inClientData, UInt32 inInterruptionState)
{
    VRToneGenerator *generator = (__bridge VRToneGenerator *)inClientData;
    [generator stop];
}

#pragma mark -
#pragma mark VRToneGenerator implementation

@interface VRToneGenerator()

@property (nonatomic) double amplitude;
@property (nonatomic) double theta;
@property (nonatomic) int sampleRate;
@property (nonatomic) int lastFrame;
@property (nonatomic) AudioComponentInstance toneUnit;

@property (nonatomic) NSArray * audioSequence;
@property (nonatomic) int currentFrame;


@end

@implementation VRToneGenerator

- (id) init {
    self = [super init];
    if (self){
        [self initializeAudioSession];
    }
    
    return self;
}

- (double) frequencyForFrame:(int)frame {
    int numberOfTonesPerSecond = self.audioSequence.count;
    int framesPerTone = self.sampleRate/numberOfTonesPerSecond;
    int numberOfTone = frame / framesPerTone;
    double ans = [self.audioSequence[numberOfTone] doubleValue];
    return 2000.0;//ans;
}

- (void) generateTonesForSequence:(NSArray *)sequence{
    self.audioSequence = sequence;
}

- (OSStatus) renderToneWithAudioBufferList:(AudioBufferList **)ioDataStar inNumberFrames:(UInt32)inNumberFrames{
    
    AudioBufferList * ioData = *ioDataStar;
    // Fixed amplitude is good enough for our purposes
	const double amplitude = 0.25;

	double theta = self.theta;
    
	// This is a mono tone generator so we only need the first buffer
	const int channel = 0;
	Float32 *buffer = (Float32 *)ioData->mBuffers[channel].mData;
	
	// Generate the samples
	for (UInt32 frame = 0; frame < inNumberFrames; frame++)
	{
        int numberOfTonesPerSecond = self.audioSequence.count;
        int framesPerTone = self.sampleRate/numberOfTonesPerSecond;
        int numberOfTone = self.lastFrame / framesPerTone;
        numberOfTone = numberOfTone % numberOfTonesPerSecond;
        double f = [self.audioSequence[numberOfTone] doubleValue];

        double theta_increment = 2.0 * M_PI * f / (double)self.sampleRate;

		buffer[frame] = sin(theta) * amplitude;
		
		theta += theta_increment;
		if (theta > 2.0 * M_PI)
		{
			theta -= 2.0 * M_PI;
		}
        self.lastFrame ++;
	}
    
    if (self.lastFrame >= self.sampleRate){
        self.lastFrame = 0;
    }
	
	// Store the theta
	self.theta = theta;
    
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
	streamFormat.mChannelsPerFrame = 1;
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
}

- (void)stop
{
	if (self.toneUnit)
	{
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

- (void)dealloc {
	AudioSessionSetActive(false);
}



@end
