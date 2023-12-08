cmake_minimum_required(VERSION 3.14)

project(cloud-storage-server LANGUAGES C)

include_directories(include)

file(GLOB SOURCES "src/*c")

add_executable(server ${SOURCES})