include(ExternalProject)

set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}/extern/depot_tools:$ENV{PATH}")

ExternalProject_Add(
    skia_build
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/skia
    CONFIGURE_COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/extern/skia/tools/git-sync-deps && python3 ${CMAKE_CURRENT_SOURCE_DIR}/extern/skia/bin/fetch-ninja && bin/gn gen out/Static
    BUILD_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/extern/depot_tools/ninja -C out/Static
    INSTALL_COMMAND ""
    CONFIGURE_HANDLED_BY_BUILD ON
    BUILD_IN_SOURCE 1
)

ExternalProject_Get_Property(skia_build BINARY_DIR)
add_library(skia STATIC IMPORTED)
set_target_properties(skia PROPERTIES
  IMPORTED_LOCATION "${BINARY_DIR}/out/Static/libskia.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/extern/skia/include"
)
