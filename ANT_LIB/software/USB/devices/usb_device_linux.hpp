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
#include <libusb.h>

class USBDeviceLinux: public USBDevice
{
public:
    USHORT GetVid() const
    {
        return usVid;
    }
    USHORT GetPid() const
    {
        return usPid;
    }

    ULONG GetSerialNumber() const
    {
        return ulSerialNumber;
    }
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

    DeviceType::Enum GetDeviceType() const
    {
        return DeviceType::LIBUSB;   //!!Or we could use a private enum!
    }

    BOOL USBReset() const
    {
        return TRUE;   //!!Should we change this to USBReEnumerate()?
    }
    USBDeviceLinux(libusb_device_handle* dev_handle)
        :usVid(0),
        usPid(0),
        ulSerialNumber(0)
    {
        szProductDescription[0]='\0';
        szSerialString[0]='\0';
        libusb_device_descriptor desc;

        if(dev_handle!=NULL)
        {
            m_dev = libusb_get_device (dev_handle);
            if(m_dev!=NULL)
            {
                if(LIBUSB_SUCCESS == libusb_get_device_descriptor (m_dev, &desc))
                {
                    usVid = desc.idVendor;
                    usPid = desc.idProduct;
                    ulSerialNumber = desc.iSerialNumber;
                }
                libusb_get_string_descriptor_ascii(dev_handle,desc.iProduct,szProductDescription,USB_MAX_STRLEN-1);
                libusb_get_string_descriptor_ascii(dev_handle,desc.iSerialNumber,szSerialString,USB_MAX_STRLEN-1);
            }
        }
    }
    USBDeviceLinux(const USBDeviceLinux& clDevice_)
    {
       m_dev = clDevice_.m_dev;
       usVid = clDevice_.usVid;
       usPid = clDevice_.usPid;
       ulSerialNumber = clDevice_.ulSerialNumber;
       STRNCPY((char*)szProductDescription, (char*)clDevice_.szProductDescription, sizeof(szProductDescription));
       memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));

       return;
    }

    USBDeviceLinux& operator=(const USBDeviceLinux& clDevice_)
    {
       if(this == &clDevice_)
          return *this;

       m_dev = clDevice_.m_dev;
       usVid = clDevice_.usVid;
       usPid = clDevice_.usPid;
       ulSerialNumber = clDevice_.ulSerialNumber;
       STRNCPY((char*)szProductDescription, (char*)clDevice_.szProductDescription, sizeof(szProductDescription));
       memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));

       return *this;
    }

private:


    USHORT usVid;
    USHORT usPid;
    ULONG  ulSerialNumber;

    UCHAR szProductDescription[USB_MAX_STRLEN];
    UCHAR szSerialString[USB_MAX_STRLEN];

private:
    friend class USBDeviceHandleLinux;
    libusb_device			*m_dev;

};


//TODO:implement usb device in linux

#endif //defined(DSI_TYPES_LINUX)
#endif
