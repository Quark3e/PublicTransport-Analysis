cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/berkh/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(displayDelays_gtfs_proj)

set(CMAKE_CXX_STANDARD 17)


find_package(utf8cpp    CONFIG REQUIRED)
find_package(fmt        CONFIG REQUIRED)
find_package(protobuf   CONFIG REQUIRED)

set(DIR_REPO_ROOT "${CMAKE_SOURCE_DIR}/../..")
set(DIR_REPO_LIBS "${DIR_REPO_ROOT}/lib")
set(DIR_PROTO_GTFS_REALTIME "C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/protobuf_files/gtfs-realtime/build/gen")


add_executable(${PROJECT_NAME}
    src/main.cpp
    ${DIR_PROTO_GTFS_REALTIME}/gtfs-realtime.pb.cc
)

if (MSVC)
    # warning level 4
    add_compile_options(/utf-8)
else()
    # additional warnings
    add_compile_options(-Wutf-8)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_PROTO_GTFS_REALTIME})
target_include_directories(${PROJECT_NAME} PRIVATE "${DIR_REPO_LIBS}/Pos2d/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${DIR_REPO_LIBS}/GTFS-RT__TripUpdate_DebugString_parser/include")

target_link_libraries(${PROJECT_NAME} PRIVATE utf8::cpp utf8cpp::utf8cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
