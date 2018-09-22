#import "ViewController.h"
#include "../core/math.h"
#include "../core/gfx.h"
#include "../core/input.h"
#include "../game/boot.h"

#if defined(TARGET_MACOS)
extern void _input_update_down (uint32_t pointerID, float32_t x, float32_t y);
extern void _input_update_up (uint32_t pointerID, float32_t x, float32_t y);
extern void _input_update_move (uint32_t pointerID, float32_t x, float32_t y);
#endif

#if defined(TARGET_TVOS)
#import <GameController/GameController.h>



#endif

extern void _gfx_init_state (MTKView* pView, float32_t width, float32_t height);

@interface MetalViewDelegate : NSObject<MTKViewDelegate>
- (MetalViewDelegate*)initWithMetalKitView:(MTKView*)view;
@end

@implementation MetalViewDelegate
{
    MTKView* _view;
    vec2_t _viewportSize;
}
- (MetalViewDelegate*)initWithMetalKitView:(MTKView *)view {
    _view = view;
    _viewportSize.x = _view.frame.size.width;
    _viewportSize.y = _view.frame.size.height;
    _gfx_init_state(view, _viewportSize.x, _viewportSize.y);
    gfx_initialize();
    game_start();
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    gfx_begin();
    game_loop(0.16f);
    gfx_end();
}


- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
    gfx_resize(_viewportSize.x, _viewportSize.y);
}

@end

@interface ViewController ()
@end

@implementation ViewController
{
    MTKView* _metalKitView;
    MetalViewDelegate* _metalViewDelegate;
}
- (void)viewDidLoad {
    [super viewDidLoad];
    _metalKitView = (MTKView*)self.view;
    _metalKitView.device = MTLCreateSystemDefaultDevice();
    if (!_metalKitView.device) {
        NSLog(@"Failed to create default metal device");
        exit(1);
        return;
    }
    _metalViewDelegate = [[MetalViewDelegate alloc] initWithMetalKitView:_metalKitView];
    _metalKitView.delegate = _metalViewDelegate;
#if defined(TARGET_MACOS)
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                                                  owner:self
                                                               userInfo:nil];
    [self.view addTrackingArea:trackingArea];
#endif
}

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
#if defined(TARGET_IOS)
- (UIStatusBarStyle)preferredStatusBarStyle {
    return UIStatusBarStyleLightContent;
}
- (BOOL)prefersStatusBarHidden {
    return TRUE;
}
#elif defined(TARGET_TVOS)
//- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
//    NSLog(@"FOO");
//}
#endif
#else
- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}
// ============================================
// Input Handling
// ============================================
- (void)mouseDown:(NSEvent *)event {
    NSPoint point = event.locationInWindow;
    point = [self.view convertPoint:point fromView:nil];
    point = NSMakePoint(point.x, self.view.bounds.size.height - point.y);
    _input_update_down(0, point.x, point.y);
}
- (void)mouseUp:(NSEvent *)event {
    NSPoint point = event.locationInWindow;
    point = [self.view convertPoint:point fromView:nil];
    point = NSMakePoint(point.x, self.view.bounds.size.height - point.y);
    _input_update_up(0, point.x, point.y);
}
- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = event.locationInWindow;
    point = [self.view convertPoint:point fromView:nil];
    point = NSMakePoint(point.x, self.view.bounds.size.height - point.y);
    _input_update_move(0, point.x, point.y);
}
- (void)mouseDragged:(NSEvent *)event {
    NSPoint point = event.locationInWindow;
    point = [self.view convertPoint:point fromView:nil];
    point = NSMakePoint(point.x, self.view.bounds.size.height - point.y);
    _input_update_move(0, point.x, point.y);
}
// ============================================
#endif

@end
