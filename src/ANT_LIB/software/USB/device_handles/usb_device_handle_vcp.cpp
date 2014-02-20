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
#include "usb_device_handle_vcp.hpp"
#include "macros.h"
#include "dsi_debug.hpp"

#include "usb_device_list.hpp"



#include "dsi_thread.h"
#include "dsi_vcp_si.hpp"


#include <termios.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <paths.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <AvailabilityMacros.h>

#include <memory>


#ifdef __MWERKS__
#define __CF_USE_FRAMEWORK_INCLUDES__
#endif
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOKitLib.h>


#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
#include <IOKit/serial/ioss.h>
#endif
#include <IOKit/IOBSD.h>


using namespace std;

//#define NUMBER_OF_DEVICES_CHECK_THREAD_KILL     //Only include this if we are sure we want to terminate the Rx thread if the number of devices are ever reduced.

//////////////////////////////////////////////////////////////////////////////////
// Static declarations
//////////////////////////////////////////////////////////////////////////////////

USBDeviceList<const USBDeviceSI> USBDeviceHandleSI::clDeviceList;


//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////

//Note: hService_ will be released when passed into this function, the calling function is responsible for releasing the returned io_service
namespace 
{
  io_service_t GetRegistryEntry(io_service_t hService_, const char* pcClassName_)  //!!Make member function
  {
     if(hService_ == 0 || pcClassName_ == NULL)
     {
        IOObjectRelease(hService_);
        return 0;
     }
  
     if(IOObjectConformsTo(hService_, pcClassName_))
        return hService_;
        
     io_service_t hParentService;
     if(IORegistryEntryGetParentEntry(hService_, kIOServicePlane, &hParentService) != kIOReturnSuccess)
     {
        IOObjectRelease(hService_);
        return 0;
     }
  
     IOObjectRelease(hService_);
     
     return GetRegistryEntry(hParentService, pcClassName_);
  }
}


//////////////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////////////

const USBDeviceListSI USBDeviceHandleSI::GetAllDevices()
{
   USBDeviceListSI clList;
   
   clDeviceList = USBDeviceList<const USBDeviceSI>();  //clear device list



	io_iterator_t	hSerialPortIterator;

	//Make a dictionary of all modems
    if(FindModems(&hSerialPortIterator) == FALSE)
		return clList;


	//Find the proper device in the dictionary
   io_service_t hService;
   io_service_t hRegistryEntry;
	while( (hService = IOIteratorNext(hSerialPortIterator)) != 0)
	{
      hRegistryEntry = GetRegistryEntry(hService, "IOUSBDevice");               //no need to release hService, GetRegistryEntry will do it for us.
      if(hRegistryEntry != 0)
      {
         clDeviceList.Add(USBDeviceSI(hRegistryEntry) );  //save the copies to the static private list
         clList.Add(clDeviceList.GetAddress(clDeviceList.GetSize() - 1) );  //save a pointer to the just added device
         IOObjectRelease(hRegistryEntry);
      }
	}

	// Release the iterator.
   IOObjectRelease(hSerialPortIterator);

	return clList;
}


static BOOL CanOpenDevice(const USBDeviceSI*const & pclDevice_)  //!!Make member function
{
   if(pclDevice_ == FALSE)
      return FALSE;

   return USBDeviceHandleSI::TryOpen(*pclDevice_);
}

const USBDeviceListSI USBDeviceHandleSI::GetAvailableDevices()
{
   return USBDeviceHandleSI::GetAllDevices().GetSubList(CanOpenDevice);
}



///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
USBDeviceHandleSI::USBDeviceHandleSI(const USBDeviceSI& clDevice_, ULONG ulBaudRate_)
try
:
   hSerialFileDescriptor(-1),
   USBDeviceHandle(),
   clDevice(clDevice_),
   ulBaudRate(ulBaudRate_),
   bDeviceGone(TRUE),
 
   thread_started(FALSE),
   thread(NULL),
   run_loop(NULL),
   
   hReceiveThread(NULL),
   bStopReceiveThread(TRUE)
{
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
USBDeviceHandleSI::~USBDeviceHandleSI()
{
   //!!Delete all the elements in the queue?
}



//A more efficient way to test if you can open a device.  For instance, this function won't create a receive loop, etc.)
//Doesn't test for correct baudrate.
BOOL USBDeviceHandleSI::TryOpen(const USBDeviceSI& clDevice_)
{

   // Open the serial port.
   // See open(2) ("man 2 open") for details.
   int hSerialFileDescriptor = open((char*)(clDevice_.GetBsdName()), O_RDWR | O_NOCTTY | O_NDELAY | O_EXLOCK); //O_NONBLOCK
   if(hSerialFileDescriptor == -1)
      return FALSE;
      
   close(hSerialFileDescriptor);
   return TRUE;
}


///////////////////////////////////////////////////////////////////////
// Opens port, starts receive thread.
///////////////////////////////////////////////////////////////////////
BOOL USBDeviceHandleSI::Open(const USBDeviceSI& clDevice_, USBDeviceHandleSI*& pclDeviceHandle_, ULONG ulBaudRate_)
{
   try
   {
      pclDeviceHandle_ = new USBDeviceHandleSI(clDevice_, ulBaudRate_);
   }
   catch(...)
   {
      pclDeviceHandle_ = NULL;
      return FALSE;
   }

   return TRUE;
}


BOOL USBDeviceHandleSI::POpen()
{
   // Make sure all handles are reset before opening again.
   PClose();

   // Open the serial port.
   // See open(2) ("man 2 open") for details.
   hSerialFileDescriptor = open((const char*)(clDevice.GetBsdName()), O_RDWR | O_NOCTTY | O_NDELAY | O_EXLOCK); //O_NONBLOCK
   if(hSerialFileDescriptor == -1)
      return FALSE;
   
   bDeviceGone = FALSE;
   
   //start the notification thread for surprise removal
   if(pthread_mutex_init(&lock_runloop, NULL) != 0)
   {
      PClose();
      return FALSE;
   }
   
   if(pthread_cond_init(&cond_runloop, NULL) != 0)
   {
      pthread_mutex_destroy(&lock_runloop);
      PClose();
      return FALSE;
   }
   
   thread_started = FALSE;
   
   pthread_mutex_lock(&lock_runloop);
   if(pthread_create(&thread, NULL, &USBDeviceHandleSI::EventThreadStart, this) != 0)
   {
      pthread_mutex_unlock(&lock_runloop);
      pthread_mutex_destroy(&lock_runloop);
      pthread_cond_destroy(&cond_runloop);
      PClose();
      return FALSE;
   }

   //wait for thread to signal back
   pthread_cond_wait(&cond_runloop, &lock_runloop);  //!!have a timeout
   
   if(thread_started)
      CFRetain(run_loop);
   else
      run_loop = NULL;  // run loop reference is no longer valid
   
   pthread_mutex_unlock(&lock_runloop);
      
      

   // Get the current options and save them so we can restore the default settings later.
   if (tcgetattr(hSerialFileDescriptor, &stOriginalTTYAttrs) == -1)
   {
      PClose();
      return FALSE;
   }

	struct termios options;
	options = stOriginalTTYAttrs;

	// The serial port attributes such as timeouts and baud rate are set by modifying the termios
   // structure and then calling tcsetattr() to cause the changes to take effect. Note that the
   // changes will not become effective without the tcsetattr() call.
   // See tcsetattr(4) ("man 4 tcsetattr") for details.


	//Clear termios attribute flags
	options.c_iflag = 0;
	options.c_oflag = 0;
	options.c_cflag = 0;
	options.c_lflag = 0;

	// Set raw input (non-canonical) mode, with reads never blocking.
   // See tcsetattr(4) ("man 4 tcsetattr") and termios(4) ("man 4 termios") for details.
	options.c_cc[VMIN] = 0;		//minimum number of characters for non-canonical read (will block until this many characters have been received)
	options.c_cc[VTIME] = 0;	//timeout in deciseconds for non-canonical read (how long read will block)




	// The baud rate, word length, and handshake options can be set as follows:
	cfsetspeed(&options, B115200);	// Set input/output baud
	options.c_cflag |=  (CS8	|	// Use 8 bit words
						 CREAD	|	// Enable receiver
						 CLOCAL	|	// Ignore modem control lines
						 CRTSCTS);	// Enable RTS/CTS flow control (not POSIX)

	//Set the attributes
	//TCSANOW - changes occur immediately
	if (tcsetattr(hSerialFileDescriptor, TCSANOW, &options) == -1)
   {
      PClose();
      return FALSE;
   }



	/* Set arbitrary baud rate
	#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
	{
		// Starting with Tiger, the IOSSIOSPEED ioctl can be used to set arbitrary baud rates
		// other than those specified by POSIX. The driver for the underlying serial hardware
		// ultimately determines which baud rates can be used. This ioctl sets both the input
		// and output speed.
		speed_t speed = 14400; // Set 14400 baud
		if (ioctl(hSerialFileDescriptor, IOSSIOSPEED, &speed) == -1)
		{
			Close();
			return FALSE;
		}
	}
	#endif
    */

	/* Handshaking
    int handshake;

	//Set the attributes
	//TCSANOW - changes occur immediately
	if (tcsetattr(hSerialFileDescriptor, TCSANOW, &options) == -1)
    {
        Close();
        return FALSE;
    }

    // To set the modem handshake lines, use the following ioctls.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    if (ioctl(hSerialFileDescriptor, TIOCSDTR) == -1) // Assert Data Terminal Ready (DTR)
    {
		Close();
		return FALSE;
    }

    if (ioctl(hSerialFileDescriptor, TIOCCDTR) == -1) // Clear Data Terminal Ready (DTR)
    {
		Close();
		return FALSE;
    }

    handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;
    if (ioctl(hSerialFileDescriptor, TIOCMSET, &handshake) == -1)
    // Set the modem lines depending on the bits set in handshake
    {
		Close();
		return FALSE;
	}

    printf("Handshake lines currently set to %d\n", handshake);
	*/

	/* Read Latency
	#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
	{
		ULONG ulMicroseconds = 1;

		// Set the receive latency in microseconds. Serial drivers use this value to determine how often to
		// dequeue characters received by the hardware. Most applications don't need to set this value: if an
		// app reads lines of characters, the app can't do anything until the line termination character has been
		// received anyway. The most common applications which are sensitive to read latency are MIDI and IrDA
		// applications.

		// set latency to 1 microsecond
		if (ioctl(hSerialFileDescriptor, IOSSDATALAT, &ulMicroseconds) == -1)
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
   hReceiveThread = DSIThread_CreateThread(&USBDeviceHandleSI::ProcessThread, this);
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
BOOL USBDeviceHandleSI::Close(USBDeviceHandleSI*& pclDeviceHandle_, BOOL bReset_)
{
   if(pclDeviceHandle_ == NULL)
      return FALSE;

   pclDeviceHandle_->PClose(bReset_);
   delete pclDeviceHandle_;
   pclDeviceHandle_ = NULL;
   
   return TRUE;
}

void USBDeviceHandleSI::PClose(BOOL bReset_)
{
   
   bDeviceGone = TRUE;

   if (hReceiveThread)
   {
      DSIThread_MutexLock(&stMutexCriticalSection);
      if(bStopReceiveThread == FALSE)
      {
         bStopReceiveThread = TRUE;

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


   //stop notification thread
   if(run_loop != NULL)
	{
		//stop the async runloop
		CFRunLoopStop(run_loop);
      CFRelease(run_loop);
      run_loop = NULL;
	}

   if(thread != NULL)
   {
      if(thread_started == TRUE)
      {
         void *ret;
         pthread_join(thread, &ret);
         thread_started = FALSE;
      }
      pthread_cond_destroy(&cond_runloop);
      pthread_mutex_destroy(&lock_runloop);
   }


   //Close usb device
   if(hSerialFileDescriptor != -1)
    {

      if(bReset_)
      {
         usb_device_t** ppstDeviceInterface = clDevice.GetDeviceInterface();
         if(ppstDeviceInterface != NULL)
            (*ppstDeviceInterface)->ResetDevice(ppstDeviceInterface);   //(*ppstDeviceInterface)->USBDeviceReEnumerate(ppstDeviceInterface, 0);
      }

		// Block until all written output has been sent from the device.
		// Note: tcflush() discards data on the queue selected while tcdrain() blocks until
		//       written data has been transmitted.
		// See tcsendbreak(3) ("man 3 tcsendbreak") for details.
		tcdrain(hSerialFileDescriptor);

		// Traditionally it is good practice to reset a serial port back to
		// the state in which you found it. This is why the original termios struct
		// was saved.
		tcsetattr(hSerialFileDescriptor, TCSANOW, &stOriginalTTYAttrs);

		close(hSerialFileDescriptor);
		hSerialFileDescriptor = -1;
    }
}

///////////////////////////////////////////////////////////////////////
// Writes ucSize_ bytes to USB, returns number of bytes not written.
///////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleSI::Write(void* pvData_, ULONG ulSize_, ULONG& ulBytesWritten_)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   if(pvData_ == NULL)
      return USBError::INVALID_PARAM;

   //!!is there a max message size that we should test for?

   ULONG ulStartTime = DSIThread_GetSystemTime();
	ssize_t theWriteCount;
   ulBytesWritten_ = 0;
   do
   {
      theWriteCount = write(hSerialFileDescriptor, pvData_, ulSize_);
      if(theWriteCount == -1)
      {
         if (errno == ENXIO)           
            return USBError::FAILED;
      }
      else if(theWriteCount <= static_cast<long>(ulSize_))
      {
         UCHAR* pucDummy = (UCHAR*) pvData_;
         pucDummy += theWriteCount;
         pvData_ = pucDummy;
         
         ulSize_ -= theWriteCount;
         ulBytesWritten_ += theWriteCount;
      }
         
   } while(ulSize_ && ((DSIThread_GetSystemTime() - ulStartTime) < 15000));
   
   
   if(ulSize_)
      return USBError::FAILED;

   return USBError::NONE;
}



//////////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
USBError::Enum USBDeviceHandleSI::Read(void* pvData_, ULONG ulSize_, ULONG& ulBytesRead_, ULONG ulWaitTime_)
{
   if(bDeviceGone)
      return USBError::DEVICE_GONE;

   ulBytesRead_ = clRxQueue.PopArray(reinterpret_cast<char* const>(pvData_), ulSize_, ulWaitTime_);
   if(ulBytesRead_ == 0)
      return USBError::TIMED_OUT;

   return USBError::NONE;
}


//!!Can optimize this to push more data at a time into the receive queue
void USBDeviceHandleSI::ReceiveThread()
{
   #if defined(DEBUG_FILE)
   BOOL bRxDebug = DSIDebug::ThreadInit("ao_si_vcp_receive.txt");
   DSIDebug::ThreadEnable(TRUE);
   #endif

   ssize_t	theReadResult = 0;
   UCHAR theByte;
   while (!bStopReceiveThread)
   {
      //This is non-blocking IO
      theReadResult = read(hSerialFileDescriptor, &theByte, 1);  //!!Can we read more bytes?
      switch(theReadResult)
      {
         case 0:			//nothing in buffer, try again in 10ms
            usleep(10000);
            break;

         case 1:
            clRxQueue.Push(theByte);
            break;

         case -1:
         default:
            if(errno == EAGAIN)  //resource is temporarily unavailable, try again in 10ms
            {
               usleep(10000);
            }
            else
            {
               //!!Do we want to do this?
               #if defined(DEBUG_FILE)
               if(bRxDebug)
               {
                  char szMesg[255];
                  SNPRINTF(szMesg, 255, "Error: Read error = %d\n", errno);
                  DSIDebug::ThreadWrite(szMesg);
               }
               #endif
            }
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
DSI_THREAD_RETURN USBDeviceHandleSI::ProcessThread(void* pvParameter_)
{
   USBDeviceHandleSI* This = reinterpret_cast<USBDeviceHandleSI*>(pvParameter_);
   This->ReceiveThread();
   return 0;
}





// Returns an iterator across all known modems. Caller is responsible for
// releasing the iterator when iteration is complete.
BOOL USBDeviceHandleSI::FindModems(io_iterator_t* phMatchingServices_)
{
    kern_return_t			eKernResult;
    CFMutableDictionaryRef	hClassesToMatch;

    //@abstract Create a matching dictionary that specifies an IOService class match.
    //@param name The class name, as a const C-string. Class matching is successful on IOService's of this class or any subclass.
    //The dictionary is commonly passed to IOServiceGetMatchingServices or IOServiceAddNotification which will consume a reference, otherwise it should be released with CFRelease by the caller. */
    // Serial devices are instances of class IOSerialBSDClient
    hClassesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if(hClassesToMatch == NULL)
		return FALSE;

	//@abstract Sets the value of the key in the dictionary.
	//@param theDict - The dictionary to which the value is to be set.
	//@param key - The key of the value to set into the dictionary.
	//@param value - The value to add to or replace into the dictionary.
    CFDictionarySetValue(hClassesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDRS232Type));

	// Each serial device object has a property with key
    // kIOSerialBSDTypeKey and a value that is one of kIOSerialBSDAllTypes,
    // kIOSerialBSDModemType, or kIOSerialBSDRS232Type. You can experiment with the
    // matching by changing the last parameter in the above call to CFDictionarySetValue.




	//@abstract Look up registered IOService objects that match a matching dictionary.
    //@param masterPort The master port obtained from IOMasterPort().
    //@param matching A CF dictionary containing matching information, of which one reference is consumed by this function. IOKitLib can contruct matching dictionaries for common criteria with helper functions such as IOServiceMatching, IOOpenFirmwarePathMatching.
    //@param existing An iterator handle is returned on success, and should be released by the caller when the iteration is finished.
    eKernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, hClassesToMatch, phMatchingServices_);
    if(KERN_SUCCESS != eKernResult)
		return FALSE;

    return TRUE;
}



void USBDeviceHandleSI::DevicesDetached(void* param, io_iterator_t removed_devices)
{

  USBDeviceHandleSI* This = reinterpret_cast<USBDeviceHandleSI*>(param);

   //See if this device is in the list of removed devices
  io_service_t device;
  while ((device = IOIteratorNext(removed_devices)) != 0)
  {
    /* get the location from the i/o registry */
    CFTypeRef locationCF;
    locationCF = IORegistryEntryCreateCFProperty(device, CFSTR(kUSBDevicePropertyLocationID), kCFAllocatorDefault, 0);

    long location;
    CFNumberGetValue(reinterpret_cast<CFNumberRef>(locationCF), kCFNumberLongType, &location);
    CFRelease(locationCF);
    IOObjectRelease(device);

    if( static_cast<long>( This->clDevice.GetLocation() ) == location)
    {
      This->bDeviceGone = TRUE;
      break;
    }
  }

  //!!Need to release iterator?
  
   return;
}




void* USBDeviceHandleSI::EventThreadStart(void* param_)
{
   USBDeviceHandleSI* This = reinterpret_cast<USBDeviceHandleSI*>(param_);
   This->EventThread();
   return NULL;
}

void USBDeviceHandleSI::EventThread()
{

   //Create master port for talking to IOKit
   mach_port_t mach_port;
   io_iterator_t removed_devs_iterator;
   IOReturn kresult = IOMasterPort(MACH_PORT_NULL, &mach_port);
   
   //hotplug (device removal) source
   CFRunLoopSourceRef notification_source;
   io_notification_port_t notification_port;

   CFRetain(CFRunLoopGetCurrent());

   //add the notification port to the run loop
   try
   {
      if (!mach_port)
      {
         printf("\n\nUSBDeviceHandleSI::EventThread: mach_port NULL!\n");
         throw;
      }
      notification_port = IONotificationPortCreate(mach_port);
      if (!notification_port)
      {
         printf("\n\nUSBDeviceHandleSI::EventThread: notification_port NULL!\n");
         throw;
      }
      notification_source = IONotificationPortGetRunLoopSource(notification_port);
      if (!notification_source)
      {
         printf("\n\nUSBDeviceHandleSI::EventThread: notification_source NULL!\n");
         throw;
      }
      CFRunLoopAddSource(CFRunLoopGetCurrent(), notification_source, kCFRunLoopDefaultMode);

      //create notifications for removed devices

      kresult = IOServiceAddMatchingNotification(  notification_port,
                                                   kIOTerminatedNotification,
                                                   IOServiceMatching(kIOUSBDeviceClassName),
                                                   (IOServiceMatchingCallback)USBDeviceHandleSI::DevicesDetached,
                                                   this,
                                                   &removed_devs_iterator
                                                );
      if(kresult != kIOReturnSuccess)
      {
         printf("\n\nUSBDeviceHandleSI::EventThread: IOServiceAddMatchingNotification failed\n");
         throw;
      }
   }
   catch(...)
   {
      CFRunLoopSourceInvalidate(notification_source);
      IONotificationPortDestroy(notification_port);
      
      pthread_mutex_lock(&lock_runloop); 
      
      CFRelease(CFRunLoopGetCurrent());
      thread_started = FALSE;      
      
      pthread_cond_signal(&cond_runloop);
      pthread_mutex_unlock(&lock_runloop);
      
      pthread_exit((void*)kresult);
      return;
   }

   //arm notifiers
   //clear iterator
   io_service_t device;
   while ((device = IOIteratorNext(removed_devs_iterator)) != 0)
     IOObjectRelease (device);
   
   /* let the main thread know about the async runloop */
   pthread_mutex_lock(&lock_runloop);
   run_loop = CFRunLoopGetCurrent();
   thread_started = TRUE;
   pthread_cond_signal(&cond_runloop);
   pthread_mutex_unlock(&lock_runloop);

   /* run the runloop */
   CFRunLoopRun();

   /* delete notification port */
   CFRunLoopSourceInvalidate(notification_source);
   IONotificationPortDestroy(notification_port);
   IOObjectRelease(removed_devs_iterator);

   pthread_mutex_lock(&lock_runloop);
   CFRelease(CFRunLoopGetCurrent());
   thread_started = FALSE;
   pthread_mutex_unlock(&lock_runloop);

   pthread_exit(0);

}





#endif //defined(DSI_TYPES_MACINTOSH)
