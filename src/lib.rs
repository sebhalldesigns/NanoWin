// lib.rs
// NanoWin - NanoKit platform module
//
// This module provides app lifecycle management, window creation, graphics context management, and event handling.
// It can be used standalone or combined with other NanoKit modules.

#[cfg(target_os = "macos")]
pub mod macos;
#[cfg(target_os = "macos")]
pub use macos as platform;

#[cfg(target_os = "windows")]
pub mod windows;
#[cfg(target_os = "windows")]
pub use windows as platform;

#[cfg(target_arch = "wasm32")]
pub mod wasm;
#[cfg(target_arch = "wasm32")]
pub use wasm as platform;

pub mod app;
pub use app::*;

pub mod window;
pub use window::*;

pub mod geometry;
pub use geometry::*;

pub mod version;
pub use version::*;



