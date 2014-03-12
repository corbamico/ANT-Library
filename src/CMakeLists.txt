cmake_minimum_required(VERSION 2.8)
project(ANT) 

#include directory from all recurse sub-directories 
file(GLOB_RECURSE headers "*.h" "*.hpp")

set (ant_INCLUDE_DIRS "")
foreach (_headerFile ${headers})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND ant_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ant_INCLUDE_DIRS)

include_directories(${ant_INCLUDE_DIRS})

#lib&demo directory
add_subdirectory(ANT_LIB)

add_subdirectory(DEMO_HRM)
add_subdirectory(DEMO_ANTFS)
