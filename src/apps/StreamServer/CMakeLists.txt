include_directories(${CMAKE_SOURCE_DIR}/libs/libStreamBase)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(StreamServer StreamServer.cpp)

target_link_libraries(StreamServer debug StreamBase.lib)
target_link_libraries(StreamServer optimized StreamBase.lib)