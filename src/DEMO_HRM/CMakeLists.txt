link_directories (${ANT_BINARY_DIR}/ANT_LIB)
add_executable(demo_hrm demo_hrm.cpp)

target_link_libraries (demo_hrm ant pthread usb-1.0 glog) 
