#import "TurboBase64.h"
#import <React/RCTBridge+Private.h>
#import <ReactCommon/RCTTurboModule.h>
#import <React/RCTBridge.h>
#import <React/RCTUtils.h>
#import <jsi/jsi.h>
#import "react-native-turbo-base64.h"

using namespace facebook;

@implementation RNTurboBase64
RCT_EXPORT_MODULE(RNTurboBase64)

@synthesize bridge = _bridge;
@synthesize methodQueue = _methodQueue;

+ (BOOL)requiresMainQueueSetup {
  return NO;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install){
	NSLog(@"Installing JSI bindings for react-native-turbo-base64 ...");
	RCTBridge* bridge = [RCTBridge currentBridge];
	RCTCxxBridge* cxxBridge = (RCTCxxBridge*)bridge;

	if (cxxBridge == nil) {
		return @false;
	}

	auto jsiRuntime = (jsi::Runtime*) cxxBridge.runtime;
	if (jsiRuntime == nil) {
		return @false;
	}

	rntb_base64::install(jsiRuntime);

	return @true;
}


@end
