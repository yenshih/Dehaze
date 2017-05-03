#import "Dehaze.h"
#import "OCDehaze.h"

@implementation Dehaze {
    OCDehaze *_dehazeInterface;
}

RCT_EXPORT_MODULE();

RCT_REMAP_METHOD(run,
                 uri:(NSString *)uri
                 media:(NSString *)media
                 initResolver:(RCTPromiseResolveBlock)resolve
                 initRejecter:(RCTPromiseRejectBlock)reject) {
    _dehazeInterface = [OCDehaze create];
    NSString *response = [_dehazeInterface dehaze:uri media:media];
    if (response) {
        resolve(response);
    } else {
        reject(@"get_error", @"Error with Dehaze", nil);
    }
}

@end
