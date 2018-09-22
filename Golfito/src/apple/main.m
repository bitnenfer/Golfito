#if defined(TARGET_IOS) || defined(TARGET_TVOS)
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#else
#import <Cocoa/Cocoa.h>
#endif

int main(int argc, const char* argv[]) {
#if defined(TARGET_IOS) || defined(TARGET_TVOS)
    @autoreleasepool {
        return UIApplicationMain(argc, (char**)argv, nil, NSStringFromClass([AppDelegate class]));
    }
#else
    return NSApplicationMain(argc, argv);
#endif
}
