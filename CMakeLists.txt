cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(WebServer src/main.cpp
        src/http_request.cpp
        src/http_response.cpp
        src/server.cpp)

find_package(Threads REQUIRED)
target_link_libraries(WebServer PRIVATE Threads::Threads)
