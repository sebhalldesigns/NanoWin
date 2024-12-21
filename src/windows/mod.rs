use std::ffi::{CString, c_void};
use std::ptr;
use std::mem;


use skia_safe::gpu::ganesh::context_options;
use windows::core::*;
use windows::Win32::Foundation::*;
use windows::Win32::Graphics::Gdi::*;
use windows::Win32::Graphics::OpenGL::*;
use windows::Win32::System::LibraryLoader::*;
use windows::Win32::UI::WindowsAndMessaging::*;

use crate::{Context, AppDelegate, geometry::*};

use skia_safe::{gpu, Canvas, Color, Paint, Point, Surface, SurfaceProps, TextBlob, RCHandle, gpu::ganesh::DirectContext  };


static mut WINDOW_DATA: Option<PlatformWindow> = None;

static mut SKIA_CONTEXT : Option<*mut DirectContext> = None;
static mut SKIA_FONT : Option<*mut skia_safe::Font> = None;
static mut WINDOW_SIZE: Size = Size { width: 0.0, height: 0.0 };
static mut MOUSE_POS: Point = Point::new(0.0, 0.0);

pub struct PlatformContext 
{
    hinstance: HINSTANCE,
    window_class_name: PCWSTR,
}

#[derive(Clone, Copy)]
pub struct PlatformWindow
{
    handle: HWND,
    gl_context: GlContext,
    
}

#[derive(Clone, Copy)]
struct GlContext {
    hdc: HDC,
    hglrc: HGLRC,
}

fn load_wgl_function(name: &str) -> PROC 
{
    let name = CString::new(name).unwrap();

    println!("Loading function: {}\n", name.to_str().unwrap()); 

    let wgl_address = unsafe { 
        wglGetProcAddress(
            PCSTR(
                name.as_ptr() as *const u8
            )
        )
    };

    if wgl_address.is_some() 
    {
        return wgl_address;
    }
    else
    {
        let opengl32 = unsafe { LoadLibraryA(PCSTR("opengl32.dll".as_ptr() as *const u8)).unwrap() };

        if opengl32.is_invalid() {
            println!("Failed to load opengl32.dll!");
            return None;
        }
    
        let fallback = unsafe { 
            GetProcAddress(opengl32, PCSTR(
                name.as_ptr() as *const u8
            )) 
        };

        return fallback;

    }


    
    
}

fn load_skia_gl_function(name: &str) -> *const c_void 
{
    let proc_address = load_wgl_function(name);

    if proc_address.is_none() {
        println!("Failed to load function: {}", name);
        return std::ptr::null();
    }

    return proc_address.unwrap() as *const c_void;
  
}



unsafe fn create_opengl_context(hwnd: HWND) -> GlContext 
{

    let hdc = GetDC(hwnd);

    let pfd = PIXELFORMATDESCRIPTOR {
        nSize: mem::size_of::<PIXELFORMATDESCRIPTOR>() as u16,
        nVersion: 1,
        dwFlags: PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        iPixelType: PFD_TYPE_RGBA,
        cColorBits: 32,
        cDepthBits: 24,
        cStencilBits: 8,
        cAlphaBits: 8,
        iLayerType: 0,
        ..Default::default()
    };

    let pf = ChoosePixelFormat(hdc, &pfd);
    _ = SetPixelFormat(hdc, pf, &pfd);

    // Create a temporary context to load WGL extensions
    let temp_hglrc = wglCreateContext(hdc).expect("failed to create temporary context");
    _ = wglMakeCurrent(hdc, temp_hglrc);

    #[allow(non_snake_case)]
    let wglCreateContextAttribsARB: extern "system" fn(
        hdc: HDC,
        hsharecontext: HGLRC,
        attriblist: *const i32,
    ) -> HGLRC;

    {
        wglCreateContextAttribsARB = unsafe { std::mem::transmute(load_wgl_function("wglCreateContextAttribsARB")) };
    }

    // We also might need wglChoosePixelFormatARB, but for simplicity we reuse the chosen pixel format.

    let attribs = [
        0x2091, 3, // WGL_CONTEXT_MAJOR_VERSION_ARB
        0x2092, 3, // WGL_CONTEXT_MINOR_VERSION_ARB
        0x9126, 0x0001, // WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB
        0,
    ];

    let modern_hglrc = wglCreateContextAttribsARB(hdc, HGLRC::default(), attribs.as_ptr());

    _ = wglMakeCurrent(HDC::default(), HGLRC::default());
    _ = wglDeleteContext(temp_hglrc);

    _ = wglMakeCurrent(hdc, modern_hglrc);

    return GlContext { hdc, hglrc: modern_hglrc };
}

pub fn create_context(title: String) -> PlatformContext 
{

    let instance: HINSTANCE = unsafe { GetModuleHandleW(None).expect("failed to get module handle").into() };
    let window_class_name: PCWSTR = PCWSTR::from_raw(HSTRING::from("NanoWin").as_ptr());

    let window_class = WNDCLASSW 
    {
        style: CS_HREDRAW | CS_VREDRAW,
        lpfnWndProc: Some(wnd_proc),
        hInstance: instance,
        lpszClassName: window_class_name,
        ..Default::default()
    };

    unsafe { 
        if RegisterClassW(&window_class) == 0 
        { 
            panic!("failed to register window class"); 
        } 
    };


    return PlatformContext 
    {
        hinstance: instance,
        window_class_name: window_class_name
    };
}

pub fn run(context: Context, app_delegate: Box<dyn AppDelegate>, window_size_request: Size) 
{

    let initial_window = crate::Window 
    {
        title: context.title.clone(),
        size: Size { width: window_size_request.width as f32, height: window_size_request.height as f32 },
        platform_window: create_window(&context.platform_context, context.title.clone(), window_size_request)
    };

    unsafe {
        WINDOW_DATA = Some(initial_window.platform_window.clone());
    }
    
    unsafe {
        _ = ShowWindow(initial_window.platform_window.handle, SW_SHOW);
        _ = UpdateWindow(initial_window.platform_window.handle);
    }

    context.windows.set(vec![initial_window]);


    unsafe 
    {
        let mut msg = MSG::default();
        while GetMessageW(&mut msg, None, 0, 0).into() {
            _ = TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    
}

pub fn create_window(context: &PlatformContext, title: String, size: Size) -> PlatformWindow
{

    let window_title: PCWSTR = PCWSTR::from_raw(HSTRING::from(title).as_ptr());
    let window: HWND = unsafe {
        CreateWindowExW(
            WINDOW_EX_STYLE::default(),
            context.window_class_name,
            window_title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            size.width as i32,
            size.height as i32,
            None,
            None,
            context.hinstance,
            None,
        ).expect("failed to create window")
    };

    let gl_context = unsafe { create_opengl_context(window) };

    return PlatformWindow 
    {
        handle: window,
        gl_context: gl_context
    };
}

unsafe extern "system" fn wnd_proc(hwnd: HWND, msg: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT
{
    match msg
    {
        WM_DESTROY =>
        {
            PostQuitMessage(0);
            return LRESULT(0);
        }
        WM_SIZE => {
            let width = LOWORD(lparam.0 as usize) as i32;
            let height = HIWORD(lparam.0 as usize) as i32;
            //print!("width: {}, height: {}\n", width, height);
            WINDOW_SIZE = Size { width: width as f32, height: height as f32 };

            if let Some(window_data) = WINDOW_DATA {
                let gl_context = window_data.gl_context;
                _ = wglMakeCurrent(gl_context.hdc, gl_context.hglrc);
                glViewport(0, 0, width, height);
               println!("Resized window to {}x{}", width, height);
            }
            return LRESULT(0);
        }

        WM_MOUSEMOVE => {
            let x = LOWORD(lparam.0 as usize) as i32;
            let y = HIWORD(lparam.0 as usize) as i32;

            MOUSE_POS = Point::new(x as f32, y as f32);
            //println!("Mouse moved to {}, {}\n", x, y);
            // force a paint event
            _ = PostMessageW(hwnd, WM_PAINT, WPARAM(0), LPARAM(0));
            return LRESULT(0);
        }

        WM_ERASEBKGND => {
            return LRESULT(1);
        }

        WM_PAINT => {
            println!("PAINT");
            
            let mut ps = PAINTSTRUCT::default();
            _ = BeginPaint(hwnd, &mut ps);

            

            if let Some(window_data) = WINDOW_DATA {

                if SKIA_CONTEXT.is_none() {

                    let skia_backend = 
                    skia_safe::gpu::gl::Interface::new_load_with(load_skia_gl_function).expect("failed to create skia backend");
                    
                    let mut context_options = context_options::ContextOptions::new();
                    


                    let skia_context = skia_safe::gpu::direct_contexts::make_gl(skia_backend, None).expect("failed to create skia context");
                    let skia_context_raw = Box::leak(Box::new(skia_context));

                    SKIA_CONTEXT = Some(skia_context_raw);

                    println!("Created skia context {}\n", skia_context_raw as *mut DirectContext as usize);
                    

                    let font_mgr = skia_safe::FontMgr::default();
                    let typeface = font_mgr
                        .match_family_style("JetBrains Mono", skia_safe::FontStyle::normal())
                        .expect("Failed to load typeface");

                    let mut font = skia_safe::Font::from_typeface(typeface, 7.0);
                    font.set_subpixel(true);
                    let font_raw = Box::leak(Box::new(font));
                    SKIA_FONT = Some(font_raw);

                    println!("Created skia font {}\n", font_raw as *mut skia_safe::Font as usize);
                }

                _ = wglMakeCurrent(window_data.gl_context.hdc, window_data.gl_context.hglrc);
                //glClearColor(1.0, 0.0, 0.0, 1.0);
                //glClear(GL_COLOR_BUFFER_BIT);

                let framebuffer_info = skia_safe::gpu::gl::FramebufferInfo {
                    fboid: 0,
                    format: skia_safe::gpu::gl::Format::RGBA8.into(),
                    ..Default::default()
                };

                let window_size = WINDOW_SIZE;

                let render_target = unsafe {
                    skia_safe::gpu::backend_render_targets::make_gl((window_size.width as i32, window_size.height as i32), 0, 8, framebuffer_info)
                };

                let mut surface  =  gpu::surfaces::wrap_backend_render_target(
                    &mut *SKIA_CONTEXT.unwrap(),
                    &render_target,
                    skia_safe::gpu::SurfaceOrigin::BottomLeft,
                    skia_safe::ColorType::RGBA8888,
                    None,
                    None,
                ).expect("failed to create surface");

                let canvas = surface.canvas();

                canvas.clear(skia_safe::Color4f::new(0.0, 1.0, 1.0, 1.0));

                let mouse_pos = MOUSE_POS;

                let mut text_paint = skia_safe::Paint::new(skia_safe::Color4f::new(0.0, 0.0, 0.0, 1.0), None);
                text_paint.set_anti_alias(true);
    
                
    
                
    
                let step_x = 50.0;
                let step_y = 9.0;
                
                let label = format!("{:4.0} {:4.0}", mouse_pos.x, mouse_pos.y);
                let blob = unsafe { TextBlob::new(&label, &mut *SKIA_FONT.unwrap()).expect("Failed to create text blob") };
                
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
                canvas.draw_rect(&rect, &paint);

                (&mut *SKIA_CONTEXT.unwrap()).flush_and_submit();

                drop(surface);
                
                _ = SwapBuffers(window_data.gl_context.hdc);
                _ = EndPaint(hwnd, &ps);
            }

            return LRESULT(0);
        }
        _ => {}

    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

fn LOWORD(l: usize) -> usize {
    l & 0xffff
}

fn HIWORD(l: usize) -> usize {
    (l >> 16) & 0xffff
}