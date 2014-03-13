link_directories (${ANT_BINARY_DIR}/ANT_LIB)
ADD_EXECUTABLE(demo_antfs demo_antfs.cpp)
target_link_libraries (demo_antfs ant pthread usb-1.0 glog) 
