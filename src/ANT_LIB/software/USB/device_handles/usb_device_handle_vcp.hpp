/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#ifndef USB_DEVICE_HANDLE_VCP_HPP
#define USB_DEVICE_HANDLE_VCP_HPP

#include "types.h"
#include "dsi_thread.h"

#include "usb_device_handle.hpp"
#include "usb_device_vcp.hpp"
#include "usb_device_list.hpp"

#include "dsi_ts_queue.hpp"

#include <termios.h>
#include <IOKit/IOKitLib.h>


//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////

typedef USBDeviceList<const USBDeviceSI*> USBDeviceListSI;


//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

class USBDeviceHandleSI : public USBDeviceHandle
{
   private:

      TSQueue<char> clRxQueue;
      
      // Thread Variables
      DSI_THREAD_ID hReceiveThread;                         // Handle for the receive thread.
      DSI_MUTEX stMutexCriticalSection;                     // Mutex used with the wait condition
      DSI_CONDITION_VAR stEventReceiveThreadExit;           // Event to signal the receive thread has ended.
      BOOL bStopReceiveThread;                              // Flag to stop the receive thread.

      int hSerialFileDescriptor;
      struct termios stOriginalTTYAttrs;				// Holds the original termios attributes so we can reset them

      // Device Variables
      const USBDeviceSI clDevice;
      const ULONG ulBaudRate;
      volatile BOOL bDeviceGone;

      // Private Member Functions
      BOOL POpen();
      void PClose(BOOL bReset_ = FALSE);
      void ReceiveThread();
      static DSI_THREAD_RETURN ProcessThread(void *pvParameter_);
      
   
      static USBDeviceList<const USBDeviceSI> clDeviceList;  //This holds only instances of USBDeviceSI (unless someone manually makes their own)
      
      static BOOL FindModems(io_iterator_t* phMatchingServices_);

      //stuff used for notification thread
      pthread_t thread;
      pthread_mutex_t lock_runloop;   // lock protects run loop
      pthread_cond_t cond_runloop;
      
      volatile BOOL thread_started;
      volatile CFRunLoopRef run_loop;
      
      static void DevicesDetached(void* param, io_iterator_t removed_devices);
      static void* EventThreadStart(void* param_);
      void EventThread();

      //!!Const-correctness!
   public:
      
      static const USBDeviceListSI GetAllDevices();  //!!List copy!   //!!Should we make the list static instead and return a reference to it?
      static const USBDeviceListSI GetAvailableDevices();  //!!List copy!

      static BOOL Open(const USBDeviceSI& clDevice_, USBDeviceHandleSI*& pclDeviceHandle_, ULONG ulBaudRate_);  //should these be member functions?
      static BOOL Close(USBDeviceHandleSI*& pclDeviceHandle_, BOOL bReset_ = FALSE);
      static BOOL TryOpen(const USBDeviceSI& clDevice_);

      static UCHAR GetNumberOfDevices();
      
      
      // Methods inherited from the base class:
      USBError::Enum Write(void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_);
      USBError::Enum Read(void* pvData_, ULONG ulSize_, ULONG& ulBytesRead_, ULONG ulWaitTime_);
      
      const USBDevice& GetDevice() { return clDevice; }
      
      
   protected:

      USBDeviceHandleSI(const USBDeviceSI& clDevice_, ULONG ulBaudRate_);
      virtual ~USBDeviceHandleSI();
      
      const USBDeviceHandleSI& operator=(const USBDeviceHandleSI& clDevicehandle_) { return clDevicehandle_; }  //!!NOP

};

#endif // !defined(USB_DEVICE_HANDLE_VCP_HPP)

