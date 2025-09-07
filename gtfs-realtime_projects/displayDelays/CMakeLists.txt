cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/berkh/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(displayDelays_gtfs_proj)

set(CMAKE_CXX_STANDARD 17)

# find_package(implot     CONFIG REQUIRED)
find_package(Allegro    CONFIG REQUIRED)
find_package(lazyimgui  CONFIG REQUIRED)

set(DIR_REPO_ROOT "${CMAKE_SOURCE_DIR}/../..")
set(DIR_REPO_LIBS "${DIR_REPO_ROOT}/lib")


add_executable(${PROJECT_NAME}
    src/main.cpp
)


target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${DIR_REPO_LIBS}/Pos2d/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${DIR_REPO_LIBS}/GTFS-RT__TripUpdate_DebugString_parser/include")

target_link_libraries(${PROJECT_NAME} PUBLIC lazyimgui::lazyimgui)
target_link_libraries(${PROJECT_NAME} PRIVATE Allegro::allegro Allegro::allegro_ttf Allegro::allegro_font Allegro::allegro_main)
