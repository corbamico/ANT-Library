noinst_LIBRARIES = libantfs.a

LIBANTFS_INC = @LIBUSB_CFLAGS@\
	-Iinc \
	-Icommon \
	-Isoftware/ANTFS \
	-Isoftware/serial \
	-Isoftware/serial/device_management \
	-Isoftware/system \
	-Isoftware/USB \
	-Isoftware/USB/devices \
	-Isoftware/USB/device_handles \
	-Isoftware/USB/iokit_driver \
	-I../ANT_DLL
	
	

LIBANTFS_SRC = \
	./common/crc.c \
	./common/checksum.c \
	./software/serial/dsi_vcp_si.cpp \
	./software/serial/dsi_serial_iokit.cpp \
	./software/serial/dsi_serial.cpp \
	./software/serial/dsi_serial_generic.cpp \
	./software/serial/dsi_framer_integrated_antfs_client.cpp \
	./software/serial/dsi_framer.cpp \
	./software/serial/device_management/dsi_ant_device.cpp \
	./software/serial/dsi_framer_ant.cpp \
	./software/ANTFS/antfs_client_channel.cpp \
	./software/ANTFS/antfs_directory.c \
	./software/ANTFS/antfs_host.cpp \
	./software/ANTFS/antfs_host_channel.cpp \
	./software/USB/iokit_driver/iokit_device.cpp \
	./software/USB/iokit_driver/iokit_device_list.cpp \
	./software/USB/iokit_driver/iokit_interface.cpp \
	./software/USB/iokit_driver/iokit_device_handle.cpp \
	./software/USB/iokit_driver/iokit_transfer.cpp \
	./software/USB/devices/usb_device.cpp \
	./software/USB/devices/usb_device_linux.cpp \
	./software/USB/devices/usb_device_vcp.cpp \
	./software/USB/devices/usb_device_iokit.cpp \
	./software/USB/device_handles/usb_device_handle_iokit.cpp \
	./software/USB/device_handles/usb_device_handle_vcp.cpp \
	./software/USB/device_handles/usb_device_handle_si_iokit.cpp \
	./software/USB/device_handles/usb_device_handle_linux.cpp \
	./software/USB/device_handles/usb_device_handle_mac.cpp \
	./software/system/dsi_thread_win32.c \
	./software/system/dsi_debug.cpp \
	./software/system/macros.c \
	./software/system/dsi_convert.c \
	./software/system/dsi_timer.cpp \
	./software/system/dsi_thread_posix.c \
	../ANT_DLL/ant.cpp


libantfs_a_SOURCES = ${LIBANTFS_SRC}
AM_CPPFLAGS = ${LIBANTFS_INC} 

