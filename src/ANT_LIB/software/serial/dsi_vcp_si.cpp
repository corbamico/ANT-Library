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


#include "dsi_vcp_si.hpp"

#include "dsi_thread.h"

#include "usb_device_handle_vcp.hpp"
#include "usb_device_vcp.hpp"
#include "macros.h"

#define MAX_PATH_SIZE					((UCHAR) 255)
#define USB_ANT_BSD_NAME "ANTUSBStick.slabvcp"


//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
DSISerialSI::DSISerialSI()
:
   DSISerial(),
   pclDeviceHandle(NULL),
   pclDevice(NULL),
   hReceiveThread(NULL),
   bStopReceiveThread(TRUE),
   ucDeviceNumber(0xFF),
   ulBaud(0)
{   
   return;
}

///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
DSISerialSI::~DSISerialSI()
{
   Close();
}

namespace
{
BOOL MatchBsdName(const USBDeviceSI*const & pclDevice_)  //!!Make a member function
  {
  	if(pclDevice_ == NULL)
  		return FALSE;
     
     CFStringRef hBsdName = CFStringCreateWithCString(kCFAllocatorDefault, (const char*)(pclDevice_->GetBsdName()), kCFStringEncodingUTF8);
  	BOOL bIsMatch = CFStringFindWithOptions(hBsdName, CFSTR(USB_ANT_BSD_NAME), CFRangeMake(0,CFStringGetLength(hBsdName)), 0, NULL);
     CFRelease(hBsdName);
     return bIsMatch;
   }
}

///////////////////////////////////////////////////////////////////////
// Initializes and opens the object.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::AutoInit()
{
   Close();
	
   const USBDeviceListSI clDeviceList = USBDeviceHandleSI::GetAvailableDevices().GetSubList(MatchBsdName);
   if(clDeviceList.GetSize() == 0)
   {
      this->USBReset();
      return FALSE;
   }
   
   pclDevice = NULL;
   ucDeviceNumber = 0;
   return TRUE;
   
}


///////////////////////////////////////////////////////////////////////
// Initializes the object.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::Init(ULONG ulBaud_, UCHAR ucDeviceNumber_)
{
	Close();

   const USBDeviceListSI clDeviceList = USBDeviceHandleSI::GetAllDevices().GetSubList(MatchBsdName);
   if(clDeviceList.GetSize() <= ucDeviceNumber_)
   {
      this->USBReset();
      return FALSE;
   }

   pclDevice = NULL;
   ulBaud = ulBaud_;
   ucDeviceNumber = ucDeviceNumber_;

   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Initializes the object.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::Init(ULONG ulBaud_, const USBDeviceSI& clDevice_, UCHAR ucDeviceNumber_)
{
   Close();

   pclDevice = &clDevice_;  //!!Make copy?
   ulBaud = ulBaud_;
   ucDeviceNumber = ucDeviceNumber_;

   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Returns the device serial number.
///////////////////////////////////////////////////////////////////////
ULONG DSISerialSI::GetDeviceSerialNumber()
{
   if(pclDeviceHandle == NULL)
      return 0;
	
   return pclDeviceHandle->GetDevice().GetSerialNumber();
}

///////////////////////////////////////////////////////////////////////
// Returns the device port number.
///////////////////////////////////////////////////////////////////////
UCHAR DSISerialSI::GetDeviceNumber()
{
	return ucDeviceNumber;
}



//Error is MAX_UCHAR
UCHAR DSISerialSI::GetNumberOfDevices()  //!!Should this be a static function?
{
   return USBDeviceHandleSI::GetAvailableDevices().GetSubList(MatchBsdName).GetSize();
}

///////////////////////////////////////////////////////////////////////
// Opens port, starts receive thread.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::Open(void)
{

   // Make sure all handles are reset before opening again.
   Close();

   if (pclCallback == NULL)
      return FALSE;


   //If the user specified a device number instead of a USBDevice instance, then grab it from the list
   const USBDeviceSI* pclTempDevice = pclDevice;
   if(pclDevice == NULL)
   {
      const USBDeviceListSI clDeviceList = USBDeviceHandleSI::GetAllDevices();
      if(clDeviceList.GetSize() <= ucDeviceNumber)
         return FALSE;

      pclTempDevice = clDeviceList[ucDeviceNumber];
   }

   if(USBDeviceHandleSI::Open(*pclTempDevice, pclDeviceHandle, ulBaud) == FALSE)
   {
      pclDeviceHandle = NULL;
      Close();
      return FALSE;
   }

   
   
	//hReceiveThread keeps track if these are initialized/destroyed
    if (DSIThread_MutexInit(&stMutexCriticalSection) != DSI_THREAD_ENONE)
    {
        Close();
        return FALSE;
    }

    if (DSIThread_CondInit(&stEventReceiveThreadExit) != DSI_THREAD_ENONE)
    {
		DSIThread_MutexDestroy(&stMutexCriticalSection);
        Close();
        return FALSE;
    }
    
   bStopReceiveThread = FALSE;
    hReceiveThread = DSIThread_CreateThread(&DSISerialSI::ProcessThread, this);
    if (hReceiveThread == NULL)
    {
        DSIThread_MutexDestroy(&stMutexCriticalSection);
    	DSIThread_CondDestroy(&stEventReceiveThreadExit);
        Close();
        return FALSE;
    }

	return TRUE;
}

///////////////////////////////////////////////////////////////////////
// Closes the USB connection, kills receive thread.
///////////////////////////////////////////////////////////////////////
void DSISerialSI::Close(BOOL bReset_)
{

    if(hReceiveThread)
    {
    	DSIThread_MutexLock(&stMutexCriticalSection);
        bStopReceiveThread = TRUE;

        if (DSIThread_CondTimedWait(&stEventReceiveThreadExit, &stMutexCriticalSection, 3000) == DSI_THREAD_ETIMEDOUT)
        {
            // We were unable to stop the thread normally.
            DSIThread_DestroyThread(hReceiveThread);
        }
        DSIThread_MutexUnlock(&stMutexCriticalSection);

        DSIThread_ReleaseThreadID(hReceiveThread);
        hReceiveThread = NULL;

		DSIThread_MutexDestroy(&stMutexCriticalSection);
        DSIThread_CondDestroy(&stEventReceiveThreadExit);
    }



   if(pclDeviceHandle)
   {
      USBDeviceHandleSI::Close(pclDeviceHandle, bReset_);

      if(bReset_)
         DSIThread_Sleep(1750);           //Stall to allow the device to reset, trying to reopen the driver too soon can cause bad things to happen.

      pclDeviceHandle = NULL;
      
      //Set device number to an invalid state
      ucDeviceNumber = MAX_UCHAR;
   }
   
   return;
}

///////////////////////////////////////////////////////////////////////
// Writes usSize_ bytes to USB, returns TRUE if successful.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::WriteBytes(void *pvData_, USHORT usSize_)
{

	ULONG ulActualWritten = 0;

   if(pclDeviceHandle == NULL || pvData_ == NULL)
	   return FALSE;

   if(pclDeviceHandle->Write(pvData_, usSize_, ulActualWritten) != USBError::NONE)
   {
      pclCallback->Error(DSI_SERIAL_EWRITE);
      return FALSE;
   }

   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Get USB enumeration info. Not necessarily connected to USB.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::GetDeviceUSBInfo(UCHAR ucDevice_, UCHAR* pucProductString_, UCHAR* pucSerialString_, USHORT usBufferSize_)
{
   const USBDeviceListSI clDeviceList = USBDeviceHandleSI::GetAllDevices().GetSubList(MatchBsdName);
   if(clDeviceList.GetSize() <= ucDevice_)
      return FALSE;

   if(clDeviceList[ucDevice_]->GetProductDescription(pucProductString_, usBufferSize_) != TRUE)
      return FALSE;
      
   if(clDeviceList[ucDevice_]->GetSerialString(pucSerialString_, usBufferSize_) != TRUE)
      return FALSE;   
   
   return TRUE;
}

///////////////////////////////////////////////////////////////////////
// Get USB PID. Need to be connected to USB device.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::GetDevicePID(USHORT& usPid_)
{
   if(pclDeviceHandle == NULL)
      return FALSE;

   usPid_ = pclDeviceHandle->GetDevice().GetPid();
   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Get USB VID. Need to be connected to USB device.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialSI::GetDeviceVID(USHORT& usVid_)
{
   if(pclDeviceHandle == NULL)
      return FALSE;

   usVid_ = pclDeviceHandle->GetDevice().GetVid();
   return TRUE;
}


void DSISerialSI::USBReset()  //!!Should be a static function?
{
   //If the user specified a device number instead of a USBDevice instance, then grab it from the list
   const USBDeviceSI* pclTempDevice = pclDevice;
   if(pclDevice == NULL)
   {
      const USBDeviceListSI clDeviceList = USBDeviceHandleSI::GetAllDevices();
      if(clDeviceList.GetSize() <= ucDeviceNumber)
         return;

      pclTempDevice = clDeviceList[ucDeviceNumber];
   }
   
   pclTempDevice->USBReset();
   return;
}



//////////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
void DSISerialSI::ReceiveThread()
{
   UCHAR aucData[255];
   while(!bStopReceiveThread)
   {
      ULONG ulRxBytesRead;
      USBError::Enum eStatus = pclDeviceHandle->Read(aucData, sizeof(aucData), ulRxBytesRead, 1000);

      switch(eStatus)
      {
         case USBError::NONE:
            for(ULONG i=0; i<ulRxBytesRead; i++)
               pclCallback->ProcessByte(aucData[i]);
            break;
         
         case USBError::DEVICE_GONE:
            pclCallback->Error(DSI_SERIAL_DEVICE_GONE);
            break;

         case USBError::TIMED_OUT:
            break;

         default:
            pclCallback->Error(DSI_SERIAL_EREAD);
            bStopReceiveThread = TRUE;
            break;
      }
   }

   DSIThread_MutexLock(&stMutexCriticalSection);
      bStopReceiveThread = TRUE;
      DSIThread_CondSignal(&stEventReceiveThreadExit);                       // Set an event to alert the main process that Rx thread is finished and can be closed.
   DSIThread_MutexUnlock(&stMutexCriticalSection);

}

///////////////////////////////////////////////////////////////////////
DSI_THREAD_RETURN DSISerialSI::ProcessThread(void *pvParameter_)
{
    DSISerialSI *This = (DSISerialSI *) pvParameter_;
    This->ReceiveThread();
    return 0;
}



#endif //defined(DSI_TYPES_MACINTOSH)
