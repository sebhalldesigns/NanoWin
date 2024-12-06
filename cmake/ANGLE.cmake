include(ExternalProject)

LIST(APPEND CMAKE_PROGRAM_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/extern/depot_tools)

ExternalProject_Add(
    angle_build
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/angle
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E echo $ENV{PATH} &&
        ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> python3 scripts/bootstrap.py &&
        ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> gn gen out/Static
    BUILD_COMMAND
        ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ninja -C out/Static
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

ExternalProject_Get_Property(angle_build BINARY_DIR)
add_library(angle STATIC IMPORTED)
set_target_properties(angle PROPERTIES
  IMPORTED_LOCATION "${BINARY_DIR}/out/Static/libangle.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/extern/angle/include"
)
