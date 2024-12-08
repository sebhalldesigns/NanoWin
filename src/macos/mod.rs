// mod.rs
// NanoWin - macOS platform module
//
// This module contains the macOS platform specific code for NanoWin.
// It is responsible for app lifecycle management, window creation, graphics context management, and event handling.

mod nsappdelegate;
use nsappdelegate::NSAppDelegate;

mod nsmtkview;
use nsmtkview::NSMtkView;

use crate::{Context, AppDelegate, geometry::*};

use std::cell::Cell;
use objc2::rc::{Retained, Allocated};
use objc2::runtime::ProtocolObject;
use objc2_foundation::*; 
use objc2_app_kit::*;


#[link(name = "CoreGraphics", kind = "framework")]
extern "C" {}

use objc2_metal::{
    MTLClearColor, MTLCommandQueue, MTLCreateSystemDefaultDevice, MTLDevice, MTLDrawable
};

pub struct PlatformContext 
{
    pub main_thread_marker: MainThreadMarker,    
    pub app_delegate: Retained<NSAppDelegate>,
    pub app: Retained<NSApplication>,
}

pub struct PlatformWindow
{
    pub window: Retained<NSWindow>,
    pub view: Retained<NSMtkView>
}

// MARK: CONTEXT FNS

pub fn create_context(title: String) -> PlatformContext 
{

    let main_thread_marker = MainThreadMarker::new().expect("Failed to create MainThreadMarker");
    let app_delegate = NSAppDelegate::new(title, main_thread_marker);

    let app = NSApplication::sharedApplication(main_thread_marker);
    app.setActivationPolicy(NSApplicationActivationPolicy::Regular);
    app.setDelegate(Some(ProtocolObject::from_ref(&*app_delegate)));

    return PlatformContext 
    {
        main_thread_marker: main_thread_marker,
        app_delegate: app_delegate,
        app: app
    }
}

pub fn run(context: Context, app_delegate: Box<dyn AppDelegate>, window_size_request: Size) 
{
    context.platform_context.app_delegate.set_app_delegate(app_delegate);

    let initial_window = crate::Window 
    {
        title: context.title.clone(),
        size: Size { width: window_size_request.width as f32, height: window_size_request.height as f32 },
        platform_window: create_window(&context.platform_context, context.title.clone(), window_size_request)
    };

    context.windows.set(vec![initial_window]);

    unsafe 
    {
        context.platform_context.app.run();
    }
    
}

// MARK: WINDOW FNS

pub fn create_window(context: &PlatformContext, title: String, size: Size) -> PlatformWindow
{
    let window_alloc: Allocated<NSWindow> = context.main_thread_marker.alloc();


    let window_rect: NSRect = NSRect::new(
        NSPoint::new(0.0, 0.0), 
        NSSize::new(size.width as f64, size.height as f64)
    );
    
    let window_title = NSString::from_str(&title);

    let window_style_mask = 
          NSWindowStyleMask::Titled 
        | NSWindowStyleMask::Closable
        | NSWindowStyleMask::Resizable 
        | NSWindowStyleMask::Miniaturizable;

    let window_color = unsafe {
        NSColor::colorWithRed_green_blue_alpha(0.0, 0.0, 0.0, 1.0)
    };

    let window: Retained<NSWindow> = unsafe  {
        NSWindow::initWithContentRect_styleMask_backing_defer(window_alloc, window_rect, window_style_mask, NSBackingStoreType::NSBackingStoreBuffered, true)
    };

    window.setTitle(&window_title);   
    window.setBackgroundColor(Some(&window_color));
    window.center(); 
    window.makeKeyAndOrderFront(None);
    window.setAcceptsMouseMovedEvents(true);
    
    let view = NSMtkView::new(context.main_thread_marker, window_rect.clone());
    window.setContentView(Some(&*view)); 

    unsafe {

        view.setPaused(true);
        view.setPresentsWithTransaction(false);
        view.setEnableSetNeedsDisplay(true);
        view.setAutoResizeDrawable(true);
        view.setLayerContentsPlacement(NSViewLayerContentsPlacement::TopLeft);
        view.setLayerContentsRedrawPolicy(NSViewLayerContentsRedrawPolicy::NSViewLayerContentsRedrawDuringViewResize);
        view.setWantsLayer(true);
        view.setDelegate(Some(ProtocolObject::from_ref(&*view)));
    }

    return PlatformWindow 
    {
        window: window,
        view: view
    }
}