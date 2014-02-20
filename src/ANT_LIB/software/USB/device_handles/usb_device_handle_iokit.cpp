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


#include "defines.h"
#include "macros.h"
#include "dsi_debug.hpp"

#include "usb_device_list.hpp"

#include "dsi_thread.h"

#include "usb_device_handle_iokit.hpp"
#include "usb_device_iokit.hpp"

#include "iokit_device.hpp"
#include "iokit_device_list.hpp"
#include "iokit_device_handle.hpp"

#include <vector>

using namespace std;

//#define NUMBER_OF_DEVICES_CHECK_THREAD_KILL     //Only include this if we are sure we want to terminate the Rx thread if the number of devices are ever reduced.

//////////////////////////////////////////////////////////////////////////////////
// Static declarations
//////////////////////////////////////////////////////////////////////////////////

USBDeviceList<const USBDeviceIOKit> USBDeviceHandleIOKit::clDeviceList;


//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////

const UCHAR USB_ANT_CONFIGURATION = 1;
const UCHAR USB_ANT_INTERFACE = 0;
const UCHAR USB_ANT_EP_IN  = 0x81;
const UCHAR USB_ANT_EP_OUT = 0x01;



//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////

const USBDeviceListIOKit USBDeviceHandleIOKit::GetAllDevices()
{
   USBDeviceListIOKit clList;

   clDeviceList = USBDeviceList<const USBDeviceIOKit>();  //clear device list


   vector<IOKitDevice*> clIOKitDeviceList;
   if(IOKitDeviceList::GetDeviceList(clIOKitDeviceList) != IOKitError::SUCCESS)
      return clList;


   vector<IOKitDevice*>::const_iterator clIterator;
   for(clIterator = clIOKitDeviceList.begin(); clIterator != clIOKitDeviceList.end(); clIterator++)
   {
      if(*clIterator == NULL)
         continue;

      IOKitDevice& clIOKitDevice = **clIterator;

      clDeviceList.Add(USBDeviceIOKit(clIOKitDevice) );  //save the copies to the static private list
      clList.Add(clDeviceList.GetAddress(clDeviceList.GetSize() - 1) );  //save a pointer to the just added device
   }

   IOKitDeviceList::FreeDeviceList(clIOKitDeviceList);

	return clList;
}


BOOL USBDeviceHandleIOKit::CanOpenDevice(const USBDeviceIOKit*const & pclDevice_)
{
   if(pclDevice_ == FALSE)
      return FALSE;

   return USBDeviceHandleIOKit::TryOpen(*pclDevice_);
}

const USBDeviceListIOKit USBDeviceHandleIOKit::GetAvailableDevices()
{
   return USBDeviceHandleIOKit::GetAllDevices().GetSubList(USBDeviceHandleIOKit::CanOpenDevice);
}



///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
USBDeviceHandleIOKit::USBDeviceHandleIOKit(const USBDeviceIOKit& clDevice_)
try
:
   USBDeviceHandle(),
   pclDeviceHandle(NULL),
   clDevice(clDevice_),
   bDeviceGone(TRUE)
{

   hReceiveThread = NULL;
   bStopReceiveThread = TRUE;

   if(POpen() == FALSE)
      throw 0; //!!We need something to throw

   return;
}
catch(...)
{
   throw;
}

///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
USBDeviceHandleIOKit::~USBDeviceHandleIOKit()
{
   //!!Delete all the elements in the queue?
}



//A more efficient way to test if you can open a device.  For instance, this function won't create a receive loop, etc.)
//Doesn't test for correct baudrate.
BOOL USBDeviceHandleIOKit::TryOpen(const USBDeviceIOKit& clDevice_)
{
   IOKitDeviceHandle* pclTempHandle;
   IOKitError::Enum eReturn = IOKitDeviceHandle::Open(clDevice_.GetIOKitDevice(), pclTempHandle);
   if(eReturn != IOKitError::SUCCESS)
      return FALSE;

   IOKitDeviceHandle::Close(pclTempHandle);
   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Opens port, starts receive thread.
///////////////////////////////////////////////////////////////////////
BOOL USBDeviceHandleIOKit::Open(const USBDeviceIOKit& clDevice_, USBDeviceHandleIOKit*& pclDeviceHandle_)
{
   try
   {
      pclDeviceHandle_ = new USBDeviceHandleIOKit(clDevice_);
   }
   catch(...)
   {
      pclDeviceHandle_ = NULL;
      return FALSE;
   }

   return TRUE;
}


BOOL USBDeviceHandleIOKit::POpen()
{
   // Make sure all handles are reset before opening again.
   PClose();

   IOKitError::Enum eReturn = IOKitDeviceHandle::Open(clDevice.GetIOKitDevice(), pclDeviceHandle);
   if(eReturn != IOKitError::SUCCESS)
      return FALSE;

   bDeviceGone = FALSE;

   //get serial string if valid serial string exists
   if (clDevice.GetIOKitDevice().stDeviceDescriptor.iSerialNumber)
   {
      UCHAR aucData[255];
      int iTransferred = 0;
      eReturn = pclDeviceHandle->GetStringDescriptorAscii(clDevice.GetIOKitDevice().stDeviceDescriptor.iSerialNumber, aucData, sizeof(aucData), iTransferred);
      if(eReturn == IOKitError::SUCCESS)
         clDevice.UpdateSerialString(aucData, sizeof(aucData));
   }

   /*
	//Warning: Setting the configuration to the current configuration could cause a lightweight reset.
   ret = pfSetConfiguration(device_handle, USB_ANT_CONFIGURATION);
   if(ret != 0)
   {
      Close();
      return FALSE;
   }
   */

	eReturn = pclDeviceHandle->ClaimInterface(USB_ANT_INTERFACE);
	if(eReturn != IOKitError::SUCCESS)
	{
		PClose();
		return FALSE;
	}

   /* //!!Fails
   ret = usb_set_altinterface(device_handle, 0);
   if(ret != 0)
	{
		Close();
		return FALSE;
	}
   */

	/*
   #if !defined(DSI_TYPES_WINDOWS)     //!!Fails on Windows
   {
      ret = pfClearHalt(device_handle, USB_ANT_EP_OUT);
      if(ret != 0)
	   {
		   Close();
		   return FALSE;
	   }
   }
   #endif
   */


	//hReceiveThread is used keeps track if these are initialized/destroyed
   if(DSIThread_MutexInit(&stMutexCriticalSection) != DSI_THREAD_ENONE)
   {
      PClose();
      return FALSE;
   }

   if(DSIThread_CondInit(&stEventReceiveThreadExit) != DSI_THREAD_ENONE)
   {
      DSIThread_MutexDestroy(&stMutexCriticalSection);
      PClose();
      return FALSE;
   }

   bStopReceiveThread = FALSE;
   hReceiveThread = DSIThread_CreateThread(&USBDeviceHandleIOKit::ProcessThread, this);
   if(hReceiveThread == NULL)
   {
      DSIThread_MutexDestroy(&stMutexCriticalSection);
    	DSIThread_CondDestroy(&stEventReceiveThreadExit);
      PClose();
      return FALSE;
   }

	return TRUE;
}

///////////////////////////////////////////////////////////////////////
// Closes the USB connection, kills receive thread.
///////////////////////////////////////////////////////////////////////

//!!User is still expected to close the handle if the device is gone
BOOL USBDeviceHandleIOKit::Close(USBDeviceHandleIOKit*& pclDeviceHandle_, BOOL bReset_)
{
   if(pclDeviceHandle_ == NULL)
      return FALSE;

   pclDeviceHandle_->PClose(bReset_);
   delete pclDeviceHandle_;
   pclDeviceHandle_ = NULL;

   return TRUE;
}

void USBDeviceHandleIOKit::PClose(BOOL bReset_)
{

   bDeviceGone = TRUE;

   if (hReceiveThread)
   {
      DSIThread_MutexLock(&stMutexCriticalSection);
      if(bStopReceiveThread == FALSE)
      {
         bStopReceiveThread = TRUE;

         if (pclDeviceHandle != NULL)
         {
            // the receive thread may still be awaiting data; cancel transfers and signal waiting function to release receive thread
            pclDeviceHandle->CancelQueuedTransfers();
         }

         if (DSIThread_CondTimedWait(&stEventReceiveThreadExit, &stMutexCriticalSection, 3000) == DSI_THREAD_ETIMEDOUT)
         {
            // We were unable to stop the thread normally.
            DSIThread_DestroyThread(hReceiveThread);
         }
      }
      DSIThread_MutexUnlock(&stMutexCriticalSection);

      DSIThread_ReleaseThreadID(hReceiveThread);
      hReceiveThread = NULL;

      DSIThread_MutexDestroy(&stMutexCriticalSection);
      DSIThread_CondDestroy(&stEventReceiveThreadExit);
   }

   if(pclDeviceHandle != NULL)
   {
      if(bReset_)
         IOKitDeviceHandle::Reset(pclDeviceHandle);
      else
         IOKitDeviceHandle::Close(pclDeviceHandle);
      pclDeviceHandle = NULL;
   }

   return;
}

///////////////////////////////////////////////////////////////////////
// Writes ucSize_ bytes to USB, returns number of bytes not written.
///////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleIOKit::Write(void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   if(pvData_ == NULL)
      return USBError::INVALID_PARAM;

   //!!is there a max message size that we should test for?
   int iActualWritten = 0;
   if(pclDeviceHandle->Write(USB_ANT_EP_OUT, (unsigned char*)pvData_, ulSize_, iActualWritten, 1000) != IOKitError::SUCCESS)
   {
      ulBytesWritten_ = iActualWritten;
      return USBError::FAILED;
   }

   ulBytesWritten_ = iActualWritten;

   if(iActualWritten != static_cast<int>(ulSize_))
      return USBError::FAILED;

   return USBError::NONE;
}



//////////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleIOKit::Read(void* pvData_, ULONG ulSize_, ULONG& ulBytesRead_, ULONG ulWaitTime_)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   ulBytesRead_ = clRxQueue.PopArray(reinterpret_cast<char* const>(pvData_), ulSize_, ulWaitTime_);
   if(ulBytesRead_ == 0)
      return USBError::TIMED_OUT;

   return USBError::NONE;
}


//!!Can optimize this to push more data at a time into the receive queue
void USBDeviceHandleIOKit::ReceiveThread()
{
   #if defined(DEBUG_FILE)
   BOOL bRxDebug = DSIDebug::ThreadInit("ao_iokit_receive.txt");
   DSIDebug::ThreadEnable(TRUE);
   #endif

   int iBytesRead = 0;
   char acData[255];
   while (!bStopReceiveThread)
   {
      IOKitError::Enum eReturn = pclDeviceHandle->Read(USB_ANT_EP_IN, (unsigned char*)acData, sizeof(acData), iBytesRead, 60000);
      switch(eReturn)
      {
         case IOKitError::TIMEOUT:
            break;

         case IOKitError::SUCCESS:
            clRxQueue.PushArray(acData, iBytesRead);
            break;

         case IOKitError::IO:
         case IOKitError::BUSY:
         case IOKitError::PIPE:
         case IOKitError::INTERRUPTED:
            #if defined(DEBUG_FILE)
            if(bRxDebug)
            {
               char szMesg[255];
               SNPRINTF(szMesg, 255, "Error: Read error %d\n", eReturn);
               DSIDebug::ThreadWrite(szMesg);
            }
            #endif
            break;


         case IOKitError::INVALID_PARAM:
         case IOKitError::ACCESS:
         case IOKitError::NOT_FOUND:
         case IOKitError::OVERFLOWED:
         case IOKitError::NO_MEM:
         case IOKitError::NOT_SUPPORTED:
         case IOKitError::OTHER:
         default:
            #if defined(DEBUG_FILE)
            if(bRxDebug)
            {
               char szMesg[255];
               SNPRINTF(szMesg, 255, "Error: Read error %d\n", eReturn);
               DSIDebug::ThreadWrite(szMesg);
            }
            #endif
         //thru
         case IOKitError::NO_DEVICE:
            bDeviceGone = TRUE;
            bStopReceiveThread = TRUE;
            break;
      }
   }

   bDeviceGone = TRUE;

   DSIThread_MutexLock(&stMutexCriticalSection);
      bStopReceiveThread = TRUE;
      DSIThread_CondSignal(&stEventReceiveThreadExit);                       // Set an event to alert the main process that Rx thread is finished and can be closed.
   DSIThread_MutexUnlock(&stMutexCriticalSection);

   return;
}

///////////////////////////////////////////////////////////////////////
DSI_THREAD_RETURN USBDeviceHandleIOKit::ProcessThread(void* pvParameter_)
{
   USBDeviceHandleIOKit* This = reinterpret_cast<USBDeviceHandleIOKit*>(pvParameter_);
   This->ReceiveThread();
   return 0;
}



#endif //defined(DSI_TYPES_MACINTOSH)
