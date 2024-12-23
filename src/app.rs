// app.rs - app module
// This module provides app types and functions for NanoWin.

pub use crate::platform;
pub use crate::window::*;
pub use crate::version::*;
pub use crate::geometry::*;

pub type App = usize;

pub trait AppDelegate 
{
    fn application_did_finish_launching(&self);
    fn application_will_terminate(&self);
}

struct RawApp 
{
    title: String,
    description: String,
    developer: String,
    version: Version,
    delegate: Option<Box<dyn AppDelegate>>,
    platform_context: platform::PlatformContext,
    windows: Vec<Window>
}

// MARK: APP API

pub fn create_app() -> App
{

    let raw_app = RawApp {
        title: "NanoKit Application".to_string(),
        description: "A NanoKit application".to_string(),
        developer: "NanoKit".to_string(),
        version: Version { major: 1, minor: 0, sub: 0 },
        platform_context: platform::create_context(),
        delegate: None,
        windows: Vec::new()
    };

    return Box::into_raw(Box::new(raw_app)) as usize;
}

pub fn run_app(app: App) -> i32
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };

    if raw_app.delegate.is_none()
    {
        println!("No delegate set for app!");
        return -1;
    }

    let delegate = raw_app.delegate.as_ref().unwrap();
    delegate.application_did_finish_launching();

    platform::run();

    return 0;
}

// MARK: APP PROPERTIES

pub fn set_app_title(app: App, title: &str)
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    raw_app.title = title.to_string();
}

pub fn get_app_title(app: App) -> String
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    return raw_app.title.clone();
}

pub fn set_app_description(app: App, description: &str)
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    raw_app.description = description.to_string();
}

pub fn get_app_description(app: App) -> String
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    return raw_app.description.clone();
}

pub fn set_app_developer(app: App, developer: String)
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    raw_app.developer = developer;
}

pub fn get_app_developer(app: App) -> String
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    return raw_app.developer.clone();
}

pub fn set_app_version(app: App, version: Version)
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    raw_app.version = version;
}

pub fn get_app_version(app: App) -> Version
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    return raw_app.version.clone();
}

pub fn set_app_delegate(app: App, delegate: Box<dyn AppDelegate>)
{
    let raw_app = unsafe { &mut *(app as *mut RawApp) };
    raw_app.delegate = Some(delegate);
}
