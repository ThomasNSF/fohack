cmake_minimum_required(VERSION 3.8)

project(Fallout VERSION 1.0)

find_package(Curses REQUIRED)
find_package(Boost COMPONENTS program_options exception system REQUIRED)

add_subdirectory(fallout)
add_subdirectory(screensave)
