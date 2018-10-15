include_directories(${CMAKE_SOURCE_DIR}/libs/libStreamBase)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(StreamClient StreamClient.cpp)

target_link_libraries(StreamClient debug StreamBase.lib)
target_link_libraries(StreamClient optimized StreamBase.lib)