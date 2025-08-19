cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/berkh/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(gtfs_realtime_test)

find_package(protobuf CONFIG REQUIRED)
set(DIR_ROOT "..")
set(DIR_PROTO_GTFS_REALTIME "${DIR_ROOT}/protobuf_files/gtfs-realtime/build/gen")


add_executable(${PROJECT_NAME}
    src/main.cpp
    ${DIR_PROTO_GTFS_REALTIME}/gtfs-realtime.pb.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_PROTO_GTFS_REALTIME})
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
