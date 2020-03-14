#import "ExceptionWrapper.h"

@implementation NTExceptionWrapper

+(BOOL)catchException:(void(^)(void))tryBlock error:(__autoreleasing NSError**)error {
    @try {
        tryBlock();
        return YES;
    }
    @catch (NSException* exception) {
        NSMutableDictionary* info = [[NSMutableDictionary alloc] init];
        [info setValue:exception.name forKey:@"ExceptionName"];
        [info setValue:exception.reason forKey:@"ExceptionReason"];
        [info setValue:exception.callStackReturnAddresses forKey:@"ExceptionCallStackReturnAddresses"];
        [info setValue:exception.callStackSymbols forKey:@"ExceptionCallStackSymbols"];
        [info setValue:exception.userInfo forKey:@"ExceptionUserInfo"];
        *error = [[NSError alloc] initWithDomain:exception.name code:0 userInfo:info];
        return NO;
    }
}

@end
