/*
 * usb_device_handle_linux.cpp
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

#include "types.h"
#if defined(DSI_TYPES_LINUX)


#include "macros.h"
#include "usb_device_list.hpp"
#include "usb_device_linux.hpp"

using namespace std;
#include "usb_device_handle.hpp"

BOOL USBDeviceHandle::CopyANTDevice(const USBDevice*& pclUSBDeviceCopy_, const USBDevice* pclUSBDeviceOrg_)
{
    if (pclUSBDeviceOrg_ == NULL)
        return FALSE;
   
    if (pclUSBDeviceCopy_ != NULL) 
        return FALSE;
    return TRUE;
}

//!!Just here temporarily until we get ANTDeviceList to do it.
const ANTDeviceList USBDeviceHandle::GetAllDevices(ULONG ulDeviceTypeField_)
{
    ANTDeviceList clDeviceList;
    return clDeviceList;
}

const ANTDeviceList USBDeviceHandle::GetAvailableDevices(ULONG ulDeviceTypeField_)
{
   ANTDeviceList clDeviceList;
  return clDeviceList;
}

BOOL USBDeviceHandle::Open(const USBDevice& clDevice_, USBDeviceHandle*& pclDeviceHandle_, ULONG ulBaudRate_)
{
    //return FALSE;
    pclDeviceHandle_ = NULL;
    return TRUE;
}

BOOL USBDeviceHandle::Close(USBDeviceHandle*& pclDeviceHandle_, BOOL bReset_)
{
   if(pclDeviceHandle_ == NULL)
      return FALSE;
    return TRUE;
}

//TODO:implement usb device in linux
class USBDeviceHandleLinux: public USBDeviceHandle
{
public:
    USBError::Enum Write(void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_) 
    {
        return USBError::NONE;
    }
    USBError::Enum Read(void* pvData_, ULONG ulSize_, ULONG& ulBytesRead_, ULONG ulWaitTime_)
    {
        return USBError::NONE;
    }
    const USBDevice& GetDevice()
    {
        return clDevice;
    }
private:
    USBDeviceLinux clDevice;
    static USBDeviceList<const USBDeviceLinux> clDeviceList;
};

USBDeviceList<const USBDeviceLinux> USBDeviceHandleLinux::clDeviceList;


#endif //defined(DSI_TYPES_LINUX)
