// window.rs - window module
// This module provides window types and functions for NanoWin.

//pub use crate::platform;
pub use crate::geometry::*;
pub use crate::app::*;

pub type Window = usize;
pub type Layer = usize;

pub trait WindowDelegate 
{
    fn window_did_load(&self, window: Window);
    fn window_did_change_visibility(&self, window: Window);
    fn window_did_resize(&self, window: Window);
}

struct RawWindow
{
    pub size: Size,
    pub title: String,
    pub delegate: Option<Box<dyn WindowDelegate>>,
    pub platform_window: platform::PlatformWindow
}

// MARK: WINDOW API

pub fn create_window() -> Window
{   
    let size = Size { width: 800.0, height: 600.0 };
    let title = "NanoKit Window";

    let raw_window = RawWindow {
        size: size,
        title: title.to_string(),
        platform_window: platform::create_window(title.to_string(), size),
        delegate: None
    };

    let window = Box::into_raw(Box::new(raw_window)) as usize;
    return window;
}

pub fn show_window(window: Window)
{
    let raw_window = unsafe { &mut *(window as *mut RawWindow) };
    //platform::show_window(raw_window.platform_window);
}

// MARK: WINDOW PROPERTIES

pub fn set_window_size(window: Window, size: Size)
{
    let raw_window = unsafe { &mut *(window as *mut RawWindow) };
    raw_window.size = size;
    //platform::set_window_size(raw_window.platform_window, size);
}

pub fn get_window_size(window: Window) -> Size
{
    let raw_window = unsafe { &mut *(window as *mut RawWindow) };
    return raw_window.size;
}

pub fn set_window_title(window: Window, title: &str)
{
    let raw_window = unsafe { &mut *(window as *mut RawWindow) };
    raw_window.title = title.to_string();
    //platform::set_window_title(raw_window.platform_window, title);
}

pub fn get_window_title(window: Window) -> String
{
    let raw_window = unsafe { &mut *(window as *mut RawWindow) };
    return raw_window.title.clone();
}

pub fn set_window_delegate(window: Window, delegate: Box<dyn WindowDelegate>)
{
    let raw_window = unsafe { &mut *(window as *mut RawWindow) };
    raw_window.delegate = Some(delegate);
}


