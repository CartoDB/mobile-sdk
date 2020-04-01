/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#import "NTRedrawRequestListener.h"
#import "ui/MapView.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@interface NTMapRedrawRequestListener : NTRedrawRequestListener

@property (weak, nonatomic) NTGLKView* view;

-(id)initWithView:(NTGLKView*)view;

@end

#pragma clang diagnostic pop
