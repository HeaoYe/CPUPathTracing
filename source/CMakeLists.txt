set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE source CONFIGURE_DEPENDS src/*.cpp)

add_executable(CPUPathTracing ${source})

target_include_directories(CPUPathTracing PUBLIC include)

target_link_libraries(CPUPathTracing glm)
