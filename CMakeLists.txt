cmake_minimum_required(VERSION 3.27)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(WebServer src/main.cpp
        src/http_request.h
        src/usings.h
        src/http_response.h)
