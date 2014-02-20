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

#include "usb_device_vcp.hpp"

#include "macros.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>


//!!Need to make sure there are no memory leaks!

USBDeviceSI::USBDeviceSI(const io_service_t& hService_)  //hService_ should be a IOUSBDevice class
:
   usVid(0),
   usPid(0),
   ulSerialNumber(0),
   ulLocation(0),
   ppstDeviceInterface(NULL)
{
   //initailize array
   szBsdName[0] = '\0';
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';

   if(hService_ == 0 || !IOObjectConformsTo(hService_, "IOUSBDevice"))
      return;  //throw exception?

   IOObjectRetain(hService_);


   //Get BSD name
 	// Get the callout device's path (/dev/cu.xxxxx). The callout device should almost always be
	// used: the dialin device (/dev/tty.xxxxx) would be used when monitoring a serial port for
	// incoming calls, e.g. a fax listener.
   USBDeviceSI::GetDeviceString(hService_, CFSTR(kIOCalloutDeviceKey), szBsdName, sizeof(szBsdName), TRUE);  //throw exception if this fails?

   USBDeviceSI::GetDeviceString(hService_, CFSTR("USB Product Name"), szProductDescription, sizeof(szProductDescription));  //could also use "Product Name" (with a search) to get from drivers

   usVid = USBDeviceSI::GetDeviceNumber(hService_, CFSTR("idVendor"));
   usPid = USBDeviceSI::GetDeviceNumber(hService_, CFSTR("idProduct"));
   ulLocation = USBDeviceSI::GetDeviceNumber(hService_, CFSTR("locationID"));
   ulSerialNumber = USBDeviceSI::GetSerialNumber(hService_, szSerialString, sizeof(szSerialString));

   ppstDeviceInterface = CreateDeviceInterface(hService_);
   
   return;
}


USBDeviceSI::USBDeviceSI(UCHAR* pucBsdName_)
:
   usVid(0),
   usPid(0),
   ulSerialNumber(0),
   ulLocation(0),
   ppstDeviceInterface(NULL)
{
   //initialize array
   szBsdName[0] = '\0';
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';

   STRNCPY((char*)szBsdName, (char*)pucBsdName_, sizeof(szBsdName));

   //!!Could do a search in the IORegistry to get device information

   return;
}

USBDeviceSI::USBDeviceSI(const USBDeviceSI& clDevice_)
:
   usVid(clDevice_.usVid),
   usPid(clDevice_.usPid),
   ulSerialNumber(clDevice_.ulSerialNumber),
   ulLocation(clDevice_.ulLocation),
   ppstDeviceInterface(clDevice_.ppstDeviceInterface)
{
   //initialize array
   szBsdName[0] = '\0';
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';

   STRNCPY((char*)szBsdName, (char*)clDevice_.szBsdName, sizeof(szBsdName));
   STRNCPY((char*)szProductDescription, (char*)clDevice_.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));
   return;
}



USBDeviceSI& USBDeviceSI::operator=(const USBDeviceSI& clDevice_)
{
   if(this == &clDevice_)
      return *this;

   usVid = clDevice_.usVid;
   usPid = clDevice_.usPid;
   ulSerialNumber = clDevice_.ulSerialNumber;
   ulLocation = clDevice_.ulLocation;
   ppstDeviceInterface = clDevice_.ppstDeviceInterface;

   STRNCPY((char*)szBsdName, (char*)clDevice_.szBsdName, sizeof(szBsdName));
   STRNCPY((char*)szProductDescription, (char*)clDevice_.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));
   return *this;
}



BOOL USBDeviceSI::USBReset() const
{
/*  Removed while evaluating long term stability issues
   if(ppstDeviceInterface == NULL)
      return FALSE;

   (*ppstDeviceInterface)->USBDeviceOpen(ppstDeviceInterface);
   
   if( (*ppstDeviceInterface)->USBDeviceReEnumerate(ppstDeviceInterface, 0) != kIOReturnSuccess)
      return FALSE;
      
   //!!Release?
*/   
   return TRUE;
}







ULONG USBDeviceSI::GetDeviceNumber(const io_service_t& hService_, CFStringRef hProperty_)
{
   CFNumberRef hNumber = (CFNumberRef)IORegistryEntryCreateCFProperty(hService_,
                                           hProperty_,
                                           kCFAllocatorDefault,
                                           0);
   if(hNumber == 0)
      return 0;

   long lNumber;
   CFNumberGetValue(hNumber, kCFNumberLongType, &lNumber);
   return (ULONG)lNumber;
}


BOOL USBDeviceSI::GetProductDescription(UCHAR* pucProductDescription_, USHORT usBufferSize_) const
{
   return(STRNCPY((char*) pucProductDescription_, (char*) szProductDescription, usBufferSize_));
}


BOOL USBDeviceSI::GetSerialString(UCHAR* pucSerialString_, USHORT usBufferSize_) const
{
   if(sizeof(szSerialString) > usBufferSize_)
   {
      memcpy(pucSerialString_, szSerialString, usBufferSize_);
      return FALSE;
   }

   memcpy(pucSerialString_, szSerialString, sizeof(szSerialString));
   return TRUE;
}


ULONG USBDeviceSI::GetSerialNumber(const io_service_t& hService_, UCHAR* pucSerialString_, ULONG ulSize_)
{
    CFStringRef	hBsdPathAsCFString;  //CFTypeRef	hBsdPathAsCFString;

	hBsdPathAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(hService_,
                                                            CFSTR("USB Serial Number"),
                                                            kCFAllocatorDefault,
                                                            0);

   if(hBsdPathAsCFString == NULL)
		return 0;


    // Convert the path from a CFString to a C (NUL-terminated) string
   char acSerialNumber[255];
	BOOL bResult = CFStringGetCString(hBsdPathAsCFString,
                                 acSerialNumber,
                                 sizeof(acSerialNumber),
                                 kCFStringEncodingUTF8);
    CFRelease(hBsdPathAsCFString);

   if(!bResult)
		return 0;

   memcpy(pucSerialString_, acSerialNumber, ulSize_);
   return atol(acSerialNumber);
}

BOOL USBDeviceSI::GetDeviceString(const io_service_t& hService_, CFStringRef hProperty_, UCHAR* pucBsdName_, ULONG ulSize_, BOOL bSearchChildren_)
{
   if(pucBsdName_ == NULL)
      return FALSE;

   CFStringRef	hBsdPathAsCFString;  //CFTypeRef	hBsdPathAsCFString;

   if(bSearchChildren_)
   {
      hBsdPathAsCFString = (CFStringRef)IORegistryEntrySearchCFProperty(
                                                            hService_,
                                                            kIOServicePlane,
                                                            hProperty_,
                                                            kCFAllocatorDefault,
                                                            kIORegistryIterateRecursively);
   }
   else
   {
      hBsdPathAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(
                                                            hService_,
                                                            hProperty_,
                                                            kCFAllocatorDefault,
                                                            0);
   }

   if(hBsdPathAsCFString == NULL)
   {
      pucBsdName_[0] = '\0';
		return FALSE;
   }


    // Convert the path from a CFString to a C (NUL-terminated) string for use
	// with the POSIX open() call.
	BOOL bResult = CFStringGetCString(hBsdPathAsCFString,
                                 (char*)pucBsdName_,
                                 ulSize_,
                                 kCFStringEncodingUTF8);
    CFRelease(hBsdPathAsCFString);

   if(!bResult)
   {
      pucBsdName_[0] = '\0';
		return FALSE;
   }

   return TRUE;
}


usb_device_t** USBDeviceSI::CreateDeviceInterface(const io_service_t& hService_)
{
//Removied IOCreatePlugInInterfaceForService for VCP to try to prevent crashing
/*   io_cf_plugin_ref_t* plugInInterface = NULL;
   long score;
   IOReturn result = IOCreatePlugInInterfaceForService(hService_, kIOUSBDeviceUserClientTypeID,
					     kIOCFPlugInInterfaceID, &plugInInterface,
					     &score);

  if(result || !plugInInterface)*/
    return NULL;

/*
  usb_device_t **device;
  (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(DeviceInterfaceID), reinterpret_cast<LPVOID*>(&device));

  (*plugInInterface)->Stop(plugInInterface);
  IODestroyPlugInInterface(plugInInterface);

   return device;*/
}


#endif //defined(DSI_TYPES_MACINTOSH)
