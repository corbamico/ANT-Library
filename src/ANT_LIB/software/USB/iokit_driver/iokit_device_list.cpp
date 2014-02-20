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

#include "iokit_device_list.hpp"

#include "iokit_device.hpp"

#include "usb_standard_types.hpp"


#include <config.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <mach/clock.h>
#include <mach/clock_types.h>
#include <mach/mach_host.h>

#include <mach/mach_port.h>
#include <IOKit/IOCFBundle.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>


#include <vector>

using namespace std;


//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////

#define SI_LABS_DRIVER_CLASS  "SiLabsUSBDriver"

//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////


IOKitError::Enum IOKitDeviceList::GetDeviceList(vector<IOKitDevice*>& dev_list)
{
	FreeDeviceList(dev_list);
	return MakeDeviceList(dev_list);
}

void IOKitDeviceList::FreeDeviceList(vector<IOKitDevice*>& dev_list)
{
	//destroy all devices
	vector<IOKitDevice*>::iterator i;
	for(i = dev_list.begin(); i != dev_list.end(); i++)
        {      
           delete *i;
        }

	dev_list.clear();

	return;
}

IOKitError::Enum IOKitDeviceList::MakeDeviceList(vector<IOKitDevice*>& dev_list)
{


  IOReturn kresult;

  //set up device iterator
  io_iterator_t        deviceIterator;
  kresult = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(kIOUSBDeviceClassName), &deviceIterator);
  if(kresult != kIOReturnSuccess)
    return darwin_to_libusb(kresult);
   
   
  io_service_t device;
  while((device = GetNextDevice(deviceIterator)) != 0)
  {
      try
      {
         IOKitDevice* dev;
         dev = new IOKitDevice(device);
         dev_list.push_back(dev); 
      }
      catch(...) {}
      IOObjectRelease(device);
  }

  IOObjectRelease(deviceIterator);

  return IOKitError::SUCCESS;
}


io_service_t IOKitDeviceList::GetNextDevice(io_iterator_t deviceIterator)
{
   io_service_t usbDevice = NULL;

   BOOL bSiLabsDevice;
   do
   {
      bSiLabsDevice = FALSE;

      if (usbDevice)
         IOObjectRelease(usbDevice);

      if (!IOIteratorIsValid (deviceIterator) || !(usbDevice = IOIteratorNext(deviceIterator)))
         return NULL;

      //Check to see if this is an SiLabs device
      io_iterator_t childIterator;
      if(IORegistryEntryGetChildIterator(usbDevice, kIOServicePlane, &childIterator) == kIOReturnSuccess)
      {
         io_service_t usbChild = IOIteratorNext(childIterator);
         while(usbChild && !bSiLabsDevice)
         {
            io_name_t name;
            if(IOObjectGetClass(usbChild, name) == kIOReturnSuccess)  //can also use IORegistryEntryGetName(usbChild, name)
            {
               if(strcmp(name, SI_LABS_DRIVER_CLASS) == 0)
                  bSiLabsDevice = TRUE;
            }
            IOObjectRelease(usbChild);
            usbChild = IOIteratorNext(childIterator);
         }

         IOObjectRelease(usbChild);
         IOObjectRelease(childIterator);
      }

   } while(bSiLabsDevice);  //we want to ignore silabs devices

   
   return usbDevice;
}

#endif //defined(DSI_TYPES_MACINTOSH)
