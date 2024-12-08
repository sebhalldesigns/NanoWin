// lib.rs
// NanoWin - NanoKit platform module
//
// This module provides app lifecycle management, window creation, graphics context management, and event handling.
// It can be used standalone or combined with other NanoKit modules.

#[cfg(target_os = "macos")]
mod macos;
#[cfg(target_os = "macos")]
use macos as platform;

#[cfg(target_os = "windows")]
mod windows;
#[cfg(target_os = "windows")]
use windows as platform;

#[cfg(target_arch = "wasm32")]
mod wasm;
#[cfg(target_arch = "wasm32")]
use wasm as platform;

use std::cell::Cell;

pub mod geometry;
pub use geometry::*;

pub struct Window
{
    pub size: Size,
    pub title: String,
    pub platform_window: platform::PlatformWindow
}

pub trait AppDelegate 
{
    fn application_did_finish_launching(&self);
    fn application_will_terminate(&self);
}

pub trait WindowDelegate 
{
    fn window_was_created(&self, window: Window);
    fn window_did_appear(&self, window: Window);
    fn window_did_resize(&self, window: Window);
}

pub struct Context 
{
    title: String,
    platform_context: platform::PlatformContext,

    windows: Cell<Vec<Window>>
}

pub fn create_context(title: String) -> Context 
{
    return Context 
    {
        title: title.clone(),
        platform_context: platform::create_context(title),
        windows: Cell::new(Vec::new())
    };
}

pub fn run(context: Context, app_delegate: Box<dyn AppDelegate>, window_size_request: Size) 
{
    platform::run(context, app_delegate, window_size_request);
}
