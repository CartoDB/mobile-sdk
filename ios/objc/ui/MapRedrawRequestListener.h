/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#import "NTRedrawRequestListener.h"

@class GLKView;

@interface NTMapRedrawRequestListener : NTRedrawRequestListener

@property (weak, nonatomic) GLKView* view;

-(id)initWithView:(GLKView*)view;

@end
