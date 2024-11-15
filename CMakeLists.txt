find_package(SDL2 REQUIRED)

# Add the library
add_library(NanoWin STATIC
    NanoWin.c
)

target_include_directories(NanoWin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIR}
    ../NanoDraw/
)

target_link_libraries(NanoWin PRIVATE
    ${SDL2_LIBRARIES}
    NanoDraw
)