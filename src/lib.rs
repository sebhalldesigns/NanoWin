
use std::os::macos;

use std::cell::Cell;

use skia_safe::{gpu, Canvas, Color, Paint, Point, Surface, SurfaceProps, TextBlob, RCHandle, gpu::ganesh::DirectContext  };
use std::time::Instant;

#[cfg(target_os = "macos")]
//mod macos;

#[cfg(target_os = "macos")]
//use macos::MacosContext as PlatformContext;

#[cfg(target_arch = "wasm32")]
mod wasm;

#[cfg(target_arch = "wasm32")]
use wasm::WasmContext as PlatformContext;



pub struct Context 
{
    pub macos_context: PlatformContext}
}
