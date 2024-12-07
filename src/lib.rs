
use std::os::macos;

use std::cell::Cell;
use core::ffi::c_void;
use objc2::rc::autoreleasepool;
use objc2::rc::Allocated;
use objc2::rc::Retained;
use objc2::runtime::ProtocolObject;
use objc2::{declare_class, msg_send_id, mutability, ClassType, DeclaredClass};
use objc2_app_kit::NSViewLayerContentsPlacement;
use objc2_app_kit::NSViewLayerContentsRedrawPolicy;
use objc2_app_kit::{NSApplication, NSApplicationActivationPolicy, NSApplicationDelegate, NSWindow, NSWindowStyleMask, NSBackingStoreType, NSView, NSColor, NSEvent, NSResponder, NSViewController};
use objc2_foundation::{
    ns_string, MainThreadMarker, NSCopying, NSNotification, NSObject, NSObjectProtocol, NSString, NSAutoreleasePool, NSRect, NSPoint, NSSize, CGRect
}; 

use objc2_metal::MTLCommandBuffer;
use objc2_metal_kit::{MTKView, MTKViewDelegate};

use objc2_quartz_core::CAMetalDrawable;
use skia_safe::{gpu, Canvas, Color, Paint, Point, Surface, SurfaceProps, TextBlob, RCHandle, gpu::ganesh::DirectContext  };
use std::time::Instant;

#[link(name = "CoreGraphics", kind = "framework")]
extern "C" {}

use objc2_metal::{
    MTLDevice,
    MTLCreateSystemDefaultDevice,
    MTLCommandQueue,
    MTLDrawable
};

pub struct MacosContext 
{
    pub app_delegate: Retained<AppDelegate>,
    pub main_thread_marker: MainThreadMarker,
    pub pool: Retained<NSAutoreleasePool>,
    pub app: Retained<NSApplication>,
    pub metal_device: Retained<ProtocolObject<dyn MTLDevice>>
}

pub struct Context 
{
    pub macos_context: MacosContext
}

pub struct MacosWindow 
{
    pub window: Retained<NSWindow>,
    pub mtk_view: Retained<CustomMtkView>
}

pub struct Window 
{
    pub raw_handle: usize,
    pub width: f32,
    pub height: f32,
    pub title: String,
    pub macos_window: MacosWindow
}


#[allow(unused)]
struct Ivars {
    ivar: u8,
    canvas_size: Cell<Option<skia_safe::Size>>, // Use Cell for interior mutability
    iter: Cell<u32>,
    skia_context: Cell<Option<*mut DirectContext>>,
    font: Cell<Option<*mut skia_safe::Font>>
}

declare_class!(
    struct AppDelegate;

    // SAFETY:
    // - The superclass NSObject does not have any subclassing requirements.
    // - Main thread only mutability is correct, since this is an application delegate.
    // - `AppDelegate` does not implement `Drop`.
    unsafe impl ClassType for AppDelegate {
        type Super = NSObject;
        type Mutability = mutability::MainThreadOnly;
        const NAME: &'static str = "MyAppDelegate";
    }

    impl DeclaredClass for AppDelegate {
        type Ivars = Ivars;
    }

    unsafe impl NSObjectProtocol for AppDelegate {}

    unsafe impl NSApplicationDelegate for AppDelegate {
        #[method(applicationDidFinishLaunching:)]
        fn did_finish_launching(&self, notification: &NSNotification) {
            println!("Did finish launching!");
            // Do something with the notification
            dbg!(notification);
        }

        #[method(applicationWillTerminate:)]
        fn will_terminate(&self, _notification: &NSNotification) {
            println!("Will terminate!");
        }

        #[method(mouseMoved:)]
        fn mouse_moved(&self, event: &NSEvent) {
            unsafe {
                let location = event.locationInWindow();
                println!("Mouse moved to: ({}, {})", location.x, location.y);

            }
            
        }
    }

    unsafe impl MTKViewDelegate for AppDelegate {
        #[method(mtkView:drawableSizeWillChange:)]
        fn drawable_size_will_change(&self, view: &CustomMtkView, size: NSSize) {
            println!("Drawable size will change!");
            // Do something with the view and size
            let canvas_size = skia_safe::Size::new(size.width as f32, size.height as f32);
            self.ivars().canvas_size.set(Some(canvas_size)); // Update canvas size
            
            unsafe {
                
                view.layer().unwrap().display(); // Force the layer to update its drawable
                let _ = view.currentDrawable(); // Ensure the view has a drawable
            
                view.setNeedsDisplay(true); // Mark the view for redisplay
                view.draw();
                
            }
        }

        #[method(drawInMTKView:)]
        fn draw_in_mtk_view(&self, view: &CustomMtkView) {

            

            unsafe {
               
                let canvas_size = self.ivars().canvas_size.get().unwrap_or_else(|| {
                    skia_safe::Size::new(view.drawableSize().width as f32, view.drawableSize().height as f32)
                });

                let drawable_size = skia_safe::Size::new(view.drawableSize().width as f32, view.drawableSize().height as f32);

                // Skip rendering if sizes don't match
                if canvas_size != drawable_size {
                    println!(
                        "Skipping frame: canvas size {}x{}, drawable size {}x{}",
                        canvas_size.width, canvas_size.height, drawable_size.width, drawable_size.height
                    );
                    return;
                }

                if self.ivars().skia_context.get() == None {
                    let device = view.device().unwrap();
                    let command_queue = device.newCommandQueue().unwrap();
                    let skia_backend = skia_safe::gpu::mtl::BackendContext::new(
                        Retained::as_ptr(&device) as *const c_void,
                        Retained::as_ptr(&command_queue) as *const c_void,
                    );
                    let mut skia_context = skia_safe::gpu::direct_contexts::make_metal(&skia_backend, None).unwrap();
                    let mut skia_context_raw = Box::leak(Box::new(skia_context));
                    self.ivars().skia_context.set(Some(skia_context_raw));

                    let font_mgr = skia_safe::FontMgr::default();
                    let typeface = font_mgr
                        .match_family_style("JetBrains Mono", skia_safe::FontStyle::normal())
                        .expect("Failed to load typeface");

                    let mut font = skia_safe::Font::from_typeface(typeface, 7.0);
                    font.set_subpixel(true);
                    let mut font_raw = Box::leak(Box::new(font));
                    self.ivars().font.set(Some(font_raw));
                }
                
               

                let command_queue = view.device().unwrap().newCommandQueue().unwrap();


               

                let device = view.device().unwrap();

                

                let drawable = view.currentDrawable().unwrap();
                
                let start = Instant::now();
                println!("Draw in MTK view!");

                let texture_info = skia_safe::gpu::mtl::TextureInfo::new(
                    Retained::as_ptr(&drawable.texture()) as *const c_void
                );

                let render_target = skia_safe::gpu::backend_render_targets::make_mtl(
                    (view.drawableSize().width as i32, view.drawableSize().height as i32),
                    &texture_info
                );

                println!("Canvas size {}x{} drawable size {}x{}", canvas_size.width, canvas_size.height, view.drawableSize().width, view.drawableSize().height);

                let mut surface = skia_safe::gpu::surfaces::wrap_backend_render_target(
                    &mut *self.ivars().skia_context.get().unwrap(),
                    &render_target,
                    skia_safe::gpu::SurfaceOrigin::TopLeft,
                    skia_safe::ColorType::BGRA8888,
                    None,
                    None
                ).expect("Failed to create surface");

                let canvas = surface.canvas();
                canvas.clear(skia_safe::Color4f::new(1.0, 0.0, 1.0, 1.0));

                let rect_size = canvas_size / 2.0;
                let rect = skia_safe::Rect::from_point_and_size(
                    skia_safe::Point::new(
                        (canvas_size.width - rect_size.width) / 2.0,
                        (canvas_size.height - rect_size.height) / 2.0,
                    ),
                    rect_size,
                );

                let mut paint = skia_safe::Paint::new(
                    skia_safe::Color4f::new(0.0, 1.0, 1.0, 1.0),
                    None,
                );

                canvas.draw_rect(&rect, &paint);

                let mut text_paint = skia_safe::Paint::new(skia_safe::Color4f::new(0.0, 0.0, 0.0, 1.0), None);
                text_paint.set_anti_alias(true);

                

                

                let step_x = 50.0;
                let step_y = 9.0;

                self.ivars().iter.set(self.ivars().iter.get() + 1);
                
              
                for i in 0..10000 {
                    let row = i / 70;
                    let col = i % 70;
                    let x = col as f32 * step_x;
                    let y = row as f32 * step_y;
                    let label = format!("Label {}", self.ivars().iter.get());
                    let blob = TextBlob::new(&label, &mut *self.ivars().font.get().unwrap()).expect("Failed to create text blob");
                    
                    canvas.draw_text_blob(&blob, Point::new(x, y), &text_paint);
                }

                 

                let mouse_x = view.ivars().mouseX.get() as f32;
                let mouse_y = view.ivars().mouseY.get() as f32;
                let rect = skia_safe::Rect::from_point_and_size(
                    skia_safe::Point::new(mouse_x - 10.0, mouse_y - 10.0),
                    skia_safe::Size::new(20.0, 20.0),
                );

                let mut paint = skia_safe::Paint::new(
                    skia_safe::Color4f::new(1.0, 0.0, 0.0, 1.0),
                    None,
                );

                canvas.draw_rect(&rect, &paint);

                (*self.ivars().skia_context.get().unwrap()).flush_and_submit();

                drop(surface);

                let mtl_drawable: &ProtocolObject<dyn MTLDrawable> = drawable.as_ref().as_ref();

                
                let duration = start.elapsed();
                println!("Drawable in MTK view took: {:?}", duration);
                
                let command_buffer = command_queue.commandBuffer().unwrap();
                command_buffer.presentDrawable(mtl_drawable);
                command_buffer.commit();

             
            }

            
        }
    }
);

impl AppDelegate {
    fn new(ivar: u8, mtm: MainThreadMarker) -> Retained<Self> {
        let this = mtm.alloc();
        let this = this.set_ivars(Ivars {
            ivar,
            canvas_size: Cell::new(None),
            iter: Cell::new(0),
            skia_context: Cell::new(None),
            font: Cell::new(None)
        });
        unsafe { msg_send_id![super(this), init] }
    }

}

#[derive(Debug)]
#[allow(unused)]
struct ViewIvars {
    ivar: u8,
    canvas_size: Cell<Option<skia_safe::Size>>, // Use Cell for interior mutability
    iter: Cell<u32>,
    mouseX: Cell<f64>,
    mouseY: Cell<f64>,
}

declare_class!(
    struct CustomMtkView;

    unsafe impl ClassType for CustomMtkView {
        type Super = MTKView;
        type Mutability = mutability::MainThreadOnly;
        const NAME: &'static str = "CustomMtkView";
    }

    impl DeclaredClass for CustomMtkView {
        type Ivars = ViewIvars;
    }

    unsafe impl CustomMtkView {

        #[method(mouseMoved:)]
        unsafe fn mouse_moved(&self, event: &NSEvent) {
            let location = event.locationInWindow();
            println!("Mouse moved to: ({}, {})", location.x, location.y);
            self.ivars().mouseX.set(location.x);
            self.ivars().mouseY.set(self.frame().size.height - location.y);

            self.draw();
        }

        #[method(acceptsFirstResponder)]
        fn accepts_first_responder(&self) -> bool {
            true
        }

        #[method(becomeFirstResponder)]
        fn become_first_responder(&self) -> bool {
            true
        }

    }
    
);

impl CustomMtkView {
    fn new(mtm: MainThreadMarker, frame_rect: CGRect, device: Option<&ProtocolObject<dyn MTLDevice>>) -> Retained<Self> {
        let this = mtm.alloc();
        let this = this.set_ivars(ViewIvars {
            ivar: 0,
            canvas_size: Cell::new(None),
            iter: Cell::new(0),
            mouseX: Cell::new(0.0),
            mouseY: Cell::new(0.0),
        });

        unsafe {  msg_send_id![super(this), initWithFrame:frame_rect device:device]  }
    }
}




pub fn create_context() -> Context 
{
    unsafe 
    {
        let main_thread_marker: MainThreadMarker = MainThreadMarker::new().unwrap();
    
        let pool: Retained<NSAutoreleasePool> = NSAutoreleasePool::new();

        let app_delegate: Retained<AppDelegate> = AppDelegate::new(42, main_thread_marker);
        
        let app: Retained<NSApplication> = NSApplication::sharedApplication(main_thread_marker);
        app.setActivationPolicy(NSApplicationActivationPolicy::Regular);
        app.setDelegate(Some(ProtocolObject::from_ref(&*app_delegate)));

        let raw_device: *mut ProtocolObject<dyn MTLDevice> = MTLCreateSystemDefaultDevice();
        let device: Retained<ProtocolObject<dyn MTLDevice>> = Retained::from_raw(raw_device).unwrap();

        let macos_context = MacosContext 
        { 
            main_thread_marker: main_thread_marker, 
            app_delegate: app_delegate,
            pool: pool, 
            app: app, 
            metal_device: device 
        };

        let context: Context = Context { macos_context: macos_context };

        return context;
    }
}

pub fn create_window(context: &Context, width: f32, height: f32, title: &str) -> Window 
{
    unsafe 
    {   
        let window_alloc: Allocated<NSWindow> = context.macos_context.main_thread_marker.alloc();

        let window_rect: NSRect = NSRect::new(
            NSPoint::new(0.0, 0.0), 
            NSSize::new(width as f64, height as f64)
        );
        
        let window_title: Retained<NSString> = NSString::from_str(title);

        let window_style_mask: NSWindowStyleMask = 
              NSWindowStyleMask::Titled
            | NSWindowStyleMask::Closable
            | NSWindowStyleMask::Resizable
            | NSWindowStyleMask::Miniaturizable;
        let window = NSWindow::initWithContentRect_styleMask_backing_defer(
            window_alloc,
            window_rect.clone(),
            window_style_mask,
            NSBackingStoreType::NSBackingStoreBuffered,
            true,
        );

        window.setTitle(&window_title);
        window.center();
        window.makeKeyAndOrderFront(None);

        // Enable mouse tracking
        window.setAcceptsMouseMovedEvents(true);

        let window_color = NSColor::colorWithRed_green_blue_alpha(0.0, 0.0, 0.0, 1.0);
        window.setBackgroundColor(Some(&window_color));

        let metal_view = CustomMtkView::new(context.macos_context.main_thread_marker, window_rect.clone(), Some(&context.macos_context.metal_device));
        // Ensure the view can receive mouse events
        //metal_view.becomeFirstResponder();
        
        window.setContentView(Some(&metal_view));
        metal_view.setPaused(true);
        metal_view.setPresentsWithTransaction(false);
        metal_view.setEnableSetNeedsDisplay(true);
        metal_view.setAutoResizeDrawable(true);
        metal_view.setLayerContentsPlacement(NSViewLayerContentsPlacement::TopLeft);
        //metal_view.setLayerContentsRedrawPolicy(NSViewLayerContentsRedrawPolicy::NSViewLayerContentsRedrawNever);
        metal_view.setWantsLayer(true);
        metal_view.setDelegate(Some(ProtocolObject::from_ref(&*context.macos_context.app_delegate)));

        let raw_handle = 0 as usize;

        let macos_window = MacosWindow 
        {
            window: window,
            mtk_view: metal_view
        };

        return Window { raw_handle, width, height, title: title.to_string(), macos_window: macos_window };
    }
}

pub fn run(context: &Context) 
{
    unsafe 
    {
        context.macos_context.app.run();
    }
}