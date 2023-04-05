cmake_minimum_required(VERSION 3.23)
project(http_server C)

set(CMAKE_C_STANDARD 17)

add_executable(http_server server.c client.c)
