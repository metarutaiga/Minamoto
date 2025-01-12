// dear imgui: standalone example application
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "DearImGui.h"
#include "Logger.h"
#include "Module.h"
#include "Renderer.h"

#include <stdio.h>
#include <Runtime/Runtime.h>

#if defined(xxMACOS)
#import <Cocoa/Cocoa.h>
#elif defined(xxIOS)
#import <UIKit/UIKit.h>
#endif

//-----------------------------------------------------------------------------------
// ImGuiExampleView
//-----------------------------------------------------------------------------------

#if defined(xxMACOS)
@interface ImGuiExampleView : NSView
@property (nonatomic) Boolean imguiUpdate;
@property (nonatomic) NSTimer* animationTimer;
@property (nonatomic) id localMonitor;
@end
#elif defined(xxIOS)
@interface ImGuiExampleView : UIView
@property (nonatomic) Boolean imguiUpdate;
@property (nonatomic) Boolean resetSize;
@end
#endif

@implementation ImGuiExampleView

#if defined(xxMACOS)
-(instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];

    self.imguiUpdate = YES;
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:0 target:self selector:@selector(updateAndDraw) userInfo:nil repeats:YES];
    self.localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskAny handler:^NSEvent * _Nullable(NSEvent *event)
    {
        self.imguiUpdate = YES;
        return event;
    }];

    [self setWantsLayer:YES];
    [self setPostsFrameChangedNotifications:YES];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reset) name:NSViewFrameDidChangeNotification object:self];

    return self;
}

-(BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
    [super performKeyEquivalent:theEvent];
    return YES;
}
#elif defined(xxIOS)
+(Class)layerClass
{
    return [CAMetalLayer class];
}

-(id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];

    self.imguiUpdate = YES;

    CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(updateAndDraw)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    return self;
}
#endif

-(void)updateAndDraw
{
    @autoreleasepool
    {
#if defined(xxIOS)
        if (_resetSize)
        {
            _resetSize = NO;
            [self reset];
        }
#endif
        DearImGui::NewFrame((__bridge void*)self);
        self.imguiUpdate |= Module::Update();
        self.imguiUpdate |= DearImGui::Update(Module::Count() == 0);

        if (self.imguiUpdate)
        {
            uint64_t commandEncoder = Renderer::Begin();
            if (commandEncoder)
            {
                Module::Render();
                DearImGui::Render(commandEncoder);
                Renderer::End();
                Renderer::Present();
            }
        }

        DearImGui::PostUpdate((__bridge void*)self, self.imguiUpdate);

        if (DearImGui::PowerSaving())
            xxSleep(1000 / 60);

        self.imguiUpdate = NO;
    }
}

-(void)reset
{
#if defined(xxMACOS)
    NSSize size = [[[self window] contentView] frame].size;
#elif defined(xxIOS)
    CGSize size = [[self window] bounds].size;
#endif

    Renderer::Reset((__bridge void*)[self window], size.width, size.height);
}
@end

//-----------------------------------------------------------------------------------
// ImGuiExampleViewController
//-----------------------------------------------------------------------------------

#if defined(xxMACOS)
@interface ImGuiExampleViewController : NSViewController
@end
#elif defined(xxIOS)
@interface ImGuiExampleViewController : UIViewController
@end
#endif

@implementation ImGuiExampleViewController

#if TARGET_OS_OSX

-(instancetype)initWithView:(NSView*)view
{
    self.view = view;
    self = [super init];
    return self;
}

#elif TARGET_OS_IOS

-(instancetype)initWithView:(UIView*)view
{
    self.view = view;
    self = [super init];
    return self;
}

-(void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id <UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    ImGuiExampleView* view = (ImGuiExampleView*)self.view;
    view.resetSize = YES;
}

// This touch mapping is super cheesy/hacky. We treat any touch on the screen
// as if it were a depressed left mouse button, and we don't bother handling
// multitouch correctly at all. This causes the "cursor" to behave very erratically
// when there are multiple active touches. But for demo purposes, single-touch
// interaction actually works surprisingly well.
-(void)updateIOWithTouchEvent:(UIEvent *)event
{
    UITouch* anyTouch = event.allTouches.anyObject;
    CGPoint touchLocation = [anyTouch locationInView:self.view];
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(touchLocation.x, touchLocation.y);

    BOOL hasActiveTouch = NO;
    for (UITouch* touch in event.allTouches)
    {
        if (touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled)
        {
            hasActiveTouch = YES;
            break;
        }
    }
    io.MouseDown[0] = hasActiveTouch;
}

-(void)updateIOWithPresses:(NSSet<UIPress *> *)presses
{
    ImGuiIO& io = ImGui::GetIO();

    // TODO
    io.ConfigMacOSXBehaviors = false;

    auto HIDUsageToKey = [](UIKeyboardHIDUsage usage) -> ImGuiKey
    {
        switch (usage)
        {
        case UIKeyboardHIDUsageKeyboardA ... UIKeyboardHIDUsageKeyboardZ:   return ImGuiKey(ImGuiKey_A + usage - UIKeyboardHIDUsageKeyboardA);
        case UIKeyboardHIDUsageKeyboard0:                                   return ImGuiKey_0;
        case UIKeyboardHIDUsageKeyboard1 ... UIKeyboardHIDUsageKeyboard9:   return ImGuiKey(ImGuiKey_1 + usage - UIKeyboardHIDUsageKeyboard1);

        case UIKeyboardHIDUsageKeyboardReturnOrEnter:                       return ImGuiKey_Enter;
        case UIKeyboardHIDUsageKeyboardEscape:                              return ImGuiKey_Escape;
        case UIKeyboardHIDUsageKeyboardDeleteOrBackspace:                   return ImGuiKey_Backspace;
        case UIKeyboardHIDUsageKeyboardTab:                                 return ImGuiKey_Tab;
        case UIKeyboardHIDUsageKeyboardSpacebar:                            return ImGuiKey_Space;
        case UIKeyboardHIDUsageKeyboardHyphen:                              return ImGuiKey_Minus;
        case UIKeyboardHIDUsageKeyboardEqualSign:                           return ImGuiKey_Equal;
        case UIKeyboardHIDUsageKeyboardOpenBracket:                         return ImGuiKey_LeftBracket;
        case UIKeyboardHIDUsageKeyboardCloseBracket:                        return ImGuiKey_RightBracket;
        case UIKeyboardHIDUsageKeyboardBackslash:                           return ImGuiKey_Backslash;
        case UIKeyboardHIDUsageKeyboardSemicolon:                           return ImGuiKey_Semicolon;
        case UIKeyboardHIDUsageKeyboardQuote:                               return ImGuiKey_Apostrophe;
        case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde:                 return ImGuiKey_CapsLock;
        case UIKeyboardHIDUsageKeyboardComma:                               return ImGuiKey_Comma;
        case UIKeyboardHIDUsageKeyboardPeriod:                              return ImGuiKey_Period;
        case UIKeyboardHIDUsageKeyboardSlash:                               return ImGuiKey_Slash;
        case UIKeyboardHIDUsageKeyboardCapsLock:                            return ImGuiKey_CapsLock;

        case UIKeyboardHIDUsageKeyboardF1 ... UIKeyboardHIDUsageKeyboardF12:return ImGuiKey(ImGuiKey_F1 + usage - UIKeyboardHIDUsageKeyboardF1);

        case UIKeyboardHIDUsageKeyboardLeftShift:                           return ImGuiKey_LeftShift;
        case UIKeyboardHIDUsageKeyboardRightShift:                          return ImGuiKey_RightShift;
        case UIKeyboardHIDUsageKeyboardLeftControl:                         return ImGuiKey_LeftCtrl;
        case UIKeyboardHIDUsageKeyboardRightControl:                        return ImGuiKey_RightCtrl;
        case UIKeyboardHIDUsageKeyboardLeftAlt:                             return ImGuiKey_LeftAlt;
        case UIKeyboardHIDUsageKeyboardRightAlt:                            return ImGuiKey_RightAlt;

        default:
            return ImGuiKey_None;
        }
    };

    for (UIPress* press : presses)
    {
        if (press.key == nil)
            continue;

        switch (press.phase)
        {
        case UIPressPhaseBegan:
        case UIPressPhaseChanged:
        case UIPressPhaseStationary:
            switch (press.key.keyCode)
            {
            case UIKeyboardHIDUsageKeyboardA ... UIKeyboardHIDUsageKeyboard0:
            case UIKeyboardHIDUsageKeyboardSpacebar:
            case UIKeyboardHIDUsageKeyboardHyphen ... UIKeyboardHIDUsageKeyboardSlash:
                io.AddInputCharactersUTF8(press.key.characters.UTF8String);
                break;
            case UIKeyboardHIDUsageKeyboardLeftControl:
            case UIKeyboardHIDUsageKeyboardRightControl:
                io.KeyCtrl = true;
                break;
            case UIKeyboardHIDUsageKeyboardLeftShift:
            case UIKeyboardHIDUsageKeyboardRightShift:
                io.KeyShift = true;
                break;
            case UIKeyboardHIDUsageKeyboardLeftAlt:
            case UIKeyboardHIDUsageKeyboardRightAlt:
                io.KeyAlt = true;
                break;
#if 0
            case UIKeyboardHIDUsageKeyboardLeftGUI:
            case UIKeyboardHIDUsageKeyboardRightGUI:
                io.KeySuper = true;
                break;
#endif
            default:
                break;
            }
            io.AddKeyEvent(HIDUsageToKey(press.key.keyCode), true);
            break;
        case UIPressPhaseEnded:
        case UIPressPhaseCancelled:
        default:
            switch (press.key.keyCode)
            {
            case UIKeyboardHIDUsageKeyboardLeftControl:
            case UIKeyboardHIDUsageKeyboardRightControl:
                io.KeyCtrl = false;
                break;
            case UIKeyboardHIDUsageKeyboardLeftShift:
            case UIKeyboardHIDUsageKeyboardRightShift:
                io.KeyShift = false;
                break;
            case UIKeyboardHIDUsageKeyboardLeftAlt:
            case UIKeyboardHIDUsageKeyboardRightAlt:
                io.KeyAlt = false;
                break;
#if 0
            case UIKeyboardHIDUsageKeyboardLeftGUI:
            case UIKeyboardHIDUsageKeyboardRightGUI:
                io.KeySuper = false;
                break;
#endif
            default:
                break;
            }
            io.AddKeyEvent(HIDUsageToKey(press.key.keyCode), false);
            break;
        }
    }
}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event      { [self updateIOWithTouchEvent:event];  }
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event      { [self updateIOWithTouchEvent:event];  }
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event      { [self updateIOWithTouchEvent:event];  }
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event  { [self updateIOWithTouchEvent:event];  }

-(void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event      { [self updateIOWithPresses:presses];   }
-(void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event    { [self updateIOWithPresses:presses];   }
-(void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event      { [self updateIOWithPresses:presses];   }
-(void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event  { [self updateIOWithPresses:presses];   }

#endif

@end

//-----------------------------------------------------------------------------------
// ImGuiExampleAppDelegate
//-----------------------------------------------------------------------------------

#if defined(xxMACOS)
@interface ImGuiExampleAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (nonatomic, readonly) NSWindow* window;
@end
#elif defined(xxIOS)
@interface ImGuiExampleAppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow* window;
@end
#endif

@implementation ImGuiExampleAppDelegate
@synthesize window = _window;

#if defined(xxMACOS)
-(void)setupMenu
{
    NSMenu* mainMenuBar = [NSMenu new];
    NSMenu* appMenu;
    NSMenuItem* menuItem;

    appMenu = [[NSMenu alloc] initWithTitle:@(Runtime::Version)];
    menuItem = [appMenu addItemWithTitle:@"Quit"
                                  action:@selector(terminate:)
                           keyEquivalent:@"q"];
    [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];

    menuItem = [NSMenuItem new];
    [menuItem setSubmenu:appMenu];

    [mainMenuBar addItem:menuItem];

    appMenu = nil;
    [NSApp setMainMenu:mainMenuBar];
}

-(NSWindow*)window
{
    if (_window != nil)
        return (_window);

    NSRect viewRect = NSMakeRect(100.0, 100.0, 100.0 + 1280.0, 100 + 720.0);

    _window = [[NSWindow alloc] initWithContentRect:viewRect
                                          styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable
                                            backing:NSBackingStoreBuffered
                                              defer:YES];
    [_window setDelegate:self];
    [_window setOpaque:YES];
    [_window setTitle:@(Runtime::Version)];
    [_window makeKeyAndOrderFront:nil];
    [_window zoom:nil];

    return (_window);
}

-(void)windowWillClose:(NSNotification *)notification
{
    [NSApp terminate:self];
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [self initialize];
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    [self shutdown];
}
#elif defined(xxIOS)
-(UIWindow*)window
{
    if (_window != nil)
        return (_window);

    _window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
#if defined(xxMACCATALYST)
    UITitlebar* titlebar = _window.windowScene.titlebar;
    if (titlebar)
    {
        titlebar.titleVisibility = UITitlebarTitleVisibilityHidden;
        titlebar.toolbar = nil;
    }
#endif

    return (_window);
}

-(void)applicationDidFinishLaunching:(UIApplication *)application;
{
    [self initialize];
}

-(void)applicationWillTerminate:(UIApplication *)application;
{
    [self shutdown];
}
#endif

-(void)initialize
{
#if defined(xxMACOS)
    float scale = [self.window backingScaleFactor];

    // Make the application a foreground application (else it won't receive keyboard events)
    ProcessSerialNumber psn = {0, kCurrentProcess};
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

    // Menu
    [self setupMenu];
#elif defined(xxIOS)
    float scale = [[UIScreen mainScreen] nativeScale];
#endif

    ImGuiExampleView* view = [[ImGuiExampleView alloc] initWithFrame:self.window.frame];
#if defined(xxMACOS)
    self.window.contentViewController = [[ImGuiExampleViewController alloc] initWithView:view];
    [self.window makeKeyAndOrderFront:NSApp];
    NSSize size = [view frame].size;
#elif defined(xxIOS)
    self.window.rootViewController = [[ImGuiExampleViewController alloc] initWithView:view];
    [self.window makeKeyAndVisible];
    CGSize size = [view bounds].size;
#endif

    Logger::Create();
    Renderer::Create((__bridge void*)self.window, size.width, size.height);
    DearImGui::Create((__bridge void*)view, 1.0f, scale);
    Module::Create("module", Renderer::g_device);
    Module::Message({ "INIT" });
}

-(void)shutdown
{
    Module::Message({ "SHUTDOWN" });
    Module::Shutdown();
    DearImGui::Shutdown();
    Renderer::Shutdown();
    Logger::Shutdown();
}
@end

int main(int argc, char* argv[])
{
#if defined(xxMACOS)
    chdir(xxGetExecutablePath());
    @autoreleasepool
    {
        NSApp = [NSApplication sharedApplication];
        ImGuiExampleAppDelegate* delegate = [ImGuiExampleAppDelegate new];
        [[NSApplication sharedApplication] setDelegate:delegate];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
        return NSApplicationMain(argc, (char const**)argv);
    }
#elif defined(xxIOS)
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([ImGuiExampleAppDelegate class]));
    }
#endif
}
