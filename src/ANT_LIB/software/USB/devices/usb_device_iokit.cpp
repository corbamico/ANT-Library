/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#include "types.h"
#if defined(DSI_TYPES_MACINTOSH)

#include "usb_device_iokit.hpp"

#include "macros.h"

#include "iokit_device.hpp"


USBDeviceIOKit::USBDeviceIOKit(const IOKitDevice& clIOKitDevice_)
:
   clDevice(clIOKitDevice_),
   usVid(clIOKitDevice_.usVid),
   usPid(clIOKitDevice_.usPid),
   ulSerialNumber(clIOKitDevice_.ulSerialNumber)
{
   //initialize array
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';

   STRNCPY((char*)szProductDescription, clIOKitDevice_.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, clIOKitDevice_.szSerialString, sizeof(szSerialString));

   return;
}

USBDeviceIOKit::USBDeviceIOKit(const USBDeviceIOKit& clDevice_)
:
   clDevice(clDevice_.clDevice),
   usVid(clDevice_.usVid),
   usPid(clDevice_.usPid),
   ulSerialNumber(clDevice_.ulSerialNumber)
{
   //initialize array
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';

   STRNCPY((char*)szProductDescription, (char*)clDevice_.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));
   return;
}


USBDeviceIOKit& USBDeviceIOKit::operator=(const USBDeviceIOKit& clDevice_)
{
   if(this == &clDevice_)
      return *this;

   clDevice = clDevice_.clDevice;  //!!Need to retain and release this!!!  Also for vcp!!!
   usVid = clDevice_.usVid;
   usPid = clDevice_.usPid;
   ulSerialNumber = clDevice_.ulSerialNumber;

   STRNCPY((char*)szProductDescription, (char*)clDevice_.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));
   return *this;
}


BOOL USBDeviceIOKit::USBReset() const
{
/*  Removed while evaluating long term stability issues
   if(clDevice.ppstDeviceInterface == NULL)
      return FALSE;

   (*clDevice.ppstDeviceInterface)->USBDeviceOpen(clDevice.ppstDeviceInterface);
   
   if( (*clDevice.ppstDeviceInterface)->USBDeviceReEnumerate(clDevice.ppstDeviceInterface, 0) != kIOReturnSuccess)
      return FALSE;
*/   
   return TRUE;
}

BOOL USBDeviceIOKit::GetProductDescription(UCHAR* pucProductDescription_, USHORT usBufferSize_) const
{
   return(STRNCPY((char*) pucProductDescription_, (char*) szProductDescription, usBufferSize_));
}

BOOL USBDeviceIOKit::GetSerialString(UCHAR* pucSerialString_, USHORT usBufferSize_) const
{
   if(sizeof(szSerialString) > usBufferSize_)
   {
      memcpy(pucSerialString_, szSerialString, usBufferSize_);
      return FALSE;
   }

   memcpy(pucSerialString_, szSerialString, sizeof(szSerialString));
   return TRUE;
}

BOOL USBDeviceIOKit::UpdateSerialString(UCHAR* pucSerialString_, USHORT usLength_)
{
   if(sizeof(szSerialString) < usLength_)
   {
      memcpy(szSerialString, pucSerialString_, sizeof(szSerialString));
      return FALSE;
   }

   memcpy(szSerialString, pucSerialString_, usLength_);
   return TRUE;
}

DeviceType::Enum USBDeviceIOKit::GetDeviceType() const
{
   switch (usPid)
   {
      case USB_ANT_STICK_PID:
      case USB_ANT_DEV_BOARD_PID:
         return DeviceType::SI_LABS_IOKIT;
      default:
         break;
   }
   return DeviceType::IO_KIT;
}

#endif //defined(DSI_TYPES_MACINTOSH)
