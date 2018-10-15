cmake_minimum_required(VERSION 3.6.2)
project(StreamBase)

message(STATUS "Setting MSVC flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../build/output/Windows/Win32/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../build/output/Windows/Win32/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../build/output/Windows/Win32/bin")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/libStreamBase)
add_subdirectory(${CMAKE_SOURCE_DIR}/apps/StreamClient)
add_subdirectory(${CMAKE_SOURCE_DIR}/apps/StreamServer)

add_dependencies(StreamClient StreamBase)
add_dependencies(StreamServer StreamBase)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT StreamClient)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT StreamServer)