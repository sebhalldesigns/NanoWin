# Add the library
add_library(NanoWin STATIC
    lib/NanoWin.c
)

target_include_directories(NanoWin PUBLIC
    include
)

include(cmake/Skia.cmake)
include(cmake/ANGLE.cmake)