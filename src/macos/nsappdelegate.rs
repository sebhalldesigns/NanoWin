
use crate::AppDelegate;

use std::cell::Cell;

use objc2::{declare_class, msg_send_id, mutability, ClassType, DeclaredClass};
use objc2::rc::Retained;
use objc2_app_kit::{NSApplication, NSApplicationActivationPolicy, NSApplicationDelegate, NSWindow, NSWindowStyleMask, NSBackingStoreType, NSView, NSColor, NSEvent, NSResponder, NSViewController};
use objc2_foundation::{
    ns_string, MainThreadMarker, NSCopying, NSNotification, NSObject, NSObjectProtocol, NSString, NSAutoreleasePool, NSRect, NSPoint, NSSize, CGRect
}; 

#[allow(unused)]
pub struct AppDelegateIvars 
{
    title: Cell<String>,
    app_delegate: Cell<Option<Box<dyn AppDelegate>>>,
}

declare_class!
(
    pub struct NSAppDelegate;

    unsafe impl ClassType for NSAppDelegate 
    {
        type Super = NSObject;
        type Mutability = mutability::MainThreadOnly;
        const NAME: &'static str = "MyAppDelegate";
    }

    impl DeclaredClass for NSAppDelegate 
    {
        type Ivars = AppDelegateIvars;
    }

    unsafe impl NSObjectProtocol for NSAppDelegate {}

    unsafe impl NSApplicationDelegate for NSAppDelegate 
    {
        #[method(applicationDidFinishLaunching:)]
        fn did_finish_launching(&self, _notification: &NSNotification) 
        {
            println!("Application did finish launching!");

            let app_delegate = self.ivars().app_delegate.take();

            if let Some(app_delegate) = app_delegate.as_ref() 
            {
                app_delegate.application_did_finish_launching();
            }
            
            self.ivars().app_delegate.set(app_delegate);
        }

        #[method(applicationWillTerminate:)]
        fn will_terminate(&self, _notification: &NSNotification) 
        {
            println!("Application will terminate!");
            let app_delegate = self.ivars().app_delegate.take();

            if let Some(app_delegate) = app_delegate.as_ref() 
            {
                app_delegate.application_will_terminate();
            }
            
            self.ivars().app_delegate.set(app_delegate);
        }
    }
);

impl NSAppDelegate 
{
    pub fn new(title: String, mtm: MainThreadMarker) -> Retained<Self> 
    {
        let this = mtm.alloc();
        let this = this.set_ivars
        (
            AppDelegateIvars 
            {
                title: Cell::new(title),
                app_delegate: Cell::new(None),
            }
        );

        unsafe 
        { 
            return msg_send_id![super(this), init];
        }
    }

    pub fn set_app_delegate(&self, app_delegate: Box<dyn AppDelegate>) 
    {
        self.ivars().app_delegate.set(Some(app_delegate));
    }
}
