cmake_minimum_required(VERSION 3.20)
project(Coursework_PC LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
include(FetchContent)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(Crow)
add_executable(Coursework_PC main.cpp)
target_link_libraries(Coursework_PC PRIVATE Crow::Crow Threads::Threads)