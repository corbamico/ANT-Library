bin_PROGRAMS = demo_antfs


LIBANTFS_INC = \
	-I../ANT_LIB/inc \
	-I../ANT_LIB/software/ANTFS \
	-I../ANT_LIB/software/serial \
	-I../ANT_LIB/software/serial/device_management \
	-I../ANT_LIB/software/system \
	-I../ANT_LIB/software/usb 
	
demo_antfs_SOURCES = demo_antfs.cpp
demo_antfs_CXXFLAGS = ${LIBANTFS_INC}
demo_antfs_LDADD= ../ANT_LIB/libantfs.a
