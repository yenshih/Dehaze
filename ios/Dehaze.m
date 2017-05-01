#import "Dehaze.h"
#import "OCDehaze.h"

@implementation Dehaze {
    OCDehaze *_dehazeInterface;
}

RCT_EXPORT_MODULE();

RCT_REMAP_METHOD(init,
                 uri:(NSString *)uri
                 initResolver:(RCTPromiseResolveBlock)resolve
                 initRejecter:(RCTPromiseRejectBlock)reject) {
    _dehazeInterface = [OCDehaze create];
    NSString *response = [_dehazeInterface dehazeImage: uri];
    if (response) {
        resolve(response);
    } else {
        reject(@"get_error", @"Error with init", nil);
    }
}

@end
