// geometry.rs - geometry module
// This module provides geometry types and functions for NanoWin.

pub struct Point
{
    pub x: f32,
    pub y: f32
}

pub struct Size
{
    pub width: f32,
    pub height: f32
}

pub struct Rect
{
    pub origin: Point,
    pub size: Size
}