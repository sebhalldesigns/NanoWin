find_package(SDL2 REQUIRED)

# Add the library
add_library(NanoWin STATIC
    lib/NanoWin.c
    ${SDL2_INCLUDE_DIR}
)

target_include_directories(NanoWin PRIVATE 
    lib
)

target_link_libraries(NanoWin 
    ${SDL2_LIBRARIES}
)