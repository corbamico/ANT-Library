/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright � 1998-2011 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
 

#include "types.h"
#include "dsi_framer_ant.hpp"
#include "dsi_serial_generic.hpp"
#include "dsi_thread.h"
#include "config.h"

#include "dsi_debug.hpp"
#if defined(DEBUG_FILE)
   #include "macros.h"
#endif

#include "dsi_ant_device.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////////////////

DSIANTDevice::DSIANTDevice()
{
   bInitFailed = FALSE;
   bOpened = FALSE;

   hRequestThread = (DSI_THREAD_ID)NULL;                 // Handle for the Request thread
   hReceiveThread = (DSI_THREAD_ID)NULL;

   bKillThread = FALSE;
   bReceiveThreadRunning = FALSE;
   bRequestThreadRunning = FALSE;
   bCancel = FALSE;
   ucUSBDeviceNum = 0;
   usBaudRate = 0;
   ulUSBSerialNumber = 0;

   bAutoInit = FALSE;
   bPollUSB = FALSE;

   memset(apclChannelList, 0, sizeof(apclChannelList));

   eState = ANT_USB_STATE_OFF;
   eRequest = REQUEST_NONE;

   #if defined(DEBUG_FILE)
      DSIDebug::Init();
   #endif

   pclSerialObject = new DSISerialGeneric();
   pclANT = new DSIFramerANT();
   if (pclANT->Init(pclSerialObject) == FALSE)
   {
      bInitFailed = TRUE;
   }

   pclANT->SetCancelParameter(&bCancel);

   pclSerialObject->SetCallback(pclANT);


   if (DSIThread_MutexInit(&stMutexCriticalSection) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   if (DSIThread_MutexInit(&stMutexResponseQueue) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   if (DSIThread_MutexInit(&stMutexChannelListAccess) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   if (DSIThread_CondInit(&stCondRequestThreadExit) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   if (DSIThread_CondInit(&stCondReceiveThreadExit) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   if (DSIThread_CondInit(&stCondRequest) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   if (DSIThread_CondInit(&stCondWaitForResponse) != DSI_THREAD_ENONE)
   {
      bInitFailed = TRUE;
   }

   //If the init fails, something is broken and nothing is going to work anyway
   //additionally the logging isn't even setup at this point
   //so we throw an exception so we know something is broken right away   
   if(bInitFailed == TRUE)
      throw "ANTFS constructor: init failed";
}

///////////////////////////////////////////////////////////////////////
DSIANTDevice::~DSIANTDevice()
{
   this->Close();

   delete pclSerialObject;
   delete pclANT;

   if (bInitFailed == FALSE)
   {
      DSIThread_MutexDestroy(&stMutexCriticalSection);
      DSIThread_MutexDestroy(&stMutexResponseQueue);
      DSIThread_MutexDestroy(&stMutexChannelListAccess);
      DSIThread_CondDestroy(&stCondReceiveThreadExit);
      DSIThread_CondDestroy(&stCondRequestThreadExit);
      DSIThread_CondDestroy(&stCondRequest);
      DSIThread_CondDestroy(&stCondWaitForResponse);
   }

   #if defined(DEBUG_FILE)
      DSIDebug::Close();
   #endif
}

///////////////////////////////////////////////////////////////////////
BOOL DSIANTDevice::Init(BOOL bPollUSB_)
{
   #if defined(DEBUG_FILE)
      DSIDebug::ThreadInit("Application");
   #endif

   if (bInitFailed == TRUE)
   {
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::Init():  bInitFailed == TRUE");
      #endif
      return FALSE;
   }

   bAutoInit = TRUE;
   bPollUSB = bPollUSB_;

   return ReInitDevice();
}

///////////////////////////////////////////////////////////////////////
BOOL DSIANTDevice::Init(UCHAR ucUSBDeviceNum_, USHORT usBaudRate_, BOOL bPollUSB_)
{
   #if defined(DEBUG_FILE)
   DSIDebug::ThreadInit("Application");
   #endif

   bAutoInit = FALSE;
   ucUSBDeviceNum = ucUSBDeviceNum_;
   usBaudRate = usBaudRate_;
   bPollUSB = bPollUSB_;

   return ReInitDevice();
}

///////////////////////////////////////////////////////////////////////
void DSIANTDevice::Close()
{
   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::Close():  Closing...");
   #endif

   DSIThread_MutexLock(&stMutexCriticalSection);

   // Stop the threads.
   bKillThread = TRUE;

   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::Close():  SetEvent(stCondWaitForResponse).");
   #endif

   DSIThread_MutexLock(&stMutexResponseQueue);
   DSIThread_CondSignal(&stCondWaitForResponse);
   clResponseQueue.Clear();
   DSIThread_MutexUnlock(&stMutexResponseQueue);

   ClearManagedChannelList();

   if (hRequestThread)
   {
      if (bRequestThreadRunning == TRUE)
	  {
         #if defined(DEBUG_FILE)
            DSIDebug::ThreadWrite("DSIANTDevice::Close():  SetEvent(stCondRequest).");
         #endif
         DSIThread_CondSignal(&stCondRequest);

         #if defined(DEBUG_FILE)
            DSIDebug::ThreadWrite("DSIANTDevice::Close():  Killing thread.");
         #endif

         if (DSIThread_CondTimedWait(&stCondRequestThreadExit, &stMutexCriticalSection, 9000) != DSI_THREAD_ENONE)
         {
            #if defined(DEBUG_FILE)
               DSIDebug::ThreadWrite("DSIANTDevice::Close():  Thread not dead.");
               DSIDebug::ThreadWrite("DSIANTDevice::Close():  Forcing thread termination...");
            #endif
            DSIThread_DestroyThread(hRequestThread);
         }
         else
         {
            #if defined(DEBUG_FILE)
               DSIDebug::ThreadWrite("DSIANTDevice::Close():  Thread terminated successfully.");
            #endif
         }
      }

      DSIThread_ReleaseThreadID(hRequestThread);
      hRequestThread = (DSI_THREAD_ID)NULL;
   }

   if (hReceiveThread)
   {
      if (bReceiveThreadRunning == TRUE)
      {
         #if defined(DEBUG_FILE)
            DSIDebug::ThreadWrite("DSIANTDevice::Close():  Killing receive thread.");
         #endif


         if (DSIThread_CondTimedWait(&stCondReceiveThreadExit, &stMutexCriticalSection, 2000) == DSI_THREAD_ENONE)
         {
            #if defined(DEBUG_FILE)
               DSIDebug::ThreadWrite("DSIANTDevice::Close():  Receive thread not dead.");
               DSIDebug::ThreadWrite("DSIANTDevice::Close():  Forcing thread termination...");
            #endif
            DSIThread_DestroyThread(hReceiveThread);
         }
         else
         {
            #if defined(DEBUG_FILE)
               DSIDebug::ThreadWrite("DSIANTDevice::Close():  Thread terminated successfully.");
            #endif
         }
      }

      DSIThread_ReleaseThreadID(hReceiveThread);
      hReceiveThread = (DSI_THREAD_ID)NULL;
   }

   DSIThread_MutexUnlock(&stMutexCriticalSection);

   CloseDevice(FALSE);

   eState = ANT_USB_STATE_OFF;

   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::Close():  Closed.");
   #endif
}

///////////////////////////////////////////////////////////////////////
ULONG DSIANTDevice::GetSerialNumber(void)
{
   return ulUSBSerialNumber;
}

///////////////////////////////////////////////////////////////////////
ANT_USB_STATE DSIANTDevice::GetStatus(void)
{
   return eState;
}

///////////////////////////////////////////////////////////////////////
ANT_USB_RESPONSE DSIANTDevice::WaitForResponse(ULONG ulMilliseconds_)
{
	ANT_USB_RESPONSE eResponse = ANT_USB_RESPONSE_NONE;

   if (bKillThread == TRUE)
      return ANT_USB_RESPONSE_NONE;

	//Wait for response
	DSIThread_MutexLock(&stMutexResponseQueue);
		if(clResponseQueue.isEmpty())
		{
			UCHAR ucResult = DSIThread_CondTimedWait(&stCondWaitForResponse, &stMutexResponseQueue, ulMilliseconds_);
			switch(ucResult)
			{
			   case DSI_THREAD_ENONE:
				   eResponse = clResponseQueue.GetResponse();
				   break;

			   case DSI_THREAD_ETIMEDOUT:
				   eResponse = ANT_USB_RESPONSE_NONE;
				   break;

			   case DSI_THREAD_EOTHER:
               #if defined(DEBUG_FILE)
                  DSIDebug::ThreadWrite("DSIANTDevice:WaitForResponse():  CondTimedWait() Failed!");
               #endif
               eResponse = ANT_USB_RESPONSE_NONE;
               break;

            default:
               #if defined(DEBUG_FILE)
                  DSIDebug::ThreadWrite("DSIANTDevice:WaitForResponse():  Error  Unknown...");
               #endif
               eResponse = ANT_USB_RESPONSE_NONE;
               break;
			}
		}
		else
		{
			eResponse = clResponseQueue.GetResponse();
		}
	DSIThread_MutexUnlock(&stMutexResponseQueue);

	return eResponse;
}

///////////////////////////////////////////////////////////////////////
BOOL DSIANTDevice::AddMessageProcessor(UCHAR ucChannelNumber_, DSIANTMessageProcessor* pclChannel_)
{
   DSIThread_MutexLock(&stMutexChannelListAccess);

   if(ucChannelNumber_ >= MAX_ANT_CHANNELS)
   {
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::AddManagedChannel():  Invalid channel number");
      #endif
      DSIThread_MutexUnlock(&stMutexChannelListAccess);
      return FALSE;
   }

   if(apclChannelList[ucChannelNumber_] != (DSIANTMessageProcessor*) NULL)
   {
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::AddManagedChannel(): Managed channel already registered for this channel number. Unregister first.");
      #endif
      DSIThread_MutexUnlock(&stMutexChannelListAccess);
      return FALSE;
   }

   apclChannelList[ucChannelNumber_] = pclChannel_;
   pclChannel_->Init(pclANT, ucChannelNumber_);
      
   DSIThread_MutexUnlock(&stMutexChannelListAccess);

   return TRUE;
}

///////////////////////////////////////////////////////////////////////
BOOL DSIANTDevice::RemoveMessageProcessor(DSIANTMessageProcessor* pclChannel_)
{
   UCHAR ucChannel;

   DSIThread_MutexLock(&stMutexChannelListAccess);

   if(pclChannel_ == (DSIANTMessageProcessor*) NULL)
   {
      DSIThread_MutexUnlock(&stMutexChannelListAccess);
      return FALSE;
   }

   // Find the handle in the list
   ucChannel = pclChannel_->GetChannelNumber();

   if(apclChannelList[ucChannel] == (DSIANTMessageProcessor*) NULL)
   {
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::RemoveMessageProcessor(): No message processor registered.");
      #endif
      DSIThread_MutexUnlock(&stMutexChannelListAccess);
      return FALSE;
   }

   if(apclChannelList[ucChannel] != pclChannel_)
   {
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::RemoveMessageProcessor(): Message processor mismatch during removal.");
      #endif
      DSIThread_MutexUnlock(&stMutexChannelListAccess);
      return FALSE;
   }

   pclChannel_->Close();
   apclChannelList[ucChannel] = (DSIANTMessageProcessor*) NULL;

   DSIThread_MutexUnlock(&stMutexChannelListAccess);

   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::RemoveMessageProcessor(): Message processor removed.");
   #endif

   return TRUE;
}

///////////////////////////////////////////////////////////////////////
void DSIANTDevice::ClearManagedChannelList(void)
{
   DSIThread_MutexLock(&stMutexChannelListAccess);
      memset(apclChannelList, 0, sizeof(apclChannelList));
   DSIThread_MutexUnlock(&stMutexChannelListAccess);
}

#if defined(DEBUG_FILE)
///////////////////////////////////////////////////////////////////////
void DSIANTDevice::SetDebug(BOOL bDebugOn_, const char *pcDirectory_)
{
   if (pcDirectory_)
      DSIDebug::SetDirectory(pcDirectory_);

   DSIDebug::SetDebug(bDebugOn_);
}
#endif


//////////////////////////////////////////////////////////////////////////////////
// Private Functions
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
BOOL DSIANTDevice::ReInitDevice(void)
{
   if (eState != ANT_USB_STATE_OFF)
      this->Close();

   bCancel = FALSE;
   bKillThread = FALSE;

   #if defined(DEBUG_FILE)
      DSIDebug::ResetTime();
      DSIDebug::ThreadWrite(SW_VER);
   #endif

   if (pclANT->Init(pclSerialObject) == FALSE)
   {
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::ReInitDevice():  Failed to re-init pclANT.");
      #endif
      return FALSE;
   }

   if (hRequestThread == NULL)
   {
      hRequestThread = DSIThread_CreateThread(&DSIANTDevice::RequestThreadStart, this);
      if (hRequestThread == NULL)
         return FALSE;
   }

   if (hReceiveThread == NULL)
   {
      hReceiveThread = DSIThread_CreateThread(&DSIANTDevice::ReceiveThreadStart, this);
      if (hReceiveThread == NULL)
         return FALSE;
   }   

   DSIThread_MutexLock(&stMutexResponseQueue);
   clResponseQueue.Clear();
   DSIThread_MutexUnlock(&stMutexResponseQueue);

   DSIThread_MutexLock(&stMutexCriticalSection);
   eRequest = REQUEST_OPEN;
   DSIThread_CondSignal(&stCondRequest);
   DSIThread_MutexUnlock(&stMutexCriticalSection);

   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::ReInitDevice():  Initializing.");
   #endif

   return TRUE;
}

///////////////////////////////////////////////////////////////////////
BOOL DSIANTDevice::AttemptOpen(void)
{
   if (bOpened)
   {
      pclSerialObject->Close();
      bOpened = FALSE;
   }

   if (bAutoInit)
   {
      if (pclSerialObject->AutoInit() == FALSE)
      {
         #if defined(DEBUG_FILE)
            DSIDebug::ThreadWrite("DSIANTDevice::AttemptOpen():  AutoInit Failed.");
         #endif
         return FALSE;
      }
   }
   else
   {
      if(pclSerialObject->Init(usBaudRate, ucUSBDeviceNum) == FALSE)
         return FALSE;
   }

   if (pclSerialObject->Open() == FALSE)
      return FALSE;

   ulUSBSerialNumber = pclSerialObject->GetDeviceSerialNumber();
   bOpened = TRUE;

   return TRUE;
}

///////////////////////////////////////////////////////////////////////
void DSIANTDevice::CloseDevice(BOOL bReset_)
{
   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::CloseDevice():  Closing..");
   #endif

   if ((bOpened == TRUE) && (eRequest != REQUEST_HANDLE_SERIAL_ERROR))
   {
      // Send reset command to ANT.
      pclANT->ResetSystem();

      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::CloseDevice():  ANT_ResetSystem() complete.");
      #endif
   }
   #if defined(DEBUG_FILE)
      if (bReset_)
      {
         DSIDebug::ThreadWrite("DSIANTDevice::CloseDevice():  Calling serial object close with reset = TRUE.");         
      }
   #endif
   pclSerialObject->Close(bReset_);
   bOpened = FALSE;

#if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::CloseDevice():  Closed");
   #endif
}

///////////////////////////////////////////////////////////////////////
//TODO //TTA Should change this to be like the managed side
void DSIANTDevice::HandleSerialError(void)
{
   UCHAR i;

   if (eState < ANT_USB_STATE_OPEN)   // No need to process errors if already handled
      return;

   // Notify callbacks
   for(i = 0; i< MAX_ANT_CHANNELS; i++)
   {
      if(apclChannelList[i] != (DSIANTMessageProcessor*) NULL)
         apclChannelList[i]->ProcessDeviceNotification(ANT_DEVICE_NOTIFICATION_SHUTDOWN, (void*) NULL);
   }

   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::HandleSerialError(): Resetting USB Device");
   #endif

   CloseDevice(TRUE);

   #if defined(DSI_TYPES_WINDOWS)
      // Reset the USB stick.
      pclSerialObject->USBReset();
   #endif   

   DSIThread_MutexLock(&stMutexCriticalSection);
   eRequest = REQUEST_SERIAL_ERROR_HANDLED;
   DSIThread_CondSignal(&stCondRequest);

   DSIThread_MutexUnlock(&stMutexCriticalSection);

   AddResponse(ANT_USB_RESPONSE_SERIAL_FAIL); //TODO: can move this to the Request thread?
}

///////////////////////////////////////////////////////////////////////
DSI_THREAD_RETURN DSIANTDevice::RequestThreadStart(void *pvParameter_)
{
   #if defined(DEBUG_FILE)
   DSIDebug::ThreadInit("DSIANTDevice");
   #endif

   ((DSIANTDevice*)pvParameter_)->RequestThread();

   return 0;
}

///////////////////////////////////////////////////////////////////////
//TODO //TTA Why use a polling state machine here? It just makes things complicated to trace.
void DSIANTDevice::RequestThread(void)
{
   ANT_USB_RESPONSE eResponse;
   bRequestThreadRunning = TRUE;

   while (bKillThread == FALSE)
   {
      eResponse = ANT_USB_RESPONSE_NONE;

      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Awaiting Requests...");
      #endif

      DSIThread_MutexLock(&stMutexCriticalSection);

      if ((eRequest == REQUEST_NONE) && (bKillThread == FALSE))
      {
         UCHAR ucResult = DSIThread_CondTimedWait(&stCondRequest, &stMutexCriticalSection, 1000);
         if (ucResult != DSI_THREAD_ENONE)  // Polling interval 1Hz
         {
            #if defined(DEBUG_FILE)
               if(ucResult == DSI_THREAD_EOTHER)
                  DSIDebug::ThreadWrite("DSIANTDevice::RequestThread(): CondTimedWait() Failed!");
            #endif
            if (eRequest == REQUEST_NONE && eState == ANT_USB_STATE_IDLE_POLLING_USB && bPollUSB)
            {
               eRequest = REQUEST_OPEN;
            }
            DSIThread_MutexUnlock(&stMutexCriticalSection);
            continue;
         }
      }

      DSIThread_MutexUnlock(&stMutexCriticalSection);

      if (bKillThread)
         break;      

      if (eRequest != REQUEST_NONE)
      {
         #if defined(DEBUG_FILE)
            DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Request received");
         #endif

         switch (eRequest)
         {
            case REQUEST_OPEN:
               #if defined(DEBUG_FILE)
                  DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Opening...");
               #endif

               if (AttemptOpen())
               {
                  #if defined(DEBUG_FILE)
                     DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Open passed.");
                  #endif

                  eState = ANT_USB_STATE_OPEN;
                  eResponse = ANT_USB_RESPONSE_OPEN_PASS;
               }
               else
               {
                  #if defined(DEBUG_FILE)
                     DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Open failed.");
                  #endif
                  
                  if(bPollUSB)
                  {
                     eState = ANT_USB_STATE_IDLE_POLLING_USB;
                  }
                  else
                  {
                     eState = ANT_USB_STATE_IDLE;
                     eResponse = ANT_USB_RESPONSE_OPEN_FAIL;
                  }
               }
               break;

            default:
               break;
         }

         //This is where to handle the internal requests, because they can happen asyncronously.
         //We will also clear the request here.
         DSIThread_MutexLock(&stMutexCriticalSection);

         if (eResponse != ANT_USB_RESPONSE_NONE)
            AddResponse(eResponse);

         if (eRequest == REQUEST_HANDLE_SERIAL_ERROR)
         {
            #if defined(DEBUG_FILE)
               DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Serial error!");
            #endif
            HandleSerialError();
            if(bPollUSB)
            {
               eState = ANT_USB_STATE_IDLE_POLLING_USB;
               eRequest = REQUEST_OPEN;
            }
            else
            {
               eState = ANT_USB_STATE_IDLE;
               eRequest = REQUEST_NONE;
            }
         }
         else if (eRequest == REQUEST_SERIAL_ERROR_HANDLED)
         {
            #if defined(DEBUG_FILE)
               DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Serial error handled");
            #endif
            if(bPollUSB)
            {
               eState = ANT_USB_STATE_IDLE_POLLING_USB;
               eRequest = REQUEST_OPEN;
            }
            else
            {
               eState = ANT_USB_STATE_IDLE;
               eRequest = REQUEST_NONE;
            }
         }
         else
         {
            eRequest = REQUEST_NONE;  //Clear any other request
         }

         DSIThread_MutexUnlock(&stMutexCriticalSection);
      }

   }//while()

   #if defined(DEBUG_FILE)
      DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  Exiting thread.");
   #endif

   eRequest = REQUEST_NONE;


   CloseDevice(FALSE); //Try to close the device if it was open.  This is done because the device was opened in this thread, we should try to close the device before the thread terminates.

   DSIThread_MutexLock(&stMutexCriticalSection);
      bRequestThreadRunning = FALSE;
      DSIThread_CondSignal(&stCondRequestThreadExit);
   DSIThread_MutexUnlock(&stMutexCriticalSection);


   #if defined(__cplusplus)
      return;
   #else
      ExitThread(0);
      #if defined(DEBUG_FILE)
         DSIDebug::ThreadWrite("DSIANTDevice::RequestThread():  C code reaching return statement unexpectedly.");
      #endif
      return;                                            // Code should not be reached.
   #endif
}


///////////////////////////////////////////////////////////////////////
DSI_THREAD_RETURN DSIANTDevice::ReceiveThreadStart(void *pvParameter_)
{
   #if defined(DEBUG_FILE)
   DSIDebug::ThreadInit("ANTReceive");
   #endif

   ((DSIANTDevice*)pvParameter_)->ReceiveThread();

   return 0;
}

///////////////////////////////////////////////////////////////////////
// ANT Receive Thread
///////////////////////////////////////////////////////////////////////
#define MESG_CHANNEL_OFFSET                  0
#define MESG_EVENT_ID_OFFSET                 1
#define MESG_EVENT_CODE_OFFSET               2

void DSIANTDevice::ReceiveThread(void)
{
   ANT_MESSAGE stMessage;
   USHORT usMesgSize = 0;

   bReceiveThreadRunning = TRUE;

   while (bKillThread == FALSE)
   {

      #if defined(DEBUG_FILEV2)
      DSIDebug::ThreadWrite("Rx:Wait");
      #endif

      usMesgSize = pclANT->WaitForMessage(1000);

      //TODO A: Legacy code, may be able to remove
      #if defined(DEBUG_FILEV2)
         {
            UCHAR aucString1[256];

            SNPRINTF((char *)aucString1,256, "Rx:%u", usMesgSize);
            DSIDebug::ThreadWrite((char *)aucString1);
            SNPRINTF((char *)aucString1,256, "Rx:(%lu/%lu)", ulTransferArrayIndex, ulTransferBufferSize);
            DSIDebug::ThreadWrite((char *)aucString1);
         }
      #endif

      if (bKillThread)
         break;

      if (usMesgSize == DSI_FRAMER_ERROR)
      {
         usMesgSize = pclANT->GetMessage(&stMessage, MESG_MAX_SIZE_VALUE);  //get the message to clear the error
         #if defined(DEBUG_FILE)
         {
            UCHAR aucString[256];

            SNPRINTF((char *)aucString,256, "DSIANTDevice::RecvThread():  Framer Error %u .", stMessage.ucMessageID);
            DSIDebug::ThreadWrite((char *)aucString);

            if(stMessage.ucMessageID == DSI_FRAMER_ANT_ESERIAL)
            {
               SNPRINTF((char *)aucString,256, "DSIANTDevice::RecvThread():  Serial Error %u .", stMessage.aucData[0]);
               DSIDebug::ThreadWrite((char *)aucString);
            }
         }
         #endif

         DSIThread_MutexLock(&stMutexCriticalSection);
         //bCancel = TRUE;

         if (eRequest < REQUEST_HANDLE_SERIAL_ERROR)
            eRequest = REQUEST_HANDLE_SERIAL_ERROR;
         DSIThread_CondSignal(&stCondRequest);
         DSIThread_MutexUnlock(&stMutexCriticalSection);

         continue;
      }

      if (usMesgSize < DSI_FRAMER_TIMEDOUT)  //if the return isn't DSI_FRAMER_TIMEDOUT or DSI_FRAMER_ERROR
	   {
         UCHAR ucANTChannel;
         usMesgSize = pclANT->GetMessage(&stMessage, MESG_MAX_SIZE_VALUE);

         #if defined(DEBUG_FILE)
         if (usMesgSize == 0)
         {
            UCHAR aucString2[256];

            SNPRINTF((char *)aucString2,256, "Rx msg reported size 0, dump:%u[0x%02X]...[0x%02X][0x%02X][0x%02X][0x%02X][0x%02X][0x%02X][0x%02X][0x%02X]", usMesgSize , stMessage.ucMessageID, stMessage.aucData[0], stMessage.aucData[1], stMessage.aucData[2], stMessage.aucData[3], stMessage.aucData[4], stMessage.aucData[5], stMessage.aucData[6], stMessage.aucData[7]);
            DSIDebug::ThreadWrite((char *)aucString2);
         }
         #endif

         if(stMessage.ucMessageID == MESG_SERIAL_ERROR_ID)
         {
            #if defined(DEBUG_FILE)
            {
               UCHAR aucString[256];
               SNPRINTF((char *) aucString, 256, "DSIANTDevice::RecvThread():  Serial Error.");
               DSIDebug::ThreadWrite((char *) aucString);
            }
            #endif

            DSIThread_MutexLock(&stMutexCriticalSection);
            //bCancel = TRUE;

            if (eRequest < REQUEST_HANDLE_SERIAL_ERROR)
               eRequest = REQUEST_HANDLE_SERIAL_ERROR;
            DSIThread_CondSignal(&stCondRequest);
            DSIThread_MutexUnlock(&stMutexCriticalSection);
            
            continue;
         }

         // Figure out channel
         //ucANTChannel = stMessage.aucData[MESG_CHANNEL_OFFSET] & CHANNEL_NUMBER_MASK;
         ucANTChannel = pclANT->GetChannelNumber(&stMessage);

         // Send messages to appropriate handler
         if(ucANTChannel != 0xFF)
         {
            // TODO: Add general channel and protocol event callbacks?
            if(apclChannelList[ucANTChannel] != (DSIANTMessageProcessor*) NULL)
            {
               if(apclChannelList[ucANTChannel]->GetEnabled())
                  apclChannelList[ucANTChannel]->ProcessMessage(&stMessage, usMesgSize);
            }
         }

       }
   } // while()

   DSIThread_MutexLock(&stMutexCriticalSection);
   bReceiveThreadRunning = FALSE;
   DSIThread_CondSignal(&stCondReceiveThreadExit);
   DSIThread_MutexUnlock(&stMutexCriticalSection);
}

///////////////////////////////////////////////////////////////////////
void DSIANTDevice::AddResponse(ANT_USB_RESPONSE eResponse_)
{
   DSIThread_MutexLock(&stMutexResponseQueue);
   clResponseQueue.AddResponse(eResponse_);
   DSIThread_CondSignal(&stCondWaitForResponse);
   DSIThread_MutexUnlock(&stMutexResponseQueue);
}
