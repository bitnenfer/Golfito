#import "AppDelegate.h"
#include "../core/stb_image.h"
#include "../core/input.h"
#include "../core/types.h"

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
extern void _input_update_down (uint32_t pointerID, float32_t x, float32_t y);
extern void _input_update_up (uint32_t pointerID, float32_t x, float32_t y);
extern void _input_update_move (uint32_t pointerID, float32_t x, float32_t y);
#endif

@interface AppDelegate ()

@end

@implementation AppDelegate

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
// ============================================
// Input Handling
// ============================================
- (void)touchesBegan:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
    [super touchesBegan:touches withEvent:event];
    uint32_t index = 0;
    for (UITouch* touch in touches) {
        CGPoint point = [touch locationInView:nil];
        _input_update_down(index++, point.x, point.y);
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
    [super touchesMoved:touches withEvent:event];
    uint32_t index = 0;
    for (UITouch* touch in touches) {
        CGPoint point = [touch locationInView:nil];
        _input_update_move(index++, point.x, point.y);
    }

}

- (void)touchesEnded:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
    [super touchesEnded:touches withEvent:event];
    uint32_t index = 0;
    for (UITouch* touch in touches) {
        CGPoint point = [touch locationInView:nil];
        _input_update_up(index++, point.x, point.y);
    }
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
    [super touchesCancelled:touches withEvent:event];
    uint32_t index = 0;
    for (UITouch* touch in touches) {
        CGPoint point = [touch locationInView:nil];
        _input_update_up(index++, point.x, point.y);
    }
}
// ============================================

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    input_initialize();
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    [self onTerminateApplication];
}
#else
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    input_initialize();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    [self onTerminateApplication];
}
#endif

// Common code

- (void)onTerminateApplication {
    NSLog(@"Terminate application");
}

@end
