/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright � 1998-2011 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
 

#if !defined(DSI_ANT_DEVICE_HPP)
#define DSI_ANT_DEVICE_HPP

#include "types.h"
#include "dsi_thread.h"
#include "dsi_framer_ant.hpp"
#include "dsi_serial.hpp"
#include "dsi_debug.hpp"

#include "antmessage.h"

#include "dsi_ant_message_processor.hpp"
#include "dsi_response_queue.hpp"

#define MAX_ANT_CHANNELS  8

//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////

typedef enum
{
   ANT_USB_RESPONSE_NONE = 0,
   ANT_USB_RESPONSE_OPEN_PASS,
   ANT_USB_RESPONSE_OPEN_FAIL,
   ANT_USB_RESPONSE_SERIAL_FAIL
} ANT_USB_RESPONSE;

// ANT-FS Host States
typedef enum
{
   ANT_USB_STATE_OFF = 0,
   ANT_USB_STATE_IDLE,
   ANT_USB_STATE_IDLE_POLLING_USB,
   ANT_USB_STATE_OPEN
} ANT_USB_STATE;

//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// This class handles the connection to the USB stick, polls for
// ANT messages and dispatches them to appropriate handlers (e.g.
// ANTFSClientChannel or ANTFSHostChannel) for processing
// 
// IMPORTANT: this class is under development and subject to change.
// It is not recommended to use this class for custom applications.
/////////////////////////////////////////////////////////////////
class DSIANTDevice
{
   private:

      //////////////////////////////////////////////////////////////////////////////////
      // Private Definitions
      //////////////////////////////////////////////////////////////////////////////////

      enum ENUM_REQUEST
      {
         REQUEST_NONE = 0,
         REQUEST_OPEN,
         REQUEST_HANDLE_SERIAL_ERROR,
         REQUEST_SERIAL_ERROR_HANDLED
      };

      enum RETURN_STATUS
      {
         RETURN_FAIL = 0,
         RETURN_PASS,
         RETURN_STOP,
         RETURN_REJECT,
         RETURN_NA,
         RETURN_SERIAL_ERROR
      };

      //////////////////////////////////////////////////////////////////////////////////
      // Private Variables
      ////////////////////////////////////////////////////////////////////////////////// 

      DSIResponseQueue<ANT_USB_RESPONSE> clResponseQueue;

      volatile BOOL bOpened;
      BOOL bInitFailed;

      DSI_THREAD_ID hRequestThread;                         // Handle for the request thread.
      DSI_THREAD_ID hReceiveThread;                         // Handle for the receive thread.
      DSI_MUTEX stMutexResponseQueue;                       // Mutex used with the response queue
      DSI_MUTEX stMutexCriticalSection;                     // Mutex used with the wait condition
      DSI_MUTEX stMutexChannelListAccess;                  // Mutex used with the channel list
      DSI_CONDITION_VAR stCondReceiveThreadExit;            // Event to signal the receive thread has ended.
      DSI_CONDITION_VAR stCondRequestThreadExit;            // Event to signal the request thread has ended.
      DSI_CONDITION_VAR stCondRequest;                      // Event to signal there is a new request
      DSI_CONDITION_VAR stCondWaitForResponse;              // Event to signal there is a new response to the application

      volatile BOOL bKillThread;
	   volatile BOOL bReceiveThreadRunning;
	   volatile BOOL bRequestThreadRunning;
      volatile BOOL bCancel;
      UCHAR ucUSBDeviceNum;
      USHORT usBaudRate;

      ULONG ulUSBSerialNumber;

      volatile BOOL bAutoInit;
      volatile BOOL bPollUSB;

      DSIANTMessageProcessor* apclChannelList[MAX_ANT_CHANNELS];      
      
      DSISerial *pclSerialObject;
      DSIFramerANT *pclANT;

      volatile ANT_USB_STATE eState;
      volatile ENUM_REQUEST eRequest;

      //////////////////////////////////////////////////////////////////////////////////
      // Private Function Prototypes
      //////////////////////////////////////////////////////////////////////////////////

      BOOL ReInitDevice(void);
      BOOL AttemptOpen(void);
      void CloseDevice(BOOL bReset_);

      void HandleSerialError();

      void RequestThread(void);
      static DSI_THREAD_RETURN RequestThreadStart(void *pvParameter_);

      void ReceiveThread(void);
      static DSI_THREAD_RETURN ReceiveThreadStart(void *pvParameter_);
      void AddResponse(ANT_USB_RESPONSE eResponse_);

   public:

      DSIANTDevice();
      ~DSIANTDevice();

      BOOL Init(BOOL bPollUSB_ = FALSE);
      /////////////////////////////////////////////////////////////////
      // Begins to initialize the DSIANTDevice object
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Parameters:
       //    bPollUSB_:        Specify whether to continue to retry initialization
      //                      if it fails
      // Operation:
      //    If bPollUSB_ is set to TRUE and it fails to initialize the USBstick, 
      //    it will continue to retry the intitialization once a second until
      //    it is sucessful.  Once everything has be sucessfully initialized
      //    a response of ANT_USB_RESPONSE_OPEN_PASS will be sent.
      //    If bPollUSB_ is set to FALSE, and initialization fails, a
      //    response of ANT_USB_RESPONSE_OPEN_FAIL will be sent.
      //
      // This version of Init will only work for the ANT USB stick with
      // USB device descriptor "ANT USB Stick"
      /////////////////////////////////////////////////////////////////

      BOOL Init(UCHAR ucUSBDeviceNum_, USHORT usBaudRate_, BOOL bPollUSB_ = FALSE);
      /////////////////////////////////////////////////////////////////
      // Begins to initialize the DSIANTDevice object
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Parameters:
      //    ucUSBDeviceNum_:  USB port number
      //    usBaudRate_:      Serial baud rate
      //    bPollUSB_:        Specify whether to continue to retry initialization
      //                      if it fails
      // Operation:
      //    If bPollUSB_ is set to TRUE and it fails to initialize the USBstick, 
      //    it will continue to retry the intitialization once a second until
      //    it is sucessful.  Once everything has be sucessfully initialized
      //    a response of ANT_USB_RESPONSE_OPEN_PASS will be sent.
      //    If bPollUSB_ is set to FALSE, and initialization fails, a
      //    response of ANT_USB_RESPONSE_OPEN_FAIL will be sent.
      /////////////////////////////////////////////////////////////////

      void Close(void);
      /////////////////////////////////////////////////////////////////
      // Stops any pending actions, closes all devices down and cleans
      // up any resources being used by the library.
      /////////////////////////////////////////////////////////////////

      ULONG GetSerialNumber();
      /////////////////////////////////////////////////////////////////
      // Returns the serial number of the USB device
      /////////////////////////////////////////////////////////////////

      ANT_USB_STATE GetStatus(void);
      /////////////////////////////////////////////////////////////////
      // Returns the current library status.
      /////////////////////////////////////////////////////////////////

      ANT_USB_RESPONSE WaitForResponse(ULONG ulMilliseconds_);
      /////////////////////////////////////////////////////////////////
      // Wait for a response from the library
      // Parameters:
      //    ulMilliseconds_:  Set this value to the minimum time to
      //                      wait before returning.  If the value is
      //                      0, the function will return immediately.
      //                      If the value is DSI_THREAD_INFINITE, the
      //                      function will not time out.
      // If one or more responses are pending before the timeout
      // expires the function will return the first response that
      // occurred.  If no response is pending at the time the timeout
      // expires, ANT_USB_RESPONSE_NONE is returned.
      /////////////////////////////////////////////////////////////////

      BOOL AddMessageProcessor(UCHAR ucChannelNumber_, DSIANTMessageProcessor* pclChannel_);
      /////////////////////////////////////////////////////////////////
      // Adds a message processor, to process messages for the specified 
      // channel number.
      // Parameters:
      //    ucChannelNumber_: ANT channel number
      //    *pclChannel_:    A pointer to a DSIANTMessageProcessor.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Operation:
      //    After adding attaching a message processor to a channel number,
      //    messages for the corresponding channel, are dispatched to it for
      //    processing.   Only one message processor can be added per ANT channel.
      /////////////////////////////////////////////////////////////////

      BOOL RemoveMessageProcessor(DSIANTMessageProcessor* pclChannel_);
      /////////////////////////////////////////////////////////////////
      // Unregisters a message processor, to stop processing ANT messages.
      // Parameters:
      //    *pclChannel_:    A pointer to a DSIANTMessageProcessor.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Operation:
      //    Remove instances of classes derived of DSIANTMessageProcessor 
      //    from the list to stop processing messages for that
      //    channel number
      /////////////////////////////////////////////////////////////////

      void ClearManagedChannelList(void);
      /////////////////////////////////////////////////////////////////
      // Remove all managed channels from the list
      /////////////////////////////////////////////////////////////////
      
      #if defined(DEBUG_FILE)
      void SetDebug(BOOL bDebugOn_, const char *pcDirectory_ = (const char*) NULL);      
      /////////////////////////////////////////////////////////////////
      // Enables debug files
      // Parameters:
      //    bDebugOn_:     Enable/disable debug logs.
      //    *pcDirectory_: A string to use as the path for storing
      //                   debug logs. Set to NULL to use the working
      //                   directory as the default path.
      /////////////////////////////////////////////////////////////////
      #endif

};

#endif  // DSI_ANT_DEVICE_HPP