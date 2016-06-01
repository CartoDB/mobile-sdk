#import  "MapLicenseManagerListener.h"
#include "utils/Log.h"

@interface NTMapLicenseManagerListener() { }

@property (weak, nonatomic) NSString* licenseKey;
@property (weak, nonatomic) NSString* defaultsKeyOld;
@property (weak, nonatomic) NSString* defaultsKeyNew;

@end

@implementation NTMapLicenseManagerListener

-(id)initWithLicenseKey:(NSString*)licenseKey defaultsKeyOld:(NSString*)oldKey defaultsKeyNew:(NSString*)newKey {
    self = [super init];
    _licenseKey = licenseKey;
    _defaultsKeyOld = oldKey;
    _defaultsKeyNew = newKey;
    return self;
}

-(void)onLicenseUpdated:(NSString*)licenseKey {
    @try {
        [[NSUserDefaults standardUserDefaults] setObject:_licenseKey forKey:_defaultsKeyOld];
        [[NSUserDefaults standardUserDefaults] setObject:licenseKey forKey:_defaultsKeyNew];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
    @catch (NSException* e) {
        carto::Log::Error("NTMapLicenseManagerListener: Failed to update cached key");
    }
}

@end
