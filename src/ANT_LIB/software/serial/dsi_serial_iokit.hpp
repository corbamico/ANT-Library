 /*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright � 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#ifndef DSI_SERIAL_IOKIT_HPP
#define DSI_SERIAL_IOKIT_HPP

#include "types.h"
#include "dsi_thread.h"
#include "dsi_serial.hpp"
#include "dsi_serial_callback.hpp"

#include "usb_device_handle_iokit.hpp"
#include "usb_device_iokit.hpp"


//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

class DSISerialIOKit : public DSISerial
{
   private:
   
	//Receive thread variables
	DSI_THREAD_ID hReceiveThread;					// Handle for the receive thread.
   DSI_MUTEX stMutexCriticalSection;				// Mutex used with the wait condition
   DSI_CONDITION_VAR stEventReceiveThreadExit;		// Event to signal the receive thread has ended.
   BOOL bStopReceiveThread;						// Flag to stop the receive thread.

	
   USBDeviceHandleIOKit* pclDeviceHandle;                     // Handle to the USB device.

   const USBDeviceIOKit* pclDevice;
   UCHAR ucDeviceNumber;
   
   void ReceiveThread();
   static DSI_THREAD_RETURN ProcessThread(void* pvParameter_);


   public:
      DSISerialIOKit();
      ~DSISerialIOKit();

      BOOL Init(ULONG ulBaud_, UCHAR ucDeviceNumber_);
      BOOL Init(ULONG ulBaud_, const USBDeviceIOKit& clDevice_, UCHAR ucDeviceNumber_);

      void USBReset();
      UCHAR GetNumberOfDevices();

      // Methods inherited from the base class:
      BOOL AutoInit();
      ULONG GetDeviceSerialNumber();

      BOOL Open();
      void Close(BOOL bReset = FALSE);
      BOOL WriteBytes(void *pvData_, USHORT usSize_);
      UCHAR GetDeviceNumber();
      
      BOOL GetDeviceUSBInfo(UCHAR ucDevice_, UCHAR* pucProductString_, UCHAR* pucSerialString_, USHORT usBufferSize_);
      BOOL GetDevicePID(USHORT& usPid_);
      BOOL GetDeviceVID(USHORT& usVid_);

};

#endif // !defined(DSI_SERIAL_IOKIT_HPP)
