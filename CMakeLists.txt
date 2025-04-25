cmake_minimum_required(VERSION 4.0.1)
project(thorfinn VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

set(SOURCE_FILES
    src/main.cpp
    src/config.cpp
    src/pipeline.cpp
    src/file_watcher.cpp
)

include_directories(include)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master # since latest release is not supporting cmake3.5+
)
FetchContent_MakeAvailable(yaml-cpp)
include_directories(${yaml-cpp_SOURCE_DIR}/include)

FetchContent_Declare(
    libssh2
    GIT_REPOSITORY https://github.com/libssh2/libssh2.git
    GIT_TAG        libssh2-1.11.1
)
FetchContent_MakeAvailable(libssh2)

include_directories(${libssh2_SOURCE_DIR}/include)
add_executable(thorfinn ${SOURCE_FILES})

target_link_libraries(thorfinn
    yaml-cpp::yaml-cpp
    libssh2::libssh2
)

# for installation, optional
# install(TARGETS thorfinn DESTINATION bin)
