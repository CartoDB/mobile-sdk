#import "MapRedrawRequestListener.h"

#import <GLKit/GLKit.h>

@implementation NTMapRedrawRequestListener

-(id)initWithView:(GLKView*)view {
    self = [super init];
    _view = view;
    return self;
}

-(void)onRedrawRequested {
    dispatch_async(dispatch_get_main_queue(), ^{
        GLKView* view = _view;
        if (view) {
            [view setNeedsDisplay];
        }
    });
}

@end
