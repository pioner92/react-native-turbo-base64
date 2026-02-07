#import "TurboBase64.h"

@implementation TurboBase64
- (NSNumber *)multiply:(double)a b:(double)b {
    NSNumber *result = @(a * b);

    return result;
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
    return std::make_shared<facebook::react::NativeTurboBase64SpecJSI>(params);
}

+ (NSString *)moduleName
{
  return @"TurboBase64";
}

@end
