
#if defined(TARGET_IOS) || defined(TARGET_TVOS)
#import <UIKit/UIKit.h>
#define NativeViewController UIViewController
#else
#import <Cocoa/Cocoa.h>
#define NativeViewController NSViewController
#endif

@import MetalKit;

@interface ViewController : NativeViewController


@end

