use crate::geometry::*;

use std::ffi::c_void;
use std::cell::Cell;

use objc2::{declare_class, msg_send_id, mutability, ClassType, DeclaredClass};
use objc2::runtime::ProtocolObject;
use objc2::rc::Retained;
use objc2_foundation::{
    ns_string, MainThreadMarker, NSCopying, NSNotification, NSObject, NSObjectProtocol, NSString, NSAutoreleasePool, NSRect, NSPoint, NSSize, CGRect
}; 
use objc2_quartz_core::CAMetalDrawable;
use objc2_app_kit::{NSApplication, NSApplicationActivationPolicy, NSApplicationDelegate, NSWindow, NSWindowStyleMask, NSBackingStoreType, NSView, NSColor, NSEvent, NSResponder, NSViewController};

use objc2_metal::{
    MTLDevice,
    MTLCreateSystemDefaultDevice,
    MTLCommandQueue,
    MTLCommandBuffer,
    MTLDrawable
};

use objc2_metal_kit::{MTKView, MTKViewDelegate};


use skia_safe::{gpu, Canvas, Color, Paint, Point, Surface, SurfaceProps, TextBlob, RCHandle, gpu::ganesh::DirectContext  };

#[link(name = "CoreGraphics", kind = "framework")]
extern "C" {}




pub struct ViewIvars {
    canvas_size: Cell<Size>,
    mouse_pos: Cell<Point>,
    skia_context: Cell<Option<*mut DirectContext>>,
    font: Cell<Option<*mut skia_safe::Font>>
}


declare_class!(
    pub struct NSMtkView;

    unsafe impl ClassType for NSMtkView {
        type Super = MTKView;
        type Mutability = mutability::MainThreadOnly;
        const NAME: &'static str = "NSMtkView";
    }

    impl DeclaredClass for NSMtkView {
        type Ivars = ViewIvars;
    }

    unsafe impl NSObjectProtocol for NSMtkView {}

    unsafe impl MTKViewDelegate for NSMtkView {

        #[method(mtkView:drawableSizeWillChange:)]
        fn drawable_size_will_change(&self, view: &NSMtkView, size: NSSize) {
            println!("drawableSizeWillChange");
        }

        #[method(drawInMTKView:)]
        fn draw_in_mtk_view(&self, view: &NSMtkView) {
            println!("drawInMTKView");
            
            let drawable_option = unsafe { view.currentDrawable() };
            if drawable_option.is_none() {
                return;
            }

            let drawable = drawable_option.unwrap();
            let device = unsafe { view.device().expect("Failed to get device") };
            let command_queue = device.newCommandQueue().expect("Failed to create command queue");

            if self.ivars().skia_context.get() == None {
                
                let skia_backend = unsafe { 
                    skia_safe::gpu::mtl::BackendContext::new(
                        Retained::as_ptr(&device) as *const c_void,
                        Retained::as_ptr(&command_queue) as *const c_void,
                    )
                };

                let skia_context = skia_safe::gpu::direct_contexts::make_metal(&skia_backend, None).unwrap();
                let skia_context_raw = Box::leak(Box::new(skia_context));
                self.ivars().skia_context.set(Some(skia_context_raw));

                let font_mgr = skia_safe::FontMgr::default();
                let typeface = font_mgr
                    .match_family_style("JetBrains Mono", skia_safe::FontStyle::normal())
                    .expect("Failed to load typeface");

                let mut font = skia_safe::Font::from_typeface(typeface, 7.0);
                font.set_subpixel(true);
                let font_raw = Box::leak(Box::new(font));
                self.ivars().font.set(Some(font_raw));
            }

            let texture_info = unsafe {
                skia_safe::gpu::mtl::TextureInfo::new(
                    Retained::as_ptr(&drawable.texture()) as *const c_void
                )
            };


            let render_target = unsafe {
                skia_safe::gpu::backend_render_targets::make_mtl(
                    (view.drawableSize().width as i32, view.drawableSize().height as i32),
                    &texture_info
                )
            };

            let mut surface = unsafe { 
                skia_safe::gpu::surfaces::wrap_backend_render_target(
                    &mut *self.ivars().skia_context.get().unwrap(),
                    &render_target,
                    skia_safe::gpu::SurfaceOrigin::TopLeft,
                    skia_safe::ColorType::BGRA8888,
                    None,
                    None
                ).expect("Failed to create surface")
            };

            let canvas = surface.canvas();
            canvas.clear(skia_safe::Color4f::new(0.0, 1.0, 1.0, 1.0));

            let mouse_pos = self.ivars().mouse_pos.get();

            let mut text_paint = skia_safe::Paint::new(skia_safe::Color4f::new(0.0, 0.0, 0.0, 1.0), None);
            text_paint.set_anti_alias(true);

            

            

            let step_x = 50.0;
            let step_y = 9.0;
            
            let label = format!("{:4.0} {:4.0}", mouse_pos.x, mouse_pos.y);
            let blob = unsafe { TextBlob::new(&label, &mut *self.ivars().font.get().unwrap()).expect("Failed to create text blob") };
            
            for i in 0..10000 {
                let row = i / 70;
                let col = i % 70;
                let x = col as f32 * step_x;
                let y = row as f32 * step_y;
                
                canvas.draw_text_blob(&blob, Point::new(x, y), &text_paint);
            }

                 

            let rect = skia_safe::Rect::from_point_and_size(
                skia_safe::Point::new(mouse_pos.x - 10.0, mouse_pos.y - 10.0),
                skia_safe::Size::new(20.0, 20.0),
            );

            let paint = skia_safe::Paint::new(
                skia_safe::Color4f::new(1.0, 0.0, 0.0, 1.0),
                None,
            );

            canvas.draw_rect(&rect, &paint);

            unsafe {
                (*self.ivars().skia_context.get().unwrap()).flush_and_submit();
            }


            drop(surface);

            let mtl_drawable: &ProtocolObject<dyn MTLDrawable> = drawable.as_ref().as_ref();

            let command_buffer = command_queue.commandBuffer().unwrap();
            command_buffer.presentDrawable(mtl_drawable);
            command_buffer.commit();
        }
    }


    unsafe impl NSMtkView {

        #[method(mouseMoved:)]
        unsafe fn mouse_moved(&self, event: &NSEvent) {
            let location = event.locationInWindow();
            println!("Mouse moved to: ({}, {})", location.x, location.y);
            self.ivars().mouse_pos.set(Point { x: location.x as f32, y: self.frame().size.height as f32 - location.y as f32 });
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

impl NSMtkView {
    pub fn new(mtm: MainThreadMarker, frame_rect: CGRect) -> Retained<Self> {
        let this = mtm.alloc();
        let this = this.set_ivars(ViewIvars {
            canvas_size: Cell::new(Size { width: frame_rect.size.width as f32, height: frame_rect.size.height as f32 }),
            mouse_pos: Cell::new(Point { x: -1.0, y: -1.0 }),
            skia_context: Cell::new(None),
            font: Cell::new(None)
        });

        let device = unsafe { MTLCreateSystemDefaultDevice() };

        unsafe {  msg_send_id![super(this), initWithFrame:frame_rect device:device]  }
    }
}

