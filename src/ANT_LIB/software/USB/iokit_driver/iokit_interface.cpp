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

#include "iokit_interface.hpp"

#include "iokit_transfer.hpp"
#include "iokit_device_list.hpp"
#include "iokit_device_handle.hpp"


#include <errno.h>
#include <unistd.h>

#include <mach/clock.h>
#include <mach/clock_types.h>
#include <mach/mach_host.h>

#include <mach/mach_port.h>
#include <IOKit/IOCFBundle.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>


//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
IOKitInterface::IOKitInterface()
{
   bIsClaimed = FALSE;

   interface = IO_OBJECT_NULL;
   num_endpoints = 0;
   event_source = NULL;
   run_loop = 0;

	return;
}

///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
IOKitInterface::~IOKitInterface()
{
   if(IsClaimed() == TRUE)
      Release();
}


IOKitError::Enum IOKitInterface::Claim(const IOKitDeviceHandle& dev_handle, int interface_num)
{

   if(bIsClaimed)
      return IOKitError::BUSY;

  IOReturn kresult;

   //Re-initialize variables
   interface = IO_OBJECT_NULL;
   num_endpoints = 0;
   event_source = NULL;
   run_loop = 0;


   //Get interface
  io_service_t usbInterface = IO_OBJECT_NULL;
  kresult = GetInterface(dev_handle.usb_device, interface_num, usbInterface);
  if(kresult != kIOReturnSuccess)
    return darwin_to_libusb (kresult);

  //make sure we have an interface
  //!!Do we really want to try to set the configuration? Setting an already set configuration can make the device do a soft reset!
  if(usbInterface == 0)
  {

      return IOKitError::NOT_FOUND;
	  /*
      uint8_t new_config;
      u_int8_t nConfig;     //Index of configuration to use
      IOUSBConfigurationDescriptorPtr configDesc; //to describe which configuration to select
      //Only a composite class device with no vendor-specific driver will be configured. Otherwise, we need to do it ourselves, or there
      //   will be no interfaces for the device.

      _usbi_log (HANDLE_CTX (dev_handle), LOG_LEVEL_INFO, "no interface found; selecting configuration" );

      kresult = (*(dpriv->device))->GetNumberOfConfigurations(device.usb_device, &nConfig);
      if (kresult != kIOReturnSuccess)
            return darwin_to_libusb(kresult);

      if(nConfig < 1)
      {
         //GetNumberOfConfigurations: no configurations
         return IOKitError::OTHER;
      }

      //Always use the first configuration
      kresult = (*(dpriv->device))->GetConfigurationDescriptorPtr(dpriv->device, 0, &configDesc);
      if (kresult != kIOReturnSuccess)
      {
         _usbi_log (HANDLE_CTX (dev_handle), LOG_LEVEL_ERROR, "GetConfigurationDescriptorPtr: %s",
         darwin_error_str(kresult));

         new_config = 1;
      }
      else
      {
         new_config = configDesc->bConfigurationValue;
      }

      _usbi_log (HANDLE_CTX (dev_handle), LOG_LEVEL_INFO, "new configuration value is %d", new_config);

      //set the configuration
      kresult = darwin_set_configuration (dev_handle, new_config);
      if(kresult != IOKitError::SUCCESS)
      {
         _usbi_log (HANDLE_CTX (dev_handle), LOG_LEVEL_ERROR, "could not set configuration");
         return kresult;
      }

      kresult = darwin_get_interface (dpriv->device, iface, &usbInterface);
      if(kresult)
      {
         _usbi_log (HANDLE_CTX (dev_handle), LOG_LEVEL_ERROR, "darwin_get_interface: %s", darwin_error_str(kresult));
         return darwin_to_libusb (kresult);
      }
      */
  }

  if(!usbInterface)
  {
    //interface not found
    return IOKitError::NOT_FOUND;
  }

  //get an interface to the device's interface
  IOCFPlugInInterface** plugInInterface = NULL;
  long score;
  kresult = IOCreatePlugInInterfaceForService(usbInterface, kIOUSBInterfaceUserClientTypeID,
					       kIOCFPlugInInterfaceID, &plugInInterface, &score);
  if(kresult || !plugInInterface)
  {
	IOObjectRelease (usbInterface);
    return IOKitError::NOT_FOUND;
  }


  //ignore release error
  IOObjectRelease (usbInterface);


  //Do the actual claim
  kresult = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID),
                                                                                    reinterpret_cast<LPVOID*>(&interface));
  if(kresult || !interface)
  {
	(*plugInInterface)->Release(plugInInterface);
    return IOKitError::OTHER;
  }


  //We no longer need the intermediate plug-in
  (*plugInInterface)->Release(plugInInterface);


  //claim the interface
  kresult = (*interface)->USBInterfaceOpen(interface);
  if(kresult)
  {
	this->Release();
    return darwin_to_libusb(kresult);
  }


  //update list of endpoints
  kresult = GetEndpoints();
  if(kresult)
  {
    //this should not happen
    this->Release();
    //could not build endpoint table
    return darwin_to_libusb(kresult);
  }


  //create async event source
  kresult = (*interface)->CreateInterfaceAsyncEventSource(interface, &event_source);
  if(kresult != kIOReturnSuccess)
  {
    //could not create async event source

    //can't continue without an async event source
    this->Release();

    return darwin_to_libusb(kresult);
  }

  //add the cfSource to the async thread's run loop
  CFRunLoopAddSource(dev_handle.run_loop, event_source, kCFRunLoopDefaultMode);  //!!should each interface have its own mode?
  run_loop = dev_handle.run_loop;
  CFRetain(run_loop);

  //interface opened
  bIsClaimed = TRUE;

  return IOKitError::SUCCESS;
}


IOKitError::Enum IOKitInterface::Release()
{

   if(!bIsClaimed)
      return IOKitError::SUCCESS;

   IOReturn kresult;

   //clean up endpoint data
   num_endpoints = 0;

   if(event_source != NULL)
   {
      if(run_loop != 0)
      {
         //delete the interface's async event source
         CFRunLoopRemoveSource(run_loop, event_source, kCFRunLoopDefaultMode);  //!!change the mode if we change it in Claim()
		 CFRelease(run_loop);
      }

      CFRelease(event_source);
   }


   if(interface != IO_OBJECT_NULL)
   {
	  IOReturn close_result;
      close_result = (*interface)->USBInterfaceClose(interface);
      if(close_result != kIOReturnSuccess) {} //error

	  IOReturn release_result;
      release_result = (*interface)->Release(interface);
      if(release_result != kIOReturnSuccess) {} //error

	  //we want the error of the one that fails if one does
	  if(close_result != kIOReturnSuccess)
		kresult = close_result;
	  else
		kresult = release_result;
   }

   interface = IO_OBJECT_NULL;
   bIsClaimed = FALSE;

   return darwin_to_libusb (kresult);
}






//////////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////////


IOReturn IOKitInterface::GetInterface(/*const*/ usb_device_t** darwin_device, int interface_num, io_service_t& usbInterfacep)
{

  kern_return_t             kresult;

  /* Setup the Interface Request */
  IOUSBFindInterfaceRequest request;
  request.bInterfaceClass    = kIOUSBFindInterfaceDontCare;
  request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
  request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
  request.bAlternateSetting  = kIOUSBFindInterfaceDontCare;

   //get interface iterator
  io_iterator_t interface_iterator;
  kresult = (*darwin_device)->CreateInterfaceIterator(darwin_device, &request, &interface_iterator);
  if(kresult)
    return kresult;

   //iterate to interface number and save it
  for(uint8_t current_interface = 0; current_interface <= interface_num; current_interface++)
  {
    if (usbInterfacep != IO_OBJECT_NULL)
       IOObjectRelease(usbInterfacep);

    usbInterfacep = IOIteratorNext(interface_iterator);
  }

  //done with the interface iterator
  IOObjectRelease(interface_iterator);

  return 0;
}


//building table of endpoints.
IOKitError::Enum IOKitInterface::GetEndpoints()
{

  kern_return_t kresult;

  //retrieve the total number of endpoints on this interface
  u_int8_t numep;
  kresult = (*interface)->GetNumEndpoints(interface, &numep);
  if(kresult)
  {
    //can't get number of endpoints for interface
    return darwin_to_libusb (kresult);
  }

  //iterate through pipe references
  for(int i=1; i<=numep; i++)
  {
      u_int8_t direction, number;
      u_int8_t dont_care1, dont_care3;
      u_int16_t dont_care2;

      kresult = (*interface)->GetPipeProperties(interface, i, &direction, &number, &dont_care1, &dont_care2, &dont_care3);
      if(kresult != kIOReturnSuccess)
      {
         //error getting pipe information for pipe
         return darwin_to_libusb (kresult);
      }

      endpoint_addrs[i - 1] = ((direction << 7 & LIBUSB_ENDPOINT_DIR_MASK) | (number & LIBUSB_ENDPOINT_ADDRESS_MASK));
  }

  num_endpoints = numep;

  return IOKitError::SUCCESS;
}

#endif //defined(DSI_TYPES_MACINTOSH)
