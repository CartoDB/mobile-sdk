/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#import <Foundation/Foundation.h>

/**
 * NTExceptionWrapper is an utility class for handling ObjectiveC exceptions in Swift.
 */
__attribute__ ((visibility("default"))) @interface NTExceptionWrapper : NSObject

/**
 * Catches an ObjectiveC exception and stores the exception info in NSError object.
 * @param tryBlock The block to execute
 * @param error Pointer to the NSError object used to store the exception info, if exception occured. 
                'ExceptionName' and 'ExceptionReason' keys in error userInfo can be used to retrieve details of the exception.
 */
+(BOOL)catchException:(void(^)(void))tryBlock error:(__autoreleasing NSError **)error;

@end
