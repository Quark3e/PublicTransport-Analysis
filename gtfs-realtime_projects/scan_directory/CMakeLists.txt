cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/berkh/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(scan_directory__gtfs_proj)

# set(DIR_PROTOBUF )
find_package(protobuf CONFIG REQUIRED)
#    PATHS "C:/Users/berkh/Libraries/vcpkg/packages/protobuf_x64-windows/share/protobuf"
#)

# set(CMAKE_TOOLCHAIN_FILE "C:/Users/berkh/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(DIR_PROTO_GTFS_REALTIME "C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/protobuf_files/gtfs-realtime/build/gen")


add_executable(${PROJECT_NAME}
    src/main.cpp
    ${DIR_PROTO_GTFS_REALTIME}/gtfs-realtime.pb.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR} include")
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_PROTO_GTFS_REALTIME})

target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
