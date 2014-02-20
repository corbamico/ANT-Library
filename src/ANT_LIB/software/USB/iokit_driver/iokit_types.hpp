 /*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#ifndef IOKIT_TYPES_HPP
#define IOKIT_TYPES_HPP

#include "dsi_convert.h"
#include <IOKit/IOCFBundle.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>


struct IOKitError
{
   enum Enum
   {
      //Success (no error)
      SUCCESS = 0,

      //Input/output error
      IO = -1,

      //Invalid parameter
      INVALID_PARAM = -2,

      //Access denied (insufficient permissions)
      ACCESS = -3,

      //No such device (it may have been disconnected)
      NO_DEVICE = -4,

      //Entity not found
      NOT_FOUND = -5,

      //Resource busy
      BUSY = -6,

      //Operation timed out
      TIMEOUT = -7,

      //Overflow
      OVERFLOWED = -8,

      //Pipe error
      PIPE = -9,

      //System call interrupted (perhaps due to signal)
      INTERRUPTED = -10,

      //Insufficient memory
      NO_MEM = -11,

      //Operation not supported or unimplemented on this platform
      NOT_SUPPORTED = -12,

      //Other error
      OTHER = -99
   };

   private: IOKitError();
};

static IOKitError::Enum darwin_to_libusb(int result)
{
   switch (result)
   {
      case kIOReturnSuccess:
         return IOKitError::SUCCESS;
      case kIOReturnNotOpen:
      case kIOReturnNoDevice:
         return IOKitError::NO_DEVICE;
      case kIOReturnExclusiveAccess:
         return IOKitError::ACCESS;
      case kIOUSBPipeStalled:
         return IOKitError::PIPE;
      case kIOReturnBadArgument:
         return IOKitError::INVALID_PARAM;
      case kIOUSBTransactionTimeout:
         return IOKitError::TIMEOUT;
      case kIOReturnNotResponding:
      case kIOReturnAborted:
      case kIOReturnError:
      case kIOUSBNoAsyncPortErr:
      default:
         return IOKitError::OTHER;
   }
}


/** \ingroup desc
 * A structure representing the standard USB device descriptor. This
 * descriptor is documented in section 9.6.1 of the USB 2.0 specification.
 * All multiple-byte fields are represented in host-endian format.
 */
struct usb_device_descriptor
{
	/** Size of this descriptor (in bytes) */
	uint8_t  bLength;

	/** Descriptor type. Will have value
	 * \ref libusb_descriptor_type::LIBUSB_DT_DEVICE LIBUSB_DT_DEVICE in this
	 * context. */
	uint8_t  bDescriptorType;

	/** USB specification release number in binary-coded decimal. A value of
	 * 0x0200 indicates USB 2.0, 0x0110 indicates USB 1.1, etc. */
	uint16_t bcdUSB;

	/** USB-IF class code for the device. See \ref libusb_class_code. */
	uint8_t  bDeviceClass;

	/** USB-IF subclass code for the device, qualified by the bDeviceClass
	 * value */
	uint8_t  bDeviceSubClass;

	/** USB-IF protocol code for the device, qualified by the bDeviceClass and
	 * bDeviceSubClass values */
	uint8_t  bDeviceProtocol;

	/** Maximum packet size for endpoint 0 */
	uint8_t  bMaxPacketSize0;

	/** USB-IF vendor ID */
	uint16_t idVendor;

	/** USB-IF product ID */
	uint16_t idProduct;

	/** Device release number in binary-coded decimal */
	uint16_t bcdDevice;

	/** Index of string descriptor describing manufacturer */
	uint8_t  iManufacturer;

	/** Index of string descriptor describing product */
	uint8_t  iProduct;

	/** Index of string descriptor containing device serial number */
	uint8_t  iSerialNumber;

	/** Number of possible configurations */
	uint8_t  bNumConfigurations;
};


/* Asking for the zero'th index is special - it returns a string
 * descriptor that contains all the language IDs supported by the device.
 * Typically there aren't many - often only one. The language IDs are 16
 * bit numbers, and they start at the third byte in the descriptor. See
 * USB 2.0 specification section 9.6.7 for more information. */
struct string_descriptor  //Some devices choke on size > 255
{
   UCHAR length;
   UCHAR descriptor_type;

   union
   {
      //index zero
      struct
      {
         UCHAR first_byte;
         UCHAR second_byte;
      } lang_id[64];

      //other index's
      UCHAR string[128];
   };
};



/* IOUSBInterfaceInferface */
#if defined (kIOUSBInterfaceInterfaceID300)

#define usb_interface_t IOUSBInterfaceInterface300
#define InterfaceInterfaceID kIOUSBInterfaceInterfaceID300
#define InterfaceVersion 300

#elif defined (kIOUSBInterfaceInterfaceID245)

#define usb_interface_t IOUSBInterfaceInterface245
#define InterfaceInterfaceID kIOUSBInterfaceInterfaceID245
#define InterfaceVersion 245

#elif defined (kIOUSBInterfaceInterfaceID220)

#define usb_interface_t IOUSBInterfaceInterface220
#define InterfaceInterfaceID kIOUSBInterfaceInterfaceID220
#define InterfaceVersion 220

#elif defined (kIOUSBInterfaceInterfaceID197)

#define usb_interface_t IOUSBInterfaceInterface197
#define InterfaceInterfaceID kIOUSBInterfaceInterfaceID197
#define InterfaceVersion 197

#elif defined (kIOUSBInterfaceInterfaceID190)

#define usb_interface_t IOUSBInterfaceInterface190
#define InterfaceInterfaceID kIOUSBInterfaceInterfaceID190
#define InterfaceVersion 190

#elif defined (kIOUSBInterfaceInterfaceID182)

#define usb_interface_t IOUSBInterfaceInterface182
#define InterfaceInterfaceID kIOUSBInterfaceInterfaceID182
#define InterfaceVersion 182

#else

#error "IOUSBFamily is too old. Please upgrade your OS"

#endif

/* IOUSBDeviceInterface */
#if defined (kIOUSBDeviceInterfaceID320)

#define usb_device_t    IOUSBDeviceInterface320
#define DeviceInterfaceID kIOUSBDeviceInterfaceID320
#define DeviceVersion 320

#elif defined (kIOUSBDeviceInterfaceID300)

#define usb_device_t    IOUSBDeviceInterface300
#define DeviceInterfaceID kIOUSBDeviceInterfaceID300
#define DeviceVersion 300

#elif defined (kIOUSBDeviceInterfaceID245)

#define usb_device_t    IOUSBDeviceInterface245
#define DeviceInterfaceID kIOUSBDeviceInterfaceID245
#define DeviceVersion 245

#elif defined (kIOUSBDeviceInterfaceID197)

#define usb_device_t    IOUSBDeviceInterface197
#define DeviceInterfaceID kIOUSBDeviceInterfaceID197
#define DeviceVersion 197

#elif defined (kIOUSBDeviceInterfaceID187)

#define usb_device_t    IOUSBDeviceInterface187
#define DeviceInterfaceID kIOUSBDeviceInterfaceID187
#define DeviceVersion 187

#elif defined (kIOUSBDeviceInterfaceID182)

#define usb_device_t    IOUSBDeviceInterface182
#define DeviceInterfaceID kIOUSBDeviceInterfaceID182
#define DeviceVersion 182

#else

#error "IOUSBFamily is too old. Please upgrade your OS"

#endif

typedef IOCFPlugInInterface* io_cf_plugin_ref_t;
typedef IONotificationPortRef io_notification_port_t;

/** \def libusb_cpu_to_le16
 * \ingroup misc
 * Convert a 16-bit value from host-endian to little-endian format. On
 * little endian systems, this function does nothing. On big endian systems,
 * the bytes are swapped.
 * \param x the host-endian value to convert
 * \returns the value in little-endian byte order
 */
#define libusb_cpu_to_le16(x) ({ \
   UCHAR ucByte0, ucByte1; \
   Convert_USHORT_To_Bytes(x, &ucByte1, &ucByte0); \
   Convert_Bytes_To_USHORT(ucByte1, ucByte0); \
})


/** \def libusb_le16_to_cpu
 * \ingroup misc
 * Convert a 16-bit value from little-endian to host-endian format. On
 * little endian systems, this function does nothing. On big endian systems,
 * the bytes are swapped.
 * \param x the little-endian value to convert
 * \returns the value in host-endian byte order
 */
#define libusb_le16_to_cpu libusb_cpu_to_le16

/** \def libusb_cpu_to_le32
 * \ingroup misc
 * Convert a 32-bit value from host-endian to little-endian format. On
 * little endian systems, this function does nothing. On big endian systems,
 * the bytes are swapped.
 * \param x the host-endian value to convert
 * \returns the value in little-endian byte order
 */
#define libusb_cpu_to_le32(x) ({ \
   UCHAR ucByte0, ucByte1, ucByte2, ucByte3; \
   Convert_ULONG_To_Bytes(x, &ucByte3, &ucByte2, &ucByte1, &ucByte0); \
   Convert_Bytes_To_ULONG(ucByte3, ucByte2, ucByte1, ucByte0); \
})

#endif //IOKIT_TYPES_HPP
