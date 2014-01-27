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

#include <libusb.h>

class dummyLibusb
{
public:
	libusb_context *ctx;
	dummyLibusb(){libusb_init(&ctx);}
	~dummyLibusb(){libusb_exit(ctx);}
};
static dummyLibusb g_libusb;


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
    static void  GetAllANTDevices(){
		clDeviceList.Clear();
		
		//We only hold ANT device vid=0xfc0f,pid=0x1008
		libusb_device_handle* dev_handle = libusb_open_device_with_vid_pid(g_libusb.ctx,USB_ANT_STICK_VID,USB_ANT_STICK2_PID);
		clDeviceList.Add(USBDeviceLinux(dev_handle));
	}
private:
    USBDeviceLinux clDevice;

public:
    static USBDeviceList<const USBDeviceLinux> clDeviceList;
};

USBDeviceList<const USBDeviceLinux> USBDeviceHandleLinux::clDeviceList;


BOOL USBDeviceHandle::CopyANTDevice(const USBDevice*& pclUSBDeviceCopy_, const USBDevice* pclUSBDeviceOrg_)
{
    if (pclUSBDeviceOrg_ == NULL)
        return FALSE;
   
    if (pclUSBDeviceCopy_ != NULL) 
        return FALSE;
    return FALSE;
    //no implement
    //return TRUE;
}

//!!Just here temporarily until we get ANTDeviceList to do it.
const ANTDeviceList USBDeviceHandle::GetAllDevices(ULONG ulDeviceTypeField_)
{
	ANTDeviceList clList;
	const USBDeviceLinux* pDevice=NULL;
    //TODO: 
    //We only hold ANT device vid=0xfc0f,pid=0x1008
    //libusb_device_handle* dev_handle = libusb_open_device_with_vid_pid(ctx,USB_ANT_STICK_VID,USB_ANT_STICK2_PID);
    //we only get first USB device we found.
    USBDeviceHandleLinux::GetAllANTDevices();
    
    if(USBDeviceHandleLinux::clDeviceList.GetSize()>=1){
    	pDevice = USBDeviceHandleLinux::clDeviceList.GetAddress(0) ;
		clList.Add(pDevice);
	}
    return clList;
}

const ANTDeviceList USBDeviceHandle::GetAvailableDevices(ULONG ulDeviceTypeField_)
{
	ANTDeviceList clList;
	const USBDeviceLinux* pDevice=NULL;

    if(USBDeviceHandleLinux::clDeviceList.GetSize()>=1){
    	pDevice = USBDeviceHandleLinux::clDeviceList.GetAddress(0) ;
		clList.Add(pDevice);
	}
    return clList;
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




#endif //defined(DSI_TYPES_LINUX)
