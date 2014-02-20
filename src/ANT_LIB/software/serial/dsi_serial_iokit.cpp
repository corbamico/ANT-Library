/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright � 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#include "types.h"
#if defined(DSI_TYPES_MACINTOSH)


#include "dsi_serial_iokit.hpp"

#include "dsi_thread.h"

#include "usb_device_handle_iokit.hpp"
#include "usb_device_iokit.hpp"
#include "macros.h"

//static const USHORT ANT_USB_VID = 0x0FCF;


//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
DSISerialIOKit::DSISerialIOKit()
:
   DSISerial(),
   pclDeviceHandle (NULL),
   pclDevice (NULL),   
   hReceiveThread (NULL),
   bStopReceiveThread(TRUE),
   ucDeviceNumber(0xFF)   
{
   return;
}

///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
DSISerialIOKit::~DSISerialIOKit()
{
   Close();
}

BOOL MatchVid(const USBDeviceIOKit*const & pclDevice_)  //!!Make a member function
{
	if(pclDevice_ == NULL)
		return FALSE;
   
	return (pclDevice_->GetVid() == USBDeviceHandle::USB_ANT_VID);
}

///////////////////////////////////////////////////////////////////////
// Initializes and opens the object.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialIOKit::AutoInit()
{
   Close();
	
   const USBDeviceListIOKit clDeviceList = USBDeviceHandleIOKit::GetAvailableDevices().GetSubList(MatchVid);
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
BOOL DSISerialIOKit::Init(ULONG /*ulBaud_*/, UCHAR ucDeviceNumber_)
{
	Close();

   const USBDeviceListIOKit clDeviceList = USBDeviceHandleIOKit::GetAvailableDevices().GetSubList(MatchVid);
   if(clDeviceList.GetSize() <= ucDeviceNumber_)
   {
      this->USBReset();
      return FALSE;
   }

   pclDevice = NULL;
   ucDeviceNumber = ucDeviceNumber_;

   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Initializes the object.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialIOKit::Init(ULONG /*ulBaud_*/, const USBDeviceIOKit& clDevice_, UCHAR ucDeviceNumber_)
{
   Close();

   pclDevice = &clDevice_;  //!!Make copy?
   ucDeviceNumber = ucDeviceNumber_;

   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Returns the device serial number.
///////////////////////////////////////////////////////////////////////
ULONG DSISerialIOKit::GetDeviceSerialNumber()
{
   if(pclDeviceHandle == NULL)
      return 0;
	
   return pclDeviceHandle->GetDevice().GetSerialNumber();
}

///////////////////////////////////////////////////////////////////////
// Returns the device port number.
///////////////////////////////////////////////////////////////////////
UCHAR DSISerialIOKit::GetDeviceNumber()
{
	return ucDeviceNumber;
}



//Error is MAX_UCHAR
UCHAR DSISerialIOKit::GetNumberOfDevices()  //!!Should this be a static function?
{
   return USBDeviceHandleIOKit::GetAvailableDevices().GetSubList(MatchVid).GetSize();
}

///////////////////////////////////////////////////////////////////////
// Opens port, starts receive thread.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialIOKit::Open(void)
{

   // Make sure all handles are reset before opening again.
   Close();

   if (pclCallback == NULL)
      return FALSE;


   //If the user specified a device number instead of a USBDevice instance, then grab it from the list
   const USBDeviceIOKit* pclTempDevice = pclDevice;
   if(pclDevice == NULL)
   {
      const USBDeviceListIOKit clDeviceList = USBDeviceHandleIOKit::GetAllDevices().GetSubList(MatchVid);
      if(clDeviceList.GetSize() <= ucDeviceNumber)
         return FALSE;

      pclTempDevice = clDeviceList[ucDeviceNumber];
   }

   if(USBDeviceHandleIOKit::Open(*pclTempDevice, pclDeviceHandle) == FALSE)
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
    hReceiveThread = DSIThread_CreateThread(&DSISerialIOKit::ProcessThread, this);
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
void DSISerialIOKit::Close(BOOL bReset_)
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
      USBDeviceHandleIOKit::Close(pclDeviceHandle, bReset_);

      if(bReset_)
         DSIThread_Sleep(1750);           //Stall to allow the device to reset, trying to reopen the driver too soon can cause bad things to happen.

      pclDeviceHandle = NULL;
   }
   
   return;
}

///////////////////////////////////////////////////////////////////////
// Writes usSize_ bytes to USB, returns TRUE if successful.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialIOKit::WriteBytes(void *pvData_, USHORT usSize_)
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
BOOL DSISerialIOKit::GetDeviceUSBInfo(UCHAR ucDevice_, UCHAR* pucProductString_, UCHAR* pucSerialString_, USHORT usBufferSize_)
{
   const USBDeviceListIOKit clDeviceList = USBDeviceHandleIOKit::GetAllDevices().GetSubList(MatchVid);
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
BOOL DSISerialIOKit::GetDevicePID(USHORT& usPid_)
{
   if(pclDeviceHandle == NULL)
      return FALSE;

   usPid_ = pclDeviceHandle->GetDevice().GetPid();
   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Get USB VID. Need to be connected to USB device.
///////////////////////////////////////////////////////////////////////
BOOL DSISerialIOKit::GetDeviceVID(USHORT& usVid_)
{
   if(pclDeviceHandle == NULL)
      return FALSE;

   usVid_ = pclDeviceHandle->GetDevice().GetVid();
   return TRUE;
}


void DSISerialIOKit::USBReset()  //!!Should be a static function?
{
   //If the user specified a device number instead of a USBDevice instance, then grab it from the list
   const USBDeviceIOKit* pclTempDevice = pclDevice;
   if(pclDevice == NULL)
   {
      const USBDeviceListIOKit clDeviceList = USBDeviceHandleIOKit::GetAllDevices();
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
void DSISerialIOKit::ReceiveThread()
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
DSI_THREAD_RETURN DSISerialIOKit::ProcessThread(void *pvParameter_)
{
    DSISerialIOKit *This = (DSISerialIOKit *) pvParameter_;
    This->ReceiveThread();
    return 0;
}



#endif //defined(DSI_TYPES_MACINTOSH)