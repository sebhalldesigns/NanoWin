cmake_minimum_required(VERSION 3.10)

project(NanoWin)

if (WIN32)

    set(NANOWIN_SOURCES
        lib/backends/win32/nanowin.c
    )

    set(NANOWIN_LIBS
        user32
        opengl32
    )

elseif(UNIX)

    set(NANOWIN_SOURCES
        lib/backends/x11/nanowin.c
    )

elseif(APPLE)

    set(NANOWIN_SOURCES
        lib/backends/cocoa/nanowin.m
    )
    
elseif(EMSCRIPTEN)

    set(NANOWIN_SOURCES
        lib/backends/wasm/nanowin.c
    )
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

add_library(NanoWin STATIC 
    ${NANOWIN_SOURCES}
)

target_include_directories(NanoWin PUBLIC
    lib
)

target_link_libraries(NanoWin PUBLIC
    ${NANOWIN_LIBS}
    NanoDraw
    NanoView
)
