
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;
use web_sys::*;
use std::cell::Cell;
use std::cell::RefCell;
use std::rc::Rc;

use crate::{Context, AppDelegate, WindowDelegate, geometry::*};

pub struct PlatformContext 
{
    window: web_sys::Window,
    document: web_sys::Document,
    body: web_sys::HtmlElement
}

pub struct PlatformWindow
{
    
}

pub fn create_context(title: String) -> PlatformContext 
{

        
    console::log_1(&"Create context for app!".into());


    let window = web_sys::window().unwrap();
    let document = window.document().unwrap();
    let body = document.body().unwrap();

    
    return PlatformContext 
    {
        window: window,
        document: document,
        body: body
    }
}

pub fn create_window(context: &PlatformContext, title: String, size: Size) -> PlatformWindow
{
    return PlatformWindow
    {
        
    };
}


pub fn run(context: ContextHandle, app_delegate: Box<dyn AppDelegate>, window_delegate: Box<dyn WindowDelegate>, window_size_request: Size) 
{
    
    app_delegate.application_did_finish_launching();

    let initial_window = crate::Window 
    {
        title: context.title.clone(),
        size: Size { width: window_size_request.width as f32, height: window_size_request.height as f32 },
        platform_window: create_window(&context.platform_context, context.title.clone(), window_size_request),
        layers: Vec::new()
    };

    let window_handle = crate::get_window_handle(initial_window);

    context.windows.push(window_handle.clone());

    window_delegate.window_did_load(window_handle);

    unsafe 
    {
        
    }
    
}

pub struct PlatformLayer2D
{
    canvas_element: Element,
    canvas: HtmlCanvasElement,
    context_2d: CanvasRenderingContext2d
}

pub fn create_layer_2d(context: PlatformContext, window: PlatformWindow) -> PlatformLayer2D
{
    let canvas_element = context.document.create_element("canvas").unwrap();
    context.body.append_child(&canvas_element).unwrap();

    let canvas: web_sys::HtmlCanvasElement = canvas_element.clone()
        .dyn_into::<web_sys::HtmlCanvasElement>()
        .map_err(|_| ())
        .unwrap();


    let style = canvas.style();

    style.set_property("width", "100vw").unwrap();
    style.set_property("height", "100vh").unwrap();
    style.set_property("position", "absolute").unwrap();
    style.set_property("margin", "0").unwrap();
    style.set_property("top", "0").unwrap();
    style.set_property("left", "0").unwrap();

    canvas.set_width(context.window.inner_width().unwrap().as_f64().unwrap() as u32);
    canvas.set_height(context.window.inner_height().unwrap().as_f64().unwrap() as u32);

    let context_2d = canvas
        .get_context("2d")
        .unwrap()
        .unwrap()
        .dyn_into::<web_sys::CanvasRenderingContext2d>()
        .unwrap();

    return PlatformLayer2D
    {
        canvas_element: canvas_element,
        canvas: canvas,
        context_2d: context_2d
    };
}

pub struct PlatformLayer3D
{
    //canvas_element: HtmlCanvasElement,
    //context_3d: web_sys::WebGl2RenderingContext
}

pub fn create_layer_3d(context: PlatformContext, window: PlatformWindow) -> PlatformLayer3D
{
    return PlatformLayer3D
    {
        
    };
}