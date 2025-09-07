cmake_minimum_required(VERSION 3.14)
project(lazyimgui VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create library target
add_library(lazyimgui)

# Add source files
target_sources(lazyimgui
    PRIVATE
        src/lazyimgui.cpp
        # ImGui core
        lib/imgui/imgui.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/misc/cpp/imgui_stdlib.cpp
        # ImGui Allegro backend
        lib/imgui/backends/imgui_impl_allegro5.cpp
        # Date chooser addon
        lib/imgui/addons/imguidatechooser/imguidatechooser.cpp
)


# set(DIR__INCLUDES   "include")
# set(DIR__SOURCES    "src")
# set(DIR__LIBRARIES  "lib")

# set(DIR__LIB_INCLUDES
#     "${CMAKE_CURRENT_SOURCE_DIR}/${DIR__LIBRARIES}/imgui" 
#     "${CMAKE_CURRENT_SOURCE_DIR}/${DIR__LIBRARIES}/imgui/backends" 
#     "${CMAKE_CURRENT_SOURCE_DIR}/${DIR__LIBRARIES}/imgui/misc/cpp" 
#     "${CMAKE_CURRENT_SOURCE_DIR}/${DIR__LIBRARIES}/Pos2d/include" 
# )

# file(GLOB FILES__LIB_SOURCES__IMGUI CONFIGURE_DEPENDS "${DIR__LIBRARIES}/imgui/imgui*.cpp")
# list(APPEND FILES__LIB_SOURCES__IMGUI "${DIR__LIBRARIES}/imgui/backends/imgui_impl_allegro5.cpp")
# list(APPEND FILES__LIB_SOURCES__IMGUI "${DIR__LIBRARIES}/imgui/addons/imguidatechooser/imguidatechooser.cpp")
# list(APPEND FILES__LIB_SOURCES__IMGUI "${DIR__LIBRARIES}/imgui/misc/cpp/imgui_stdlib.cpp")


# Add include directories (PUBLIC for downstream projects)
target_include_directories(lazyimgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/misc/cpp>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/addons/imguidatechooser>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/Pos2d/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set public headers for installation
set_target_properties(lazyimgui PROPERTIES
    PUBLIC_HEADER "include/lazyimgui.hpp"
)

# Install all header files from include/
install(DIRECTORY include/ DESTINATION include)

# Install ImGui headers
install(DIRECTORY lib/imgui/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install ImGui backend headers
install(DIRECTORY lib/imgui/misc/cpp/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install ImGui backend headers
install(DIRECTORY lib/imgui/backends/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install DateChooser headers
install(DIRECTORY lib/imgui/addons/imguidatechooser/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install Pos2d header (header-only library)
install(DIRECTORY lib/Pos2d/include/ DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Install targets
install(TARGETS lazyimgui
    EXPORT lazyimguiTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)


# Install export file
install(EXPORT lazyimguiTargets
    FILE lazyimguiConfig.cmake
    NAMESPACE lazyimgui::
    DESTINATION lib/cmake/lazyimgui
)

# Create config file for find_package
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lazyimguiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/lazyimguiConfig.cmake
    INSTALL_DESTINATION lib/cmake/lazyimgui
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/lazyimguiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lazyimguiConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/lazyimguiConfigVersion.cmake
    DESTINATION lib/cmake/lazyimgui
)

# Export for build tree
export(EXPORT lazyimguiTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/lazyimguiTargets.cmake
)


# library build :  cmake .. -DCMAKE_INSTALL_PREFIX="{library install dir}"; cmake --build . --config Release; cmake --install . --config Release --prefix {library install dir}
# app build     :  cmake .. -DCMAKE_INSTALL_PREFIX="{library install dir}"; cmake --build . --config Release
