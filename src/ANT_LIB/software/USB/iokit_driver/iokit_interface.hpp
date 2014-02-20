 /*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */


#ifndef IOKIT_INTERFACE_HPP
#define IOKIT_INTERFACE_HPP

#include "types.h"
#include "iokit_types.hpp"


//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

class IOKitDeviceHandle;

//NOTE: Supports IOUSBFamily 1.8.2 and later; Mac OS X 10.0.4 and later.
class IOKitInterface
{

  public:
   IOKitInterface();
   virtual ~IOKitInterface();

   IOKitError::Enum Claim(const IOKitDeviceHandle& dev_handle, int interface_num);
   IOKitError::Enum Release();

   BOOL IsClaimed() const { return bIsClaimed; }
   usb_interface_t** GetInterface() const { return interface; }
   uint8_t GetEndpoint(int i) const { return endpoint_addrs[i]; }

   static const int MAX_ENDPOINTS = 32;

  private:
   IOReturn GetInterface(/*const*/ usb_device_t** darwin_device, int interface_num, io_service_t& usbInterfacep);
   IOKitError::Enum GetEndpoints();

   BOOL bIsClaimed;
	usb_interface_t**    interface;  //!!should we turn this into IOUSBInterfaceInterface IOUSBInterfaceInterface245?
   uint8_t              num_endpoints;
   CFRunLoopRef         run_loop;
   CFRunLoopSourceRef   event_source;
   uint8_t              endpoint_addrs[MAX_ENDPOINTS];
};

#endif // !defined(IOKIT_INTERFACE_HPP)

