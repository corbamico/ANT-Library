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

#include "usb_device_handle_si_iokit.hpp"
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

USBDeviceList<const USBDeviceIOKit> USBDeviceHandleSIIOKit::clDeviceList;


//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////

const UCHAR USB_ANT_CONFIGURATION = 1;
const UCHAR USB_ANT_INTERFACE = 0;
const UCHAR USB_ANT_EP_IN  = 0x81;
const UCHAR USB_ANT_EP_OUT = 0x01;

#define SI_REQ_IFC_ENABLE              ((UCHAR)0x00)        // Enable/disable interface
#define SI_REQ_SET_BAUDDIV             ((UCHAR)0x01)        // Set baud rate divisor
#define SI_REQ_SET_LINE_CTL            ((UCHAR)0x03)        // Set line control
#define SI_REQ_SET_BREAK               ((UCHAR)0x05)        // Set break
#define SI_REQ_IMM_CHAR                ((UCHAR)0x06)        // Send character out of order
#define SI_REQ_SET_MHS                 ((UCHAR)0x07)        // Set modem handshake
#define SI_REQ_GET_MDMSTS              ((UCHAR)0x08)        // Get modem status
#define SI_REQ_SET_XON                 ((UCHAR)0x09)        // Emulate XON
#define SI_REQ_SET_XOFF                ((UCHAR)0x0A)        // Emulate XOFF
#define SI_REQ_GET_PROPS               ((UCHAR)0x0F)        // Get properties
#define SI_REQ_RESET                   ((UCHAR)0x11)        // Reset
#define SI_REQ_PURGE                   ((UCHAR)0x12)        // Purge
#define SI_REQ_SET_FLOW                ((UCHAR)0x13)        // Set flow control
#define SI_REQ_EMBED_EVENTS            ((UCHAR)0x15)        // Control embedding of events in data stream
#define SI_REQ_SET_CHARS               ((UCHAR)0x19)        // Set special characters.

//bit 0-4 of request type bitmap
#define SI_REQ_TYPE_RECP_DEVICE        ((UCHAR)0x00)        // Recipient: device
#define SI_REQ_TYPE_RECP_INTERFACE     ((UCHAR)0x01)        // Recipient: interface
#define SI_REQ_TYPE_RECP_ENDPOINT      ((UCHAR)0x02)        // Recipient: endpoint
#define SI_REQ_TYPE_RECP_OTHER         ((UCHAR)0x03)        // Recipient: other
//bit 5,6 of request type bitmap
#define SI_REQ_TYPE_STD_TYPE           ((UCHAR)0x00)        // Type: standard
#define SI_REQ_TYPE_CLASS_TYPE         ((UCHAR)0x10)        // Type: class
#define SI_REQ_TYPE_VENDOR_TYPE        ((UCHAR)0x20)        // Type: vendor
//bit 7 of request type bitmap
#define SI_REQ_TYPE_DIR_DEV_TO_HOST    ((UCHAR)0x40)        // Direction: device to host
#define SI_REQ_TYPE_DIR_HOST_TO_DEV    ((UCHAR)0x00)        // Direction: host to device

#define SI_REQ_INDEX_BEGIN             ((UCHAR)0x00)

#define SI_BAUD_DIVISOR                ((ULONG)0x00384000)  // 3,686,400 Hz

//Line control parameters
//bits 0-3
#define SI_LINE_CTRL_1_STOP_BIT        ((USHORT)0x0000)     // set 1 stop bit
#define SI_LINE_CTRL_1_5_STOP_BITS     ((USHORT)0x0001)     // set 1.5 stop bits 
#define SI_LINE_CTRL_2_STOP_BITS       ((USHORT)0x0002)     // set 2 stop bits
//bits 4-7
#define SI_LINE_CTRL_NO_PARITY         ((USHORT)0x0000)     // no parity
#define SI_LINE_CTRL_ODD_PARITY        ((USHORT)0x0010)     // odd parity
#define SI_LINE_CTRL_EVEN_PARITY       ((USHORT)0x0020)     // even parity
#define SI_LINE_CTRL_MARK_PARITY       ((USHORT)0x0030)     // mark parity
#define SI_LINE_CTRL_SPACE_PARITY      ((USHORT)0x0040)     // space parity
//bits 8-15
#define SI_LINE_CTRL_WLENGTH_5         ((USHORT)0x0500)     // word length = 5
#define SI_LINE_CTRL_WLENGTH_6         ((USHORT)0x0600)     // word length = 6
#define SI_LINE_CTRL_WLENGTH_7         ((USHORT)0x0700)     // word length = 7
#define SI_LINE_CTRL_WLENGTH_8         ((USHORT)0x0800)     // word length = 8

#define SI_FLOW_CTRL_NUM_PARAMS        ((UCHAR)4)

//Flow control parameters
//field 1
//bits 0-1
#define SI_FLOW_CTRL_DTR_INACTIVE      ((ULONG)0x00000000)  // DTR is held inactive
#define SI_FLOW_CTRL_DTR_ACTIVE        ((ULONG)0x00000001)  // DTR is held active
#define SI_FLOW_CTRL_DTR_FIRMWARE      ((ULONG)0x00000002)  // DTR is controlled by port firmware
//bit 2 reserved, must remain zero
//bit 3
#define SI_FLOW_CTRL_CTS_STATUS_INPUT  ((ULONG)0x00000000)  // CTS is simply a status input
#define SI_FLOW_CTRL_CTS_HANDSHAKE     ((ULONG)0x00000008)  // CTS is a handshake line
//bit 4
#define SI_FLOW_CTRL_DSR_STATUS_INPUT  ((ULONG)0x00000000)  // DSR is simply a status input
#define SI_FLOW_CTRL_DSR_HANDSHAKE     ((ULONG)0x00000010)  // DSR is simply a status input
//bit 5
#define SI_FLOW_CTRL_DCD_STATUS_INPUT  ((ULONG)0x00000000)  // DSR is simply a status input
#define SI_FLOW_CTRL_DCD_HANDSHAKE     ((ULONG)0x00000020)  // DSR is simply a status input
//bit 6
#define SI_FLOW_CTRL_DSR_SENS_INPUT    ((ULONG)0x00000000)  // DSR is simply a status input
#define SI_FLOW_CTRL_DSR_DISCARD       ((ULONG)0x00000010)  // DSR low discards data
//field 2
#define SI_FLOW_CTRL_RESERVED          ((ULONG)0x00000040)  // this field should be 0 according to SiLabs' documents, but this is what was shown on the port sniff
//field 3
#define SI_FLOW_CTRL_XON_LIMIT         ((ULONG)0x00000000)  // XON not used
//field 4
#define SI_FLOW_CTRL_XOFF_LIMIT        ((ULONG)0x00000000)  // XOFF not used

//Queue purge parameters
#define SI_PURGE_CLR_TRANSMIT_0        ((UCHAR)0x01)        // clear transmit queue (unclear why this is repeated, sniffer reports this not being used)
#define SI_PURGE_CLR_RECV_1            ((UCHAR)0x02)        // clear receive queue (unclear why this is repeated, sniffer reports this not being used)
#define SI_PURGE_CLR_TRANSMIT_2        ((UCHAR)0x04)        // clear transmit queue
#define SI_PURGE_CLR_RECV_3            ((UCHAR)0x08)        // clear receive queue

//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////

const USBDeviceListIOKit USBDeviceHandleSIIOKit::GetAllDevices()
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


BOOL USBDeviceHandleSIIOKit::CanOpenDevice(const USBDeviceIOKit*const & pclDevice_)
{
   if(pclDevice_ == FALSE)
      return FALSE;

   return USBDeviceHandleSIIOKit::TryOpen(*pclDevice_);
}

const USBDeviceListIOKit USBDeviceHandleSIIOKit::GetAvailableDevices()
{
   return USBDeviceHandleSIIOKit::GetAllDevices().GetSubList(USBDeviceHandleSIIOKit::CanOpenDevice);
}



///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
USBDeviceHandleSIIOKit::USBDeviceHandleSIIOKit(const USBDeviceIOKit& clDevice_, ULONG ulBaudRate_)
try
:
   USBDeviceHandle(),
   pclDeviceHandle(NULL),
   ulBaudRate(ulBaudRate_),
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
USBDeviceHandleSIIOKit::~USBDeviceHandleSIIOKit()
{
   //!!Delete all the elements in the queue?
}



//A more efficient way to test if you can open a device.  For instance, this function won't create a receive loop, etc.)
//Doesn't test for correct baudrate.
BOOL USBDeviceHandleSIIOKit::TryOpen(const USBDeviceIOKit& clDevice_)
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
BOOL USBDeviceHandleSIIOKit::Open(const USBDeviceIOKit& clDevice_, USBDeviceHandleSIIOKit*& pclDeviceHandle_, ULONG ulBaudRate_)
{
   try
   {
      pclDeviceHandle_ = new USBDeviceHandleSIIOKit(clDevice_, ulBaudRate_);
   }
   catch(...)
   {
      pclDeviceHandle_ = NULL;
      return FALSE;
   }

   return TRUE;
}


BOOL USBDeviceHandleSIIOKit::POpen()
{
   //byte array of fixed-endian ULONG values
   unsigned char aucFlowControlParams[SI_FLOW_CTRL_NUM_PARAMS * sizeof(unsigned long)] = {0};
   unsigned char* pucFlowControlBytes = &aucFlowControlParams[0];

   // Make sure all handles are reset before opening again.
   PClose();

   IOKitError::Enum eIOReturn;
   USBError::Enum eUSBReturn;

   eIOReturn = IOKitDeviceHandle::Open(clDevice.GetIOKitDevice(), pclDeviceHandle);
   if(eIOReturn != IOKitError::SUCCESS)
      return FALSE;

   bDeviceGone = FALSE;

   //enable interface
   eUSBReturn = WriteControlTransfer(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE,
                        SI_REQ_IFC_ENABLE,
                        MAX_USHORT);         //interface enable is 0x0001, but this is the way the SiLabs' drivers performed the init
   if(eUSBReturn != USBError::NONE)
      return FALSE;

   //baudrate
   eUSBReturn = WriteControlTransfer(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE,
                        SI_REQ_SET_BAUDDIV,
                        UINTDIV(SI_BAUD_DIVISOR, ulBaudRate));
   if(eUSBReturn != USBError::NONE)
      return FALSE;

   //line control
   eUSBReturn = WriteControlTransfer(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE,
                        SI_REQ_SET_LINE_CTL,
                        SI_LINE_CTRL_1_STOP_BIT | SI_LINE_CTRL_NO_PARITY | SI_LINE_CTRL_WLENGTH_8); //8-N-1 port setting
   if(eUSBReturn != USBError::NONE)
      return FALSE;

   int iBytesWritten = 0;
   //flow control; maintain same "transmission endianness" regardless of machine endianness
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_CTS_HANDSHAKE, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   pucFlowControlBytes += sizeof(unsigned long);
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_RESERVED, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   pucFlowControlBytes += sizeof(unsigned long);
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_XON_LIMIT, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   pucFlowControlBytes += sizeof(unsigned long);
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_XOFF_LIMIT, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   eUSBReturn = WriteControlTransfer((UCHAR)(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE),
                        (UCHAR)SI_REQ_SET_FLOW,
                        (USHORT)0,
                        (USHORT)SI_REQ_INDEX_BEGIN,
                        (void*)aucFlowControlParams,
                        (ULONG)(SI_FLOW_CTRL_NUM_PARAMS * sizeof(unsigned long)),
                        (ULONG&)iBytesWritten);
   if(eUSBReturn != USBError::NONE)
      return FALSE;

   //purge the queues
   eUSBReturn = WriteControlTransfer(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE,
                        SI_REQ_PURGE,
                        SI_PURGE_CLR_TRANSMIT_2 | SI_PURGE_CLR_RECV_3); //clear transmit and recv queues
   if(eUSBReturn != USBError::NONE)
      return FALSE;
   
   //get serial string if valid serial string exists
   if (clDevice.GetIOKitDevice().stDeviceDescriptor.iSerialNumber)
   {
      UCHAR aucData[255];
      int iTransferred = 0;
      eIOReturn = pclDeviceHandle->GetStringDescriptorAscii(clDevice.GetIOKitDevice().stDeviceDescriptor.iSerialNumber, aucData, sizeof(aucData), iTransferred);
      if(eIOReturn == IOKitError::SUCCESS)
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

   eIOReturn = pclDeviceHandle->ClaimInterface(USB_ANT_INTERFACE);
	if(eIOReturn != IOKitError::SUCCESS)
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
   hReceiveThread = DSIThread_CreateThread(&USBDeviceHandleSIIOKit::ProcessThread, this);
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
BOOL USBDeviceHandleSIIOKit::Close(USBDeviceHandleSIIOKit*& pclDeviceHandle_, BOOL bReset_)
{
   if(pclDeviceHandle_ == NULL)
      return FALSE;

   pclDeviceHandle_->PClose(bReset_);
   delete pclDeviceHandle_;
   pclDeviceHandle_ = NULL;

   return TRUE;
}

void USBDeviceHandleSIIOKit::PClose(BOOL bReset_)
{
   //byte array of fixed-endian ULONG values
   unsigned char aucFlowControlParams[SI_FLOW_CTRL_NUM_PARAMS * sizeof(unsigned long)] = {0};
   unsigned char* pucFlowControlBytes = &aucFlowControlParams[0];
   int iBytesWritten = 0;

   //flow control; maintain same "transmission endianness" regardless of machine endianness
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_CTS_HANDSHAKE, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   pucFlowControlBytes += sizeof(unsigned long);
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_RESERVED, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   pucFlowControlBytes += sizeof(unsigned long);
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_XON_LIMIT, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   pucFlowControlBytes += sizeof(unsigned long);
   Convert_ULONG_To_Bytes(SI_FLOW_CTRL_XOFF_LIMIT, pucFlowControlBytes+3, pucFlowControlBytes+2, pucFlowControlBytes+1, pucFlowControlBytes);
   WriteControlTransfer((UCHAR)(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE),
                        (UCHAR)SI_REQ_SET_FLOW,
                        (USHORT)0,
                        (USHORT)SI_REQ_INDEX_BEGIN,
                        (void*)aucFlowControlParams,
                        (ULONG)(SI_FLOW_CTRL_NUM_PARAMS * sizeof(unsigned long)),
                        (ULONG&)iBytesWritten);

   //purge the queues
   WriteControlTransfer(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE,
                        SI_REQ_PURGE,
                        SI_PURGE_CLR_TRANSMIT_2 | SI_PURGE_CLR_RECV_3); //clear transmit and recv queues

   //disable interface
   WriteControlTransfer(SI_REQ_TYPE_DIR_DEV_TO_HOST | SI_REQ_TYPE_STD_TYPE | SI_REQ_TYPE_RECP_DEVICE,
                        SI_REQ_IFC_ENABLE,
                        0);         //disable interface

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
USBError::Enum USBDeviceHandleSIIOKit::Write(void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_)
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

///////////////////////////////////////////////////////////////////////////////////////////////
// Writes control transfer bytes to the device for configuring Si USBXpress device serial lines
///////////////////////////////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleSIIOKit::WriteControlTransfer(UCHAR ucReqType, UCHAR ucReq, USHORT usValue, USHORT usIndex, void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   if(pvData_ == NULL)
      return USBError::INVALID_PARAM;

   int iWrittenSize = 0;
   if(pclDeviceHandle->WriteControl((unsigned char) ucReqType, (unsigned char) ucReq, (unsigned short) usValue, (unsigned short) usIndex,
                                    (unsigned char*) pvData_, (int) ulSize_, iWrittenSize, 1000) != IOKitError::SUCCESS)
   {
      ulBytesWritten_ = iWrittenSize;
      return USBError::FAILED;
   }

   ulBytesWritten_ = iWrittenSize;

   if(iWrittenSize != static_cast<int>(ulSize_))
      return USBError::FAILED;

   return USBError::NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Writes control transfer bytes to the device for configuring Si USBXpress device serial line, no data to pass in
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleSIIOKit::WriteControlTransfer(UCHAR ucReqType, UCHAR ucReq, USHORT usValue)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   int iWrittenSize = 0;
   if(pclDeviceHandle->WriteControl((unsigned char) ucReqType, (unsigned char) ucReq, (unsigned short) usValue, 0,
                                    (unsigned char*) NULL, 0, iWrittenSize, 1000) != IOKitError::SUCCESS)
      return USBError::FAILED;

   if(iWrittenSize)
      return USBError::FAILED;

   return USBError::NONE;
}

///////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleSIIOKit::Read(void* pvData_, ULONG ulSize_, ULONG& ulBytesRead_, ULONG ulWaitTime_)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   ulBytesRead_ = clRxQueue.PopArray(reinterpret_cast<char* const>(pvData_), ulSize_, ulWaitTime_);
   if(ulBytesRead_ == 0)
      return USBError::TIMED_OUT;

   return USBError::NONE;
}


//!!Can optimize this to push more data at a time into the receive queue
void USBDeviceHandleSIIOKit::ReceiveThread()
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
DSI_THREAD_RETURN USBDeviceHandleSIIOKit::ProcessThread(void* pvParameter_)
{
   USBDeviceHandleSIIOKit* This = reinterpret_cast<USBDeviceHandleSIIOKit*>(pvParameter_);
   This->ReceiveThread();
   return 0;
}



#endif //defined(DSI_TYPES_MACINTOSH)
