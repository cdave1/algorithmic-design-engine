/*
 * Copyright (c) 2016 David Petrie david@davidpetrie.com
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software. Permission is granted to anyone to use this software for
 * any purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product, an
 * acknowledgment in the product documentation would be appreciated but is not
 * required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#import "AppDelegate.h"
#include <algorithm>
#include "OpenGLShim.h"
#include "LogoGenerator.h"
#include "ScreenShotr.h"

@interface FullScreenWindow : NSWindow
- (BOOL)canBecomeKeyWindow;
@end


@implementation FullScreenWindow
- (BOOL)canBecomeKeyWindow {
    return YES;
}
@end


@implementation AppDelegate

@synthesize window, glView;


static LogoGenerator *logoGenerator;
static ScreenShotr *screenShotr = nullptr;
static GLuint shaderProgram;
static bool isSetup = false;
static bool isRendered = false;
static int framesUntilScreenshot = 0;
static NSRect mainDisplayRect;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSRect dispRect = [[NSScreen mainScreen] frame];
    mainDisplayRect = [[NSScreen mainScreen] convertRectToBacking:[[NSScreen mainScreen] frame]];

    self.window = [[FullScreenWindow alloc] initWithContentRect:mainDisplayRect
                                                      styleMask:NSBorderlessWindowMask
                                                        backing:NSBackingStoreBuffered
                                                          defer:YES];
    [self.window setLevel:NSMainMenuWindowLevel+1];
    [self.window setOpaque:YES];
    [self.window setHidesOnDeactivate:YES];

    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAllowOfflineRenderers,
        NSOpenGLPFAMultisample,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
        NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)8,
        0
    };

    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    self.glView = [[AppView alloc] initWithFrame:mainDisplayRect pixelFormat: pixelFormat];
    [self.window setContentView: self.glView];
    [self.glView convertRectToBacking:mainDisplayRect];
    [self.glView setWantsBestResolutionOpenGLSurface:YES];
    [self.glView setRenderProxy:self];
    [self.window makeKeyAndOrderFront:self];
    [self.window makeFirstResponder:self.glView];
}


- (void)applicationDidBecomeActive:(NSNotification *)aNotification {
    if (!isSetup) {
        NSArray *args = [[NSProcessInfo processInfo] arguments];

        if ([args count] <= 1) {
            fprintf(stderr, "Need at least one argument.\n");
            exit(0);
        }

        for (NSString *arg in args) {
            NSLog(@"%@", arg);
        }

        NSString *nsFontText = [args objectAtIndex:1];
        std::string fontText = std::string([nsFontText UTF8String]);

        NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Diavlo_BLACK_II_37.otf"];
        logoGenerator = new LogoGenerator(mainDisplayRect.size.width, mainDisplayRect.size.height);
        logoGenerator->SetConditions(300.0, fontText, [path UTF8String]);

        screenShotr = new ScreenShotr(mainDisplayRect.size.width, mainDisplayRect.size.height);

        NSString *fragmentShaderPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"quad.frag"];
        NSString *vertexShaderPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"quad.vert"];

        shaderProgram = difont::examples::OpenGL::loadShaderProgram([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
        logoGenerator->SetupVertexArrays(shaderProgram);
        isSetup = true;
    }
}


- (void) update {
    if (logoGenerator) {
        //logoGenerator->Update(shaderProgram);
    }
}


- (void) render {
    if (logoGenerator) {
        logoGenerator->Render(shaderProgram);
    }
}


- (void) takeScreenShotAndExit {
    /*
    if (framesUntilScreenshot++ > 1 && screenShotr != nullptr) {

        NSString *rootPath = [[NSFileManager defaultManager] currentDirectoryPath];
        //NSString *dst = [NSString pathWithComponents:@[rootPath, @"whatever2.png"]];

        NSString *dst = [NSString pathWithComponents:@[@"/Users/davidpetrie/Projects/aesthetics-engine", @"whatever2.png"]];
        screenShotr->TakeScreenshot([dst UTF8String]);
        //screenShotr->TakeScreenshot();
        exit(0);
    }*/
}


- (void) createFailed {
    [NSApp terminate:self];
}


- (void) dealloc {
    [self.glView release];
    [super dealloc];
}

@end
