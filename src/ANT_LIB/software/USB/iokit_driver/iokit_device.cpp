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

#include "iokit_device.hpp"
#include "macros.h"
#include "iokit_types.hpp"

#include "usb_standard_types.hpp"


//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////


//!!Need to initialize vid and pid


///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
IOKitDevice::IOKitDevice()
:
   usBusNumber(0),
	usAddress(0),
	usConfigurationNum(0),
   ulSerialNumber(0),
	ulLocation(0),
   hSavedService(NULL)
{
   
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';
	memset(&stDeviceDescriptor, 0, sizeof(stDeviceDescriptor));  //!! Can initialize in stDeviceDescriptor constructor instead
	memset(szSystemPath, 0, sizeof(szSystemPath));
   return;
}


IOKitDevice::IOKitDevice(const io_service_t& hService_)
:
   usBusNumber(0),
	usAddress(0),
	usConfigurationNum(0),
   ulSerialNumber(0),
	ulLocation(0),
   hSavedService(NULL)
{
   UCHAR ucSerialIndex = 0;
   
   szProductDescription[0] = '\0';
   szSerialString[0] = '\0';
	memset(&stDeviceDescriptor, 0, sizeof(stDeviceDescriptor));  //!! Can initialize in stDeviceDescriptor constructor instead
	memset(szSystemPath, 0, sizeof(szSystemPath));

   if(hService_ == 0)
      throw IOKitError::INVALID_PARAM;

   if(!IOObjectConformsTo(hService_, "IOUSBDevice"))
      throw IOKitError::INVALID_PARAM;
      
   IOObjectRetain(hService_);
   hSavedService = hService_;
   
   
   //IOKitError::Enum eReturn;
   
   
   
   //!!eReturn = IOKitDevice::GetNewDescriptor(ppstDeviceInterface, stDeviceDescriptor);
   //if(eReturn != IOKitError::SUCCESS)
   //   throw eReturn;
   
   IOKitDevice::GetDeviceString(hService_, CFSTR("USB Product Name"), (UCHAR*)szProductDescription, sizeof(szProductDescription));  //could also use "Product Name" (with a search) to get from drivers

   usVid = IOKitDevice::GetDeviceNumber(hService_, CFSTR("idVendor"));
   usPid = IOKitDevice::GetDeviceNumber(hService_, CFSTR("idProduct"));
   usConfigurationNum = IOKitDevice::GetDeviceNumber(hService_, CFSTR("bNumConfigurations"));
   ulLocation = IOKitDevice::GetDeviceNumber(hService_, CFSTR("locationID"));
   usAddress = IOKitDevice::GetDeviceNumber(hService_, CFSTR("USB Address"));
   usBusNumber = ulLocation >> 24;
   
   UCHAR bDeviceClass = IOKitDevice::GetDeviceNumber(hService_, CFSTR("bDeviceClass"));
   UCHAR bDeviceSubClass = IOKitDevice::GetDeviceNumber(hService_, CFSTR("bDeviceSubClass"));

   ulSerialNumber = IOKitDevice::GetSerialNumber(hService_, (UCHAR*) szSerialString, sizeof(szSerialString), ucSerialIndex);
   stDeviceDescriptor.iSerialNumber = (UInt8)ucSerialIndex;  //update descriptor with serial string index

   SNPRINTF(szSystemPath, sizeof(szSystemPath), "%03i-%04x-%04x-%02x-%02x", usAddress, usVid, usPid, bDeviceClass, bDeviceSubClass);
      
   //!!eReturn = CheckDeviceSanity();
   //if(eReturn != IOKitError::SUCCESS)
   //   throw eReturn;
	
   return;
}

IOKitDevice::IOKitDevice(const IOKitDevice& device)
:
   usBusNumber(device.usBusNumber),
	usAddress(device.usAddress),
   usVid(device.usVid),
   usPid(device.usPid),
	usConfigurationNum(device.usConfigurationNum),
   ulSerialNumber(device.ulSerialNumber),
	ulLocation(device.ulLocation),
   hSavedService(NULL)
{

   if (device.hSavedService)
   {
      IOObjectRetain(device.hSavedService);
      hSavedService = device.hSavedService;
   }

   STRNCPY(szProductDescription, device.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, device.szSerialString, sizeof(szSerialString));
   memcpy(&stDeviceDescriptor, &(device.stDeviceDescriptor), sizeof(device.stDeviceDescriptor));
   memcpy(szSystemPath, device.szSystemPath, sizeof(device.szSystemPath));

   return;
}

///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
IOKitDevice::~IOKitDevice()
{
   //Need to release objects
   if (hSavedService)
      IOObjectRelease(hSavedService);
}


IOKitDevice& IOKitDevice::operator=(const IOKitDevice& clDevice_)
{
   if(this == &clDevice_)
      return *this;
      
   if (hSavedService)
   {
      IOObjectRelease(hSavedService);
      hSavedService = NULL;
   }   

   if (clDevice_.hSavedService)
   {
      IOObjectRetain(clDevice_.hSavedService);
      hSavedService = clDevice_.hSavedService;
   }

   usBusNumber = clDevice_.usBusNumber;
	usAddress = clDevice_.usAddress;
   usVid = clDevice_.usVid;
   usPid = clDevice_.usPid;
	usConfigurationNum = clDevice_.usConfigurationNum;
   ulSerialNumber = clDevice_.ulSerialNumber;
	ulLocation = clDevice_.ulLocation;
    
   STRNCPY(szProductDescription, clDevice_.szProductDescription, sizeof(szProductDescription));
   memcpy(szSerialString, clDevice_.szSerialString, sizeof(szSerialString));
   memcpy(&stDeviceDescriptor, &(clDevice_.stDeviceDescriptor), sizeof(clDevice_.stDeviceDescriptor));
   memcpy(szSystemPath, clDevice_.szSystemPath, sizeof(clDevice_.szSystemPath));

   return *this;
}


/* //!!Do we need this?
BOOL IOKitDevice::Copy(IOKitDevice& dest, const IOKitDevice& source)
{
   dest.device = source.device;
	dest.usBusNumber = source.usBusNumber;
   dest.usAddress = source.usAddress;
   dest.usConfigurationNum = source.usConfigurationNum;
   dest.ulSerialNumber = source.ulSerialNumber;

    //os stuff
   STRNCPY(dest.szProductDescription, source.szProductDescription, sizeof(dest.szProductDescription));
   memcpy(dest.szSerialString, source.szSerialString, sizeof(dest.szSerialString));
   memcpy(&(dest.stDeviceDescriptor), &(source.stDeviceDescriptor), sizeof(dest.stDeviceDescriptor));
   dest.ulLocation = source.ulLocation;
   memcpy(dest.szSystemPath, source.szSystemPath, sizeof(dest.szSystemPath));

	return TRUE;
}
*/




ULONG IOKitDevice::GetDeviceNumber(const io_service_t& hService_, CFStringRef hProperty_)
{
   CFNumberRef hNumber = (CFNumberRef)IORegistryEntryCreateCFProperty(hService_,
                                           hProperty_,
                                           kCFAllocatorDefault,
                                           0);
   if(hNumber == 0)
      return 0;

   long lNumber;
   CFNumberGetValue(hNumber, kCFNumberLongType, &lNumber);

   CFRelease(hNumber);

   return (ULONG)lNumber;
}


BOOL IOKitDevice::GetDeviceString(const io_service_t& hService_, CFStringRef hProperty_, UCHAR* pucBsdName_, ULONG ulSize_, BOOL bSearchChildren_)
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

ULONG IOKitDevice::GetSerialNumber(const io_service_t& hService_, UCHAR* pucSerialString_, ULONG ulSize_, UCHAR& ucSNIndex)
{
   BOOL bResult;
    CFStringRef	hBsdPathAsCFString;  //CFTypeRef	hBsdPathAsCFString;
   CFNumberRef hCFSerialNumberIndex;

   // get serial string index
   hCFSerialNumberIndex = (CFNumberRef)IORegistryEntryCreateCFProperty(hService_,
                                                            CFSTR(kUSBSerialNumberStringIndex),
                                                            kCFAllocatorDefault,
                                                            0);

   if(hCFSerialNumberIndex == NULL)
      return 0;

   // Convert handle value to byte
   bResult = CFNumberGetValue(hCFSerialNumberIndex, kCFNumberCharType, &ucSNIndex);
   CFRelease(hCFSerialNumberIndex);

   if (!bResult)
      return 0;

   // get serial number
	hBsdPathAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(hService_,
                                                            CFSTR("USB Serial Number"),
                                                            kCFAllocatorDefault,
                                                            0);

   if(hBsdPathAsCFString == NULL)
		return 0;


    // Convert the path from a CFString to a C (NUL-terminated) string
   char acSerialNumber[255];
	bResult = CFStringGetCString(hBsdPathAsCFString,
                                 acSerialNumber,
                                 sizeof(acSerialNumber),
                                 kCFStringEncodingUTF8);
    CFRelease(hBsdPathAsCFString);

   if(!bResult)
		return 0;
		
   memcpy(pucSerialString_, acSerialNumber, ulSize_);
   return atol(acSerialNumber);
}


//Gets a new device descriptor.
//Is also used to see if we can talk to the device.
IOKitError::Enum IOKitDevice::GetNewDescriptor(usb_device_t** device, IOUSBDeviceDescriptor& stDeviceDescriptor)
{

    //Set up request for device descriptor
	IOUSBDevRequest      req;
   req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
   req.bRequest      = kUSBRqGetDescriptor;
   req.wValue        = kUSBDeviceDesc << 8;
   req.wIndex        = 0;
   req.wLength       = sizeof(IOUSBDeviceDescriptor);
   req.pData         = &(stDeviceDescriptor);


   
   //// retrieve device descriptors ////
   //device must be open for DeviceRequest
   (*device)->USBDeviceOpen(device);

   
   IOReturn ret = (*device)->DeviceRequest(device, &req);
   if(ret != kIOReturnSuccess)
   {
      int try_unsuspend = 1;

      #if (DeviceVersion >= 320)
      {
         UInt32 info;

         //device may be suspended. unsuspend it and try again
         //IOUSBFamily 320+ provides a way to detect device suspension but earlier versions do not
         (*device)->GetUSBDeviceInformation(device, &info);

         try_unsuspend = info & (1 << kUSBInformationDeviceIsSuspendedBit);
      }
      #endif

      if(try_unsuspend)
      {
         //resume the device
         (*device)->USBDeviceSuspend(device, 0);

         ret = (*device)->DeviceRequest(device, &req);

         //resuspend the device
         (*device)->USBDeviceSuspend(device, 1);
      }
   }
   
   /*
   //!!Would it just be easier to get this information from the IORegistry?
	//Set up request for product description
    IOUSBDevRequest     product_desc_req;
    string_descriptor productDescription;
    product_desc_req.bmRequestType = LIBUSB_ENDPOINT_IN;
    product_desc_req.bRequest      = LIBUSB_REQUEST_GET_DESCRIPTOR;
    product_desc_req.wValue        = (LIBUSB_DT_STRING << 8) | stDeviceDescriptor.iSerialNumber;
    product_desc_req.wIndex        = 0x409;
    product_desc_req.wLength       = sizeof(productDescription);
    product_desc_req.pData         = &productDescription;

   UCHAR aucBlah[255];

	IOReturn product_desc_ret = (*device)->DeviceRequest(device, &product_desc_req);  //!!Is this guaranteed to be null-terminated?
	if(product_desc_ret == kIOReturnSuccess)
   {
      //Convert from unicode to ascii
      int si, di;
      int length = sizeof(productDescription);
      for(di = 0, si = 0; si < productDescription.length; di++, si+=2)
      {
         if(di >= (length - 1))
            break;

         if(productDescription.string[si] == '\0')
         {
            aucBlah[di] = '\0';
            break;
         }

         if(productDescription.string[si + 1]) //high byte
            aucBlah[di] = '?';
         else
            aucBlah[di] = productDescription.string[si];
      }
      aucBlah[di] = '\0';

   }
   else
   {
		aucBlah[0] = '\0';
   }
   */
   
   

   (*device)->USBDeviceClose(device);
	if(ret != kIOReturnSuccess)
	{
      //could not retrieve device descriptor
      return darwin_to_libusb(ret);
   }
   //// end: retrieve device descriptors ////

	return darwin_to_libusb(ret);
}




/* Perform some final sanity checks on a newly discovered device. If this
 * function fails (negative return code), the device should not be added
 * to the discovered device list. */
IOKitError::Enum IOKitDevice::CheckDeviceSanity()
{

/*  This was not being used and cannot be done this way as we no longer create a device interface in the constructor, updated if needed later.
   USHORT idProduct;
   (*ppstDeviceInterface)->GetDeviceProduct(ppstDeviceInterface, &idProduct);
   
   //catch buggy hubs (which appear to be virtual). Apple's own USB prober has problems with these devices.
   if(stDeviceDescriptor.idProduct != idProduct)
	{
      //not a valid device
      //idProduct from iokit does not match idProduct in descriptor. skipping device
      return IOKitError::OTHER;
   }

	uint8_t localusConfigurationNum = stDeviceDescriptor.bNumConfigurations;
	if(localusConfigurationNum > IOKitDevice::MAX_CONFIG)
	{
		//too many configurations
		return IOKitError::IO;
	}
	else if(localusConfigurationNum < 1)
	{
		//no configurations?
		return IOKitError::IO;
	}
*/
	return IOKitError::SUCCESS;
}



usb_device_t** IOKitDevice::CreateDeviceInterface(const io_service_t& hService_)
{
   io_cf_plugin_ref_t* plugInInterface = NULL;
   long score;
   IOReturn result = IOCreatePlugInInterfaceForService(hService_, kIOUSBDeviceUserClientTypeID,
					     kIOCFPlugInInterfaceID, &plugInInterface,
					     &score);

  if(result || !plugInInterface)
    return NULL;


  usb_device_t **device;
  (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(DeviceInterfaceID), reinterpret_cast<LPVOID*>(&device));

  (*plugInInterface)->Stop(plugInInterface);
  IODestroyPlugInInterface(plugInInterface);

   return device;
}










/* //This is the way the libusb code got its device information
IOKitError::Enum IOKitDeviceList::AddNewDevice(/*const*//* usb_device_t** device, IOKitDevice& dev)
{

    //Set up request for device descriptor
	IOUSBDevRequest      req;
	UInt16                address, idVendor, idProduct;
	UInt8                 bDeviceClass, bDeviceSubClass;
    req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
    req.bRequest      = kUSBRqGetDescriptor;
    req.wValue        = kUSBDeviceDesc << 8;
    req.wIndex        = 0;
    req.wLength       = sizeof(IOUSBDeviceDescriptor);
    req.pData         = &(dev.dev_descriptor);

    (*device)->GetDeviceAddress(device, (USBDeviceAddress*)&address);
    (*device)->GetDeviceProduct(device, &idProduct);
    (*device)->GetDeviceVendor(device, &idVendor);
    (*device)->GetDeviceClass(device, &bDeviceClass);
    (*device)->GetDeviceSubClass(device, &bDeviceSubClass);

    //// retrieve device descriptors ////
    //device must be open for DeviceRequest
    (*device)->USBDeviceOpen(device);


    IOReturn ret = (*device)->DeviceRequest(device, &req);
    if(ret != kIOReturnSuccess)
	{
      int try_unsuspend = 1;

	  #if (DeviceVersion >= 320)
	  {
		UInt32 info;

		//device may be suspended. unsuspend it and try again
		//IOUSBFamily 320+ provides a way to detect device suspension but earlier versions do not
		(*device)->GetUSBDeviceInformation(device, &info);

		try_unsuspend = info & (1 << kUSBInformationDeviceIsSuspendedBit);
	  }
	  #endif

      if (try_unsuspend)
	  {
		//resume the device
		(*device)->USBDeviceSuspend(device, 0);

		ret = (*device)->DeviceRequest(device, &req);

		//resuspend the device
		(*device)->USBDeviceSuspend(device, 1);
      }
    }


	//Set up request for serial number descriptor
	IOUSBDevRequest     serial_num_req;
	char serial_number_str[255];
    serial_num_req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
    serial_num_req.bRequest      = kUSBRqGetDescriptor;
    serial_num_req.wValue        = (kUSBStringDesc << 8) | dev.dev_descriptor.iSerialNumber;
    serial_num_req.wIndex        = 0x409;  //english
    serial_num_req.wLength       = sizeof(serial_number_str);
    serial_num_req.pData         = serial_number_str;

	IOReturn serial_number_ret = (*device)->DeviceRequest(device, &serial_num_req);
	if(serial_number_ret == kIOReturnSuccess)
		dev.serial_number = atol(serial_number_str);
	else
		dev.serial_number = 0;


   //!!Would it just be easier to get this information from the IORegistry?
	//Set up request for product description
    IOUSBDevRequest     product_desc_req;
    string_descriptor productDescription;
    product_desc_req.bmRequestType = LIBUSB_ENDPOINT_IN;
    product_desc_req.bRequest      = LIBUSB_REQUEST_GET_DESCRIPTOR;
    product_desc_req.wValue        = (LIBUSB_DT_STRING << 8) | dev.dev_descriptor.iProduct;
    product_desc_req.wIndex        = 0x409;
    product_desc_req.wLength       = sizeof(productDescription);
    product_desc_req.pData         = &productDescription;

	IOReturn product_desc_ret = (*device)->DeviceRequest(device, &product_desc_req);  //!!Is this guaranteed to be null-terminated?
	if(product_desc_ret == kIOReturnSuccess)
   {
      //Convert from unicode to ascii
      int si, di;
      int length = sizeof(dev.product_description);
      for(di = 0, si = 0; si < productDescription.length; di++, si+=2)
      {
         if(di >= (length - 1))
            break;

         if(productDescription.string[si] == '\0')
         {
            dev.product_description[di] = '\0';
            break;
         }

         if(productDescription.string[si + 1]) //high byte
            dev.product_description[di] = '?';
         else
            dev.product_description[di] = productDescription.string[si];
      }
      dev.product_description[di] = '\0';

   }
   else
   {
		dev.product_description[0] = '\0';
   }



   /*
	IOUSBDevRequest     product_desc_req;
    product_desc_req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
    product_desc_req.bRequest      = kUSBRqGetDescriptor;
    product_desc_req.wValue        = kUSBDeviceDesc << 8;
    product_desc_req.wIndex        = dev.dev_descriptor.iProduct;
    product_desc_req.wLength       = sizeof(dev.product_description);
    product_desc_req.pData         = dev.product_description;

	IOReturn product_desc_ret = (*device)->DeviceRequest(device, &product_desc_req);  //!!Is this guaranteed to be null-terminated?
	if(product_desc_ret != kIOReturnSuccess)
		dev.product_description[0] = '\0';
	*//*

    (*device)->USBDeviceClose(device);
	if(ret != kIOReturnSuccess)
	{
      //could not retrieve device descriptor
      return darwin_to_libusb(ret);
    }
    //// end: retrieve device descriptors ////

    //catch buggy hubs (which appear to be virtual). Apple's own USB prober has problems with these devices.
    if (libusb_le16_to_cpu(dev.dev_descriptor.idProduct) != idProduct)
	{
      //not a valid device
      //idProduct from iokit does not match idProduct in descriptor. skipping device
      return IOKitError::OTHER;
    }

   dev.device = device;
    dev.bus_number     = dev.GetLocation() >> 24;
    dev.device_address = address;

    snprintf(dev.sys_path, 20, "%03i-%04x-%04x-%02x-%02x", address, idVendor, idProduct, bDeviceClass, bDeviceSubClass);

    ret = SanitizeDevice(dev);

	return darwin_to_libusb(ret);
}
*/

#endif //defined(DSI_TYPES_MACINTOSH)
