pub struct MacosContext 
{
    pub app_delegate: Retained<AppDelegate>,
    pub main_thread_marker: MainThreadMarker,
    pub pool: Retained<NSAutoreleasePool>,
    pub app: Retained<NSApplication>,
    pub metal_device: Retained<ProtocolObject<dyn MTLDevice>>
}