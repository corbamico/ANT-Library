/*
 * usb_device_linux.cpp
 * 
 * Copyright 2014 corbamico <corbamico@163.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#ifndef USB_DEVICE_LINUX_HPP
#define USB_DEVICE_LINUX_HPP
#include "types.h"

#if defined(DSI_TYPES_LINUX)

#include "macros.h"
#include "usb_device.hpp"
#include <string.h>

class USBDeviceLinux: public USBDevice
{
  public:
   USHORT GetVid() const { return usVid; }
   USHORT GetPid() const { return usPid; }

   ULONG GetSerialNumber() const { return ulSerialNumber; }
   BOOL GetProductDescription(UCHAR* pucProductDescription_, USHORT usBufferSize_) const //guaranteed to be null-terminated; pointer is valid until device is released
    {
        return(STRNCPY((char*) pucProductDescription_, (char*) szProductDescription, usBufferSize_));
    }

   BOOL GetSerialString(UCHAR* pucSerialString_, USHORT usBufferSize_) const
   {
        if(sizeof(szSerialString) > usBufferSize_)
        {
          memcpy(pucSerialString_, szSerialString, usBufferSize_);
          return FALSE;
        }

        memcpy(pucSerialString_, szSerialString, sizeof(szSerialString));
        return TRUE;
   }

   DeviceType::Enum GetDeviceType() const {return DeviceType::LIBUSB;} //!!Or we could use a private enum!
   
   BOOL USBReset() const {return TRUE;} //!!Should we change this to USBReEnumerate()?
   USBDeviceLinux(){}
   
private:

   
   USHORT usVid;
   USHORT usPid;
   ULONG ulSerialNumber;

   UCHAR szProductDescription[USB_MAX_STRLEN];
   UCHAR szSerialString[USB_MAX_STRLEN];
};


//TODO:implement usb device in linux

    #endif //defined(DSI_TYPES_LINUX)
#endif
