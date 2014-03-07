/*
 * usb_device_handle_linux.cpp
 *
 * Copyright 2014 corbamico <corbamico@163.com>
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "types.h"
#if defined(DSI_TYPES_LINUX)


#include "macros.h"
#include "usb_device_list.hpp"
#include "usb_device_linux.hpp"

using namespace std;
#include "usb_device_handle.hpp"

#include <libusb-1.0/libusb.h>

USBError::Enum get_USBError_by_libusb(int libusb_err)
{
    switch(libusb_err)
    {
    case LIBUSB_SUCCESS:
        return USBError::NONE;
    case LIBUSB_ERROR_TIMEOUT:
        return USBError::TIMED_OUT;
    case LIBUSB_ERROR_NO_DEVICE:
        return USBError::DEVICE_GONE;
    default:
        return USBError::FAILED;
    }
}

class dummyLibusb
{
public:
    libusb_context *ctx;
    dummyLibusb()
    {
        libusb_init(&ctx);

#if defined(DEBUG_FILE)
        libusb_set_debug(ctx,3);
#endif

    }
    ~dummyLibusb()
    {
        libusb_exit(ctx);
    }
};
static dummyLibusb g_libusb;


///TODO:implement usb device in linux
class USBDeviceHandleLinux: public USBDeviceHandle
{
public:
    USBError::Enum Write(void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_)
    {
        //0xcf0f,0x1008 support LIBUSB_TRANSFER_TYPE_BULK
        int result = libusb_bulk_transfer(m_DevHandle,0x01|LIBUSB_ENDPOINT_OUT,(unsigned char * )pvData_,(int)ulSize_,(int * )&ulBytesWritten_,15000);
        return get_USBError_by_libusb(result);
    }
    USBError::Enum Read(void* pvData_, ULONG ulSize_, ULONG& ulBytesRead_, ULONG ulWaitTime_)
    {
        int result = libusb_bulk_transfer(m_DevHandle,0x01|LIBUSB_ENDPOINT_IN,(unsigned char * )pvData_,(int)ulSize_,(int * )&ulBytesRead_,1300);
        return get_USBError_by_libusb(result );
    }
    const USBDevice& GetDevice()
    {
        return clDevice;
    }

    //get all ANT devices, put it into static clDeviceList
    static void  GetAllANTDevices()
    {
        clDeviceList.Clear();

        //We only hold ANT device vid=0x0fcf,pid=0x1008

#define USB_ANT_STICK2m_PID       ((USHORT)0x1009)

        //try open pid=0x1008,0x1009
        libusb_device_handle* dev_handle = libusb_open_device_with_vid_pid(g_libusb.ctx,USB_ANT_STICK_VID,USB_ANT_STICK2_PID);
        if(dev_handle!=NULL)
        {
            clDeviceList.Add(USBDeviceLinux(dev_handle));
            //we should close it before we real need
            libusb_close(dev_handle);
        }
        else if(dev_handle = libusb_open_device_with_vid_pid(g_libusb.ctx,USB_ANT_STICK_VID,USB_ANT_STICK2m_PID))
        {
            clDeviceList.Add(USBDeviceLinux(dev_handle));
            //we should close it before we real need
            libusb_close(dev_handle);
        }
    }

    //we should setconfiguration,claiminterferace here
    static BOOL Open(const USBDeviceLinux& clDevice_, USBDeviceHandleLinux*& pclDeviceHandle_)
    {
        int result;
        libusb_device_handle* dev_handle = NULL;

        pclDeviceHandle_ = NULL;

        {
            ///TODO:fixbug Should release/close dev, if call libusb failed,
            if(LIBUSB_SUCCESS==libusb_open(clDevice_.m_dev,&dev_handle))
            {

                pclDeviceHandle_ = new USBDeviceHandleLinux(dev_handle,clDevice_);
                result = libusb_set_configuration(dev_handle,1);

                if (LIBUSB_SUCCESS!=result)
                {
                    libusb_close(dev_handle);
                    pclDeviceHandle_ = NULL;
                    return FALSE;
                }

                result = libusb_claim_interface(dev_handle,0);
                if (LIBUSB_SUCCESS!=result)
                {
                    libusb_close(dev_handle);
                    pclDeviceHandle_ = NULL;
                    return FALSE;
                }

                ///TODO bug
                ///I don't known why libusb_close()->unref->free(dev) always call double free error
                ///so just keep ref++ avoid free(dev), maybe bug/leak here.
                libusb_ref_device(clDevice_.m_dev);
                return TRUE;
            }
        }
        return FALSE;
    }

    static BOOL Close(USBDeviceHandleLinux*& pclDeviceHandle_, BOOL bReset_ = FALSE)
    {
        if(pclDeviceHandle_ == NULL)
            return FALSE;

        if(pclDeviceHandle_->m_DevHandle)
        {

            libusb_release_interface(pclDeviceHandle_->m_DevHandle,0);
            libusb_close(pclDeviceHandle_->m_DevHandle);
            pclDeviceHandle_->m_DevHandle=NULL;
        }

        ///TODO BUG
        ///*** Error in `./demo_dylib': double free or corruption (!prev): 0x0001e220 ***
        delete pclDeviceHandle_;
        pclDeviceHandle_ = NULL;
        return TRUE;
    }

protected:


    USBDeviceHandleLinux(libusb_device_handle* dev_handle_,const USBDeviceLinux& dev)
        :clDevice(dev)
    {
        m_DevHandle=dev_handle_;
    }

    virtual ~USBDeviceHandleLinux()
    {

    }

private:
    USBDeviceLinux 			clDevice;
    libusb_device_handle 	*m_DevHandle;      ///device_hanlde for write,

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

    if(USBDeviceHandleLinux::clDeviceList.GetSize()>=1)
    {
        pDevice = USBDeviceHandleLinux::clDeviceList.GetAddress(0) ;
        clList.Add(pDevice);
    }
    return clList;
}

const ANTDeviceList USBDeviceHandle::GetAvailableDevices(ULONG ulDeviceTypeField_)
{
    ANTDeviceList clList;
    const USBDeviceLinux* pDevice=NULL;

    if(USBDeviceHandleLinux::clDeviceList.GetSize()>=1)
    {
        pDevice = USBDeviceHandleLinux::clDeviceList.GetAddress(0) ;
        clList.Add(pDevice);
    }
    return clList;
}


BOOL USBDeviceHandle::Open(const USBDevice& clDevice_, USBDeviceHandle*& pclDeviceHandle_, ULONG ulBaudRate_)
{
    BOOL bSuccess;
    pclDeviceHandle_ = NULL;
    switch(clDevice_.GetDeviceType())
    {
    case DeviceType::LIBUSB:
    {
        const USBDeviceLinux& clDeviceLinux = dynamic_cast<const USBDeviceLinux&>(clDevice_);

        USBDeviceHandleLinux* pclDeviceHandleLinux;
        bSuccess = USBDeviceHandleLinux::Open(clDeviceLinux, pclDeviceHandleLinux);

        //pclDeviceHandle_ = dynamic_cast<USBDeviceHandle*>(pclDeviceHandleSiIOKit);
        pclDeviceHandle_ = pclDeviceHandleLinux;
        break;
    }
    default:
    {
        pclDeviceHandle_ = NULL;
        bSuccess = FALSE;
        break;
    }
    }
    return bSuccess;
}

BOOL USBDeviceHandle::Close(USBDeviceHandle*& pclDeviceHandle_, BOOL bReset_)
{
    if(pclDeviceHandle_ == NULL)
        return FALSE;

    //dynamic_cast does not handle *& types

    BOOL bSuccess;
    switch(pclDeviceHandle_->GetDevice().GetDeviceType())
    {
    case DeviceType::LIBUSB:
    {
        USBDeviceHandleLinux* pclDeviceHandleLinux = reinterpret_cast<USBDeviceHandleLinux*>(pclDeviceHandle_);
        bSuccess = USBDeviceHandleLinux::Close(pclDeviceHandleLinux, bReset_);

        pclDeviceHandle_ = pclDeviceHandleLinux;
        break;
    }
    default:
    {
        pclDeviceHandle_ = NULL;
        bSuccess = FALSE;
        break;
    }
    }

    return bSuccess;
}




#endif //defined(DSI_TYPES_LINUX)
