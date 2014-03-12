file(GLOB_RECURSE  source_file  "*.c" "*.cpp")

# Should add ../ANT_DLL/*.cpp file into static library
set(source_dll ${ANT_SOURCE_DIR}/ANT_DLL/ant.cpp)
list (APPEND source_file ${source_dll})

add_library (ant ${source_file})
