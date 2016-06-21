#import  "MapView.h"
#import  "NTBaseMapView.h"
#import  "ui/MapRedrawRequestListener.h"
#import  "ui/BaseMapView.h"
#import  "ui/MapLicenseManagerListener.h"
#include "utils/Const.h"
#include "utils/IOSUtils.h"
#include "utils/Log.h"

#include <memory>

#import  <UIKit/UIKit.h>

static BOOL MapViewCreated = NO;

@interface NTMapView() <GLKViewDelegate> { }

@property (strong, nonatomic) NTBaseMapView* baseMapView;
@property (assign, nonatomic) BOOL contextCreated;
@property (assign, nonatomic) float scale;

@property (strong, nonatomic) UITouch* pointer1;
@property (strong, nonatomic) UITouch* pointer2;

@end

static const int NATIVE_ACTION_POINTER_1_DOWN = 0;
static const int NATIVE_ACTION_POINTER_2_DOWN = 1;
static const int NATIVE_ACTION_MOVE = 2;
static const int NATIVE_ACTION_CANCEL = 3;
static const int NATIVE_ACTION_POINTER_1_UP = 4;
static const int NATIVE_ACTION_POINTER_2_UP = 5;
static const int NATIVE_NO_COORDINATE = -1;

@implementation NTMapView

+(void) initialize {
    if (self == [NTMapView class]) {
        carto::IOSUtils::InitializeLog();
    }
}

-(id)init {
    self = [super init];
    [self initBase];
    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    [self initBase];
    return self;
}

-(id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    [self initBase];
    return self;
}

-(void)initBase {
    self.delegate = self;

    _scale = 1;
    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
        _scale = [[UIScreen mainScreen] scale];
    }
    _baseMapView = [[NTBaseMapView alloc] init];
    _nativeMapView = [_baseMapView getCptr];

    NTMapRedrawRequestListener* redrawRequestListener = [[NTMapRedrawRequestListener alloc] initWithView:self];
    [_baseMapView setRedrawRequestListener:redrawRequestListener];
    
    [[_baseMapView getOptions] setDPI:carto::Const::UNSCALED_DPI * _scale];
    dispatch_async(dispatch_get_main_queue(), ^{
        [self initGL];
    });
}

-(void)initGL {
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (!self.context) {
        carto::Log::Fatal("MapView::viewDidLoad: Failed to create OpenGL ES 2.0 context");
    }
    _contextCreated = YES;

    self.context = self.context;
    self.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    self.drawableMultisample = GLKViewDrawableMultisampleNone;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.multipleTouchEnabled = YES;
    
    [EAGLContext setCurrentContext:self.context];
    [_baseMapView onSurfaceCreated];
    [_baseMapView onSurfaceChanged:(int)(self.bounds.size.width * _scale) height:(int)(self.bounds.size.height * _scale)];
    
    [self setNeedsDisplay];
}

-(void)layoutSubviews {
    [super layoutSubviews];
    if (_contextCreated) {
        [EAGLContext setCurrentContext:self.context];
        [_baseMapView onSurfaceChanged:(int)(self.bounds.size.width * _scale) height:(int)(self.bounds.size.height * _scale)];
        [self setNeedsDisplay];
    }
}

-(void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    if (_contextCreated) {
        [EAGLContext setCurrentContext:self.context];
        [_baseMapView onDrawFrame];
    }
}

-(void)dealloc {
    if (_contextCreated && [EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
        [_baseMapView setRedrawRequestListener:nil];
        _nativeMapView = nil;
        _baseMapView = nil;
        //self.context = nil;
    }
}

-(void)transformScreenCoord: (CGPoint*)screenCoord {
    screenCoord->x *= _scale;
    screenCoord->y *= _scale;
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch* pointer in [touches allObjects]) {
        if (!_pointer1) {
            _pointer1 = pointer;
            CGPoint screenPos = [_pointer1 locationInView:self];
            [self transformScreenCoord:&screenPos];
            [_baseMapView onInputEvent:NATIVE_ACTION_POINTER_1_DOWN x1:screenPos.x y1:screenPos.y x2:NATIVE_NO_COORDINATE y2:NATIVE_NO_COORDINATE];
            continue;
        }
        
        if (!_pointer2) {
            _pointer2 = pointer;
            CGPoint screenPos1 = [_pointer1 locationInView:self];
            CGPoint screenPos2 = [_pointer2 locationInView:self];
            [self transformScreenCoord:&screenPos1];
            [self transformScreenCoord:&screenPos2];
            [_baseMapView onInputEvent:NATIVE_ACTION_POINTER_2_DOWN x1:screenPos1.x y1:screenPos1.y x2:screenPos2.x y2:screenPos2.y];
            break;
        }
    }
}

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    if (_pointer1) {
        CGPoint screenPos1 = [_pointer1 locationInView:self];
        [self transformScreenCoord:&screenPos1];
        if (_pointer2) {
            CGPoint screenPos2 = [_pointer2 locationInView:self];
            [self transformScreenCoord:&screenPos2];
            [_baseMapView onInputEvent:NATIVE_ACTION_MOVE x1:screenPos1.x y1:screenPos1.y x2:screenPos2.x y2:screenPos2.y];
        } else {
            [_baseMapView onInputEvent:NATIVE_ACTION_MOVE x1:screenPos1.x y1:screenPos1.y x2:NATIVE_NO_COORDINATE y2:NATIVE_NO_COORDINATE];
        }
    }
}

-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    [_baseMapView onInputEvent:NATIVE_ACTION_CANCEL x1:NATIVE_NO_COORDINATE y1:NATIVE_NO_COORDINATE x2:NATIVE_NO_COORDINATE y2:NATIVE_NO_COORDINATE];
    _pointer1 = nil;
    _pointer2 = nil;
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    if (_pointer2 && [touches containsObject:_pointer2]) {
        // Dual pointer, second pointer goes up first
        CGPoint screenPos1 = [_pointer1 locationInView:self];
        CGPoint screenPos2 = [_pointer2 locationInView:self];
        [self transformScreenCoord:&screenPos1];
        [self transformScreenCoord:&screenPos2];
        [_baseMapView onInputEvent:NATIVE_ACTION_POINTER_2_UP x1:screenPos1.x y1:screenPos1.y x2:screenPos2.x y2:screenPos2.y];
        _pointer2 = nil;
    }
    
    if (_pointer1 && [touches containsObject:_pointer1]) {
        // Single pointer, pointer goes up
        CGPoint screenPos1 = [_pointer1 locationInView:self];
        [self transformScreenCoord:&screenPos1];
        if (_pointer2) {
            CGPoint screenPos2 = [_pointer2 locationInView:self];
            [self transformScreenCoord:&screenPos2];
            [_baseMapView onInputEvent:NATIVE_ACTION_POINTER_1_UP x1:screenPos1.x y1:screenPos1.y x2:screenPos2.x y2:screenPos2.y];
            _pointer1 = _pointer2;
            _pointer2 = nil;
        } else {
            [_baseMapView onInputEvent:NATIVE_ACTION_POINTER_1_UP x1:screenPos1.x y1:screenPos1.y x2:NATIVE_NO_COORDINATE y2:NATIVE_NO_COORDINATE];
            _pointer1 = nil;
        }
    }
}

-(NTLayers*)getLayers {
    return [_baseMapView getLayers];
}

+(BOOL)registerLicense: (NSString*)licenseKey {
    @synchronized (self) {
        if (MapViewCreated) {
            carto::Log::Error("NTMapView.registerLicense: The method [NTMapView registerLicense:] must be called before the MapView is created. Either in the static initializer or before [super viewDidLoad] method.");
            return NO;
        }
        NSString* newLicenseKey = nil;
        NSString* oldKey = @"CARTO_MOBILE_SDKLICENSE_KEY_OLD";
        NSString* newKey = @"CARTO_MOBILE_SDKLICENSE_KEY_NEW";
        @try {
            NSString* oldLicenseKey = [[NSUserDefaults standardUserDefaults] objectForKey:oldKey];
            if (oldLicenseKey && [oldLicenseKey isEqualToString:licenseKey]) {
                newLicenseKey = [[NSUserDefaults standardUserDefaults] objectForKey:newKey];
            }
        }
        @catch (NSException* e) {
            carto::Log::Warn("NTMapView.registerLicense: Failed to read updated license key");
        }
        NTLicenseManagerListener* listener = [[NTMapLicenseManagerListener alloc] initWithLicenseKey:licenseKey defaultsKeyOld:oldKey defaultsKeyNew:newKey];
        return [NTBaseMapView registerLicense:(newLicenseKey ? newLicenseKey : licenseKey) listener:listener];
    }
}

-(NTOptions*)getOptions
{
    return [_baseMapView getOptions];
}

-(NTMapRenderer*)getMapRenderer
{
    return [_baseMapView getMapRenderer];
}

-(NTMapPos*)getFocusPos
{
    return [_baseMapView getFocusPos];
}

-(float)getRotation
{
    return [_baseMapView getRotation];
}

-(float)getTilt
{
    return [_baseMapView getTilt];
}

-(float)getZoom
{
    return [_baseMapView getZoom];
}

-(void)pan: (NTMapVec*)deltaPos durationSeconds: (float)durationSeconds
{
    [_baseMapView pan:deltaPos durationSeconds:durationSeconds];
}

-(void)setFocusPos: (NTMapPos*)pos durationSeconds: (float)durationSeconds
{
    [_baseMapView setFocusPos:pos durationSeconds:durationSeconds];
}

-(void)rotate: (float)deltaAngle durationSeconds: (float)durationSeconds
{
    [_baseMapView rotate:deltaAngle durationSeconds:durationSeconds];
}

-(void)rotate: (float)deltaAngle targetPos: (NTMapPos*)targetPos durationSeconds: (float)durationSeconds
{
    [_baseMapView rotate:deltaAngle targetPos:targetPos durationSeconds:durationSeconds];
}

-(void)setRotation: (float)angle durationSeconds: (float)durationSeconds
{
    [_baseMapView setRotation:angle durationSeconds:durationSeconds];
}

-(void)setRotation: (float)angle targetPos: (NTMapPos*)targetPos durationSeconds: (float)durationSeconds
{
    [_baseMapView setRotation:angle targetPos:targetPos durationSeconds:durationSeconds];
}

-(void)tilt: (float)deltaTilt durationSeconds: (float)durationSeconds
{
    [_baseMapView tilt:deltaTilt durationSeconds:durationSeconds];
}

-(void)setTilt: (float)tilt durationSeconds: (float)durationSeconds
{
    [_baseMapView setTilt:tilt durationSeconds:durationSeconds];
}

-(void)zoom: (float)deltaZoom durationSeconds: (float)durationSeconds
{
    [_baseMapView zoom:deltaZoom durationSeconds:durationSeconds];
}

-(void)zoom: (float)deltaZoom targetPos: (NTMapPos*)targetPos durationSeconds: (float)durationSeconds
{
    [_baseMapView zoom:deltaZoom targetPos:targetPos durationSeconds:durationSeconds];
}

-(void)setZoom: (float)zoom durationSeconds: (float)durationSeconds
{
    [_baseMapView setZoom:zoom durationSeconds:durationSeconds];
}

-(void)setZoom: (float)zoom targetPos: (NTMapPos*)targetPos durationSeconds: (float)durationSeconds
{
    [_baseMapView setZoom:zoom targetPos:targetPos durationSeconds:durationSeconds];
}

-(void)moveToFitBounds: (NTMapBounds*)mapBounds screenBounds: (NTScreenBounds*)screenBounds integerZoom: (BOOL)integerZoom durationSeconds: (float)durationSeconds
{
    [_baseMapView moveToFitBounds:mapBounds screenBounds:screenBounds integerZoom:integerZoom durationSeconds:durationSeconds];
}

-(void)moveToFitBounds: (NTMapBounds*)mapBounds screenBounds: (NTScreenBounds*)screenBounds integerZoom: (BOOL)integerZoom resetRotation: (BOOL)resetRotation resetTilt: (BOOL)resetTilt durationSeconds: (float)durationSeconds
{
    [_baseMapView moveToFitBounds:mapBounds screenBounds:screenBounds integerZoom:integerZoom resetRotation:resetRotation resetTilt:resetTilt durationSeconds:durationSeconds];
}

-(NTMapEventListener*) getMapEventListener
{
    return [_baseMapView getMapEventListener];
}

-(void)setMapEventListener: (NTMapEventListener*)mapEventListener
{
    [_baseMapView setMapEventListener:mapEventListener];
}

-(NTMapPos*)screenToMap: (NTScreenPos*)screenPos
{
    return [_baseMapView screenToMap:screenPos];
}

-(NTScreenPos*)mapToScreen: (NTMapPos*)mapPos
{
    return [_baseMapView mapToScreen:mapPos];
}

-(void)cancelAllTasks {
    [_baseMapView cancelAllTasks];
}

-(void)clearPreloadingCaches {
    [_baseMapView clearPreloadingCaches];
}

-(void)clearAllCaches {
    [_baseMapView clearAllCaches];
}

@end
