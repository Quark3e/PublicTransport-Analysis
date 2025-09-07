cmake_minimum_required(VERSION 3.14)

# if(WIN32)
#     set(BOOL_USE_VCPKG true)
    
# else()
    set(BOOL_USE_VCPKG false)
# endif()

if(${BOOL_USE_VCPKG})
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/berkh/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()
project(lazyimgui VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "--- BOOL_USE_VCPKG=${BOOL_USE_VCPKG}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${BOOL_USE_VCPKG})
    find_package(imgui      CONFIG REQUIRED)
    find_package(Allegro    CONFIG REQUIRED)
endif()

# Create library target
add_library(lazyimgui)

# Add source files
target_sources(lazyimgui
    PRIVATE
        src/lazyimgui.cpp
)
target_include_directories(lazyimgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/Pos2d/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


if(${BOOL_USE_VCPKG})
    target_link_libraries(lazyimgui
        PUBLIC
            imgui::imgui
            Allegro::allegro Allegro::allegro_ttf Allegro::allegro_font Allegro::allegro_main
    )
    target_link_directories(lazyimgui
        PRIVATE
            imgui::imgui
            Allegro::allegro Allegro::allegro_ttf Allegro::allegro_font Allegro::allegro_main
    )
else()
    target_sources(lazyimgui
        PRIVATE
            # ImGui core
            lib/imgui/imgui.cpp
            lib/imgui/imgui_draw.cpp
            lib/imgui/imgui_tables.cpp
            lib/imgui/imgui_widgets.cpp
            lib/imgui/misc/cpp/imgui_stdlib.cpp
            # ImGui Allegro backend
            lib/imgui/backends/imgui_impl_allegro5.cpp
            # # Date chooser addon
            # lib/imgui/addons/imguidatechooser/imguidatechooser.cpp
    )
    
    # Add include directories (PUBLIC for downstream projects)
    target_include_directories(lazyimgui
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/misc/cpp>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends>
            # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/addons/imguidatechooser>
    )
endif()


# Set public headers for installation
set_target_properties(lazyimgui PROPERTIES
    PUBLIC_HEADER "include/lazyimgui.hpp"
)

# Install all header files from include/
install(DIRECTORY include/ DESTINATION include)

# Install Pos2d header (header-only library)
install(DIRECTORY lib/Pos2d/include/ DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)


if(NOT ${BOOL_USE_VCPKG})
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

    # # Install DateChooser headers
    # install(DIRECTORY lib/imgui/addons/imguidatechooser/ DESTINATION include
    #     FILES_MATCHING PATTERN "*.h"
    # )
endif()


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
