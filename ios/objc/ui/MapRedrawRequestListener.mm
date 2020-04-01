#import "MapRedrawRequestListener.h"

@implementation NTMapRedrawRequestListener

-(id)initWithView:(NTGLKView*)view {
    self = [super init];
    _view = view;
    return self;
}

-(void)onRedrawRequested {
    dispatch_async(dispatch_get_main_queue(), ^{
        NTGLKView* view = _view;
        if (view) {
            [view setNeedsDisplay];
        }
    });
}

@end
