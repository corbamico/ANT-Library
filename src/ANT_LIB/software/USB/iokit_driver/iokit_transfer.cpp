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

#include "iokit_transfer.hpp"

#include "dsi_thread.h"
#include "iokit_device_handle.hpp"
#include "iokit_interface.hpp"


#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

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


//!!Should transfer have own internal data buffer? yes

IOKitTransfer* IOKitTransfer::MakeControlTransfer(const IOKitDeviceHandle& dev_handle_, struct libusb_control_setup& setup_,
                                              const int pipe[2], unsigned char data_buffer_[], unsigned int timeout_)
{
   return new IOKitTransfer(dev_handle_, setup_, pipe, data_buffer_, TransferType::CONTROL, timeout_);
}

IOKitTransfer* IOKitTransfer::MakeInterruptTransfer(const IOKitDeviceHandle& dev_handle_, unsigned char endpoint_,
                             const int pipe[2], unsigned char data_buffer_[], unsigned long data_length_, unsigned int timeout_)
{
   return new IOKitTransfer(dev_handle_, pipe, endpoint_, data_buffer_, data_length_, TransferType::INTERRUPT, timeout_);
}


IOKitTransfer* IOKitTransfer::MakeBulkTransfer(const IOKitDeviceHandle& dev_handle_, unsigned char endpoint_,
                             const int pipe[2], unsigned char data_buffer_[], unsigned long data_length_, unsigned int timeout_)
{
   return new IOKitTransfer(dev_handle_, pipe, endpoint_, data_buffer_, data_length_, TransferType::BULK, timeout_);
}



///////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////
IOKitTransfer::IOKitTransfer(const IOKitDeviceHandle& dev_handle_, struct libusb_control_setup& setup_,
                             const int pipe[2],
                             unsigned char data_buffer_[],
                             unsigned char type_, unsigned int timeout_)
:
   endpoint(0),
   type(type_),
   timeout(timeout_),
   control_setup(setup_),
   buffer(data_buffer_),
   length(libusb_le16_to_cpu(setup_.wLength)),
   dev_handle(dev_handle_)
{

   return_pipe[0] = pipe[0];
   return_pipe[1] = pipe[1];

   transferred = 0;
   actual_length = 0;

   time_done = 0;
   timedout = FALSE;
   transfer_complete = FALSE;
   pclTimer = (DSITimer*)NULL;

   DSIThread_MutexInit(&stMutexCriticalSection);
   return;
}

IOKitTransfer::IOKitTransfer(const IOKitDeviceHandle& dev_handle_,
                             const int pipe[2], unsigned char endpoint_,
                             unsigned char data_buffer_[], unsigned long data_length_,
                             unsigned char type_, unsigned int timeout_)
:
   endpoint(endpoint_),
   type(type_),
   timeout(timeout_),
   control_setup(),
   buffer(data_buffer_),
   length(data_length_),
   dev_handle(dev_handle_)
{

   return_pipe[0] = pipe[0];
   return_pipe[1] = pipe[1];

   transferred = 0;
   actual_length = 0;

   time_done = 0;
   timedout = FALSE;
   transfer_complete = FALSE;
   pclTimer = (DSITimer*)NULL;

   DSIThread_MutexInit(&stMutexCriticalSection);
   return;
}
///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
IOKitTransfer::~IOKitTransfer()
{
   if (pclTimer)
   {
      //since a timer was running, we must have been waiting on a transfer; cancel existing transfers by running the timer callback
      TimeoutTransfer(this);
      delete pclTimer;
      pclTimer = (DSITimer*)NULL;
   }
   DSIThread_MutexDestroy(&stMutexCriticalSection);
   //should we cancel a pending transfer?
}


///////////////////////////////////////////////////////////////////////
// Submit Transfer
///////////////////////////////////////////////////////////////////////

IOKitError::Enum IOKitTransfer::Submit()
{

   time_done = timeout + DSIThread_GetSystemTime();

   IOKitError::Enum ret;
   switch(type)
   {
      case TransferType::CONTROL:
         ret = SubmitControlTransfer();
         break;

      case TransferType::INTERRUPT:
      case TransferType::BULK:
         ret = SubmitBulkTransfer();
         break;

      default:
         ret = IOKitError::INVALID_PARAM;
         break;
   }

   return ret;
}


IOKitError::Enum IOKitTransfer::SubmitControlTransfer()
{

   #if !defined (LIBUSB_NO_TIMEOUT_DEVICE)
      IOUSBDevRequestTO req;
   #else
      IOUSBDevRequest req;
   #endif

   /* IOUSBDeviceInterface expects the request in cpu endianess */
   req.bmRequestType     = control_setup.bmRequestType;
   req.bRequest          = control_setup.bRequest;
   /* these values should already be in bus order */
   req.wValue            = control_setup.wValue;
   req.wIndex            = control_setup.wIndex;
   req.wLength           = control_setup.wLength;
   /* data is stored after the libusb control block */
   req.pData             = buffer;
   req.completionTimeout = timeout;
   req.noDataTimeout     = timeout;

   /* all transfers in libusb-1.0 are async */
   IOReturn kresult;
   kresult = (*(dev_handle.usb_device))->DeviceRequestAsyncTO(dev_handle.usb_device, &req, IOKitTransfer::Callback, this);

   return darwin_to_libusb(kresult);
}


IOKitError::Enum IOKitTransfer::SubmitBulkTransfer()
{

   uint8_t pipeRef;
   uint8_t iface;
   if(EndpointToPipeRef(dev_handle, endpoint, pipeRef, iface) != IOKitError::SUCCESS)
      return IOKitError::NOT_FOUND;

   //Get the interface that we want to use
   const IOKitInterface& interface = dev_handle.interfaces[iface];

   //Get pipe properties
   uint8_t transferType;
   uint8_t direction;
   uint8_t number;
   uint8_t interval;
   uint16_t maxPacketSize;
   (*(interface.GetInterface()))->GetPipeProperties(interface.GetInterface(), pipeRef, &direction, &number,
						                &transferType, &maxPacketSize, &interval);

   //are we reading or writing?
   uint8_t is_read = endpoint & LIBUSB_ENDPOINT_IN;

   //submit the request
   IOReturn ret;

   //timeouts are unavailable on interrupt endpoints
   if(transferType == kUSBInterrupt)
   {
      if (is_read)
         ret = (*(interface.GetInterface()))->ReadPipeAsync(interface.GetInterface(), pipeRef, buffer,
                                              length, IOKitTransfer::Callback, this);
      else
         ret = (*(interface.GetInterface()))->WritePipeAsync(interface.GetInterface(), pipeRef, buffer,
                                              length, IOKitTransfer::Callback, this);
   }
   else
   {
      if (is_read)
      {
         //govern our own timeouts; when IOKit pipe transactions timeout they are meant to generate kernel warnings 
         pclTimer = new DSITimer(&IOKitTransfer::TimeoutTransfer, this, timeout, FALSE);
         if (pclTimer->NoError() == FALSE)
            return darwin_to_libusb(kIOUSBNoAsyncPortErr);  //other error
         ret = (*(interface.GetInterface()))->ReadPipeAsyncTO(interface.GetInterface(), pipeRef, buffer,
                                              length, timeout+1000, timeout+1000, IOKitTransfer::Callback, this); //make sure this call never times out, and is instead aborted; eliminates kernel warnings
      }
      else
         ret = (*(interface.GetInterface()))->WritePipeAsyncTO(interface.GetInterface(), pipeRef, buffer,
                                              length, timeout, timeout, IOKitTransfer::Callback, this);
   }
   return darwin_to_libusb(ret);
}

DSI_THREAD_RETURN IOKitTransfer::TimeoutTransfer(void *pvParameter_)
{
   IOKitTransfer* This = reinterpret_cast<IOKitTransfer*>(pvParameter_);

   This->Cancel();

   return 0;
}

///////////////////////////////////////////////////////////////////////
// Receive Transfer
///////////////////////////////////////////////////////////////////////


TransferStatus::Enum IOKitTransfer::Receive()
{
	unsigned long timeout_ms;
   unsigned long current_time = DSIThread_GetSystemTime();
   if(time_done < current_time)
      timeout_ms = 0;
   else
      timeout_ms = time_done - current_time;

   //Wait for response
	struct pollfd fds_to_poll[2];

	fds_to_poll[0].fd = return_pipe[0];
	fds_to_poll[0].events = POLLIN;
	fds_to_poll[0].revents = 0;

   fds_to_poll[1].fd = dev_handle.device_pipe[0];
	fds_to_poll[1].events = POLLIN;
	fds_to_poll[1].revents = 0;

   int ret;
   //struct pollfd* fds_to_poll_ptr = ;
	ret = poll(&(fds_to_poll[0]), 2, timeout_ms+60000); //!!

   TransferStatus::Enum status;
   switch(ret)
   {
      case 0:
         //timeout
         timedout = TRUE;
         status = TransferStatus::TIMED_OUT;
         break;

      case 2:
      case 1:
         for(int i=0; i<2; i++)  //does the return pipe first before the device pipe
         {
            if(fds_to_poll[i].revents == 1)
               status = HandleEvent(fds_to_poll[i]);  //!!status can get overwritten if there are two!
         }
         break;

      case -1:
         if(errno == EINTR)
         {
            //System call interrupted (perhaps due to signal)
            status = TransferStatus::INTERRUPTED;
         }
         //fall thru
      default:
         status = TransferStatus::ERROR;
         break;
	}

	return status;
}


TransferStatus::Enum IOKitTransfer::HandleEvent(struct pollfd& poll_fd)
{
  UInt32 io_size;
  IOReturn kresult;
  int ret;
  USBMessage::Enum message;


   if(poll_fd.revents == 0)
      return TransferStatus::ERROR;  //this should never happen!

   if(poll_fd.revents & POLLERR)
   {
      //could not poll the device-- response is to delete the device (this seems a little heavy-handed)
      message = USBMessage::DEVICE_GONE;
   }
   else
   {
      ret = read(poll_fd.fd, &message, sizeof(message));
      if(ret < static_cast<int>( sizeof(message) ) )
         return TransferStatus::ERROR;  //this should never happen!
   }


   TransferStatus::Enum status;
   switch(message)
   {
      case USBMessage::DEVICE_GONE:
            //done with this device
         status = HandleTransferCompletion(TransferStatus::NO_DEVICE);  //!!Do we need this?
            //!!do we need to cancel transfer?
         break;

      case USBMessage::ASYNC_IO_COMPLETE:
         read(poll_fd.fd, &kresult, sizeof(IOReturn));
         read(poll_fd.fd, &io_size, sizeof(UInt32));

         status = HandleCallback(kresult, io_size);
         break;

      default:
         //unknown message received from device pipe
         status = TransferStatus::ERROR;
         break;
    }

  return status;
}

TransferStatus::Enum IOKitTransfer::HandleCallback(kern_return_t result, UInt32 io_size)
{
   TransferStatus::Enum status;
   switch(type)
   {
      case TransferType::CONTROL:
         status = ControlCallback(result, io_size);
         break;
      case TransferType::BULK:
      case TransferType::INTERRUPT:
         status = BulkCallback(result, io_size);
         break;

      default:
         //unknown endpoint type
         status = TransferStatus::ERROR;
         break;
  }

  return status;
}

TransferStatus::Enum IOKitTransfer::ControlCallback(kern_return_t result, UInt32 io_size)
{
   TransferStatus::Enum status;

   switch(result)
   {
      case kIOReturnSuccess:
         status = TransferStatus::COMPLETED;
         transferred = io_size;
         break;

      case kIOReturnAborted:
         return HandleTransferCancellation();

      case kIOUSBPipeStalled:
         //unsupported control request
         status = TransferStatus::STALL;
         break;

      case iokit_family_err(sub_iokit_usb, kIOUSBTransactionTimeout):
         status = TransferStatus::TIMED_OUT;
         break;

      default:
         //control error
         status = TransferStatus::ERROR;
		 break;
  }

  return HandleTransferCompletion(status);
}

TransferStatus::Enum IOKitTransfer::BulkCallback(kern_return_t result, UInt32 io_size)
{

   TransferStatus::Enum status;

   switch (result)
   {
      case kIOReturnSuccess:
         status = TransferStatus::COMPLETED;
         transferred = io_size;
         break;

      case kIOReturnAborted:
         return HandleTransferCancellation();

      case kIOUSBPipeStalled:
         //unsupported control request
         status = TransferStatus::STALL;
         break;

      case iokit_family_err(sub_iokit_usb, kIOUSBTransactionTimeout):
         status = TransferStatus::TIMED_OUT;
         break;

      default:
         //control error
         status = TransferStatus::ERROR;
         break;
  }

  return HandleTransferCompletion(status);
}



///////////////////////////////////////////////////////////////////////
// Cancel Transfer
///////////////////////////////////////////////////////////////////////

IOKitError::Enum IOKitTransfer::Cancel()
{
   DSIThread_MutexLock(&stMutexCriticalSection);
   //ensure a single message per transfer, in the event both completion and abortion fire at close proximity to each other
   if (!transfer_complete)
   {
      transfer_complete = TRUE;
      timedout = TRUE;

      IOReturn result = kIOReturnAborted;
      UInt32 io_size = 0;

      /* send a completion message to the device's file descriptor */
      UInt32 message = USBMessage::ASYNC_IO_COMPLETE;

      write(return_pipe[1], &message, sizeof(message));
      write(return_pipe[1], &result, sizeof(IOReturn));
      write(return_pipe[1], &io_size, sizeof(UInt32));
   }
   DSIThread_MutexUnlock(&stMutexCriticalSection);

   switch(type)
   {
      case TransferType::CONTROL:
         return CancelControlTransfer();
      case TransferType::BULK:
      case TransferType::INTERRUPT:
      case TransferType::ISOCHRONOUS:
         return AbortTransfer();
      default:
         return IOKitError::INVALID_PARAM;
  }
}


IOKitError::Enum IOKitTransfer::CancelControlTransfer() const
{
   //WARNING: aborting all transactions control pipe

   IOReturn kresult;
   kresult = (*(dev_handle.usb_device))->USBDeviceAbortPipeZero(dev_handle.usb_device);

   return darwin_to_libusb(kresult);
}


IOKitError::Enum IOKitTransfer::AbortTransfer() const
{

  uint8_t pipeRef;
  uint8_t iface;
  if(EndpointToPipeRef(dev_handle, endpoint, pipeRef, iface) != IOKitError::SUCCESS)
    return IOKitError::NOT_FOUND;

  const IOKitInterface& interface = dev_handle.interfaces[iface];

  //WARNING: aborting all transactions on interface %d pipe %d

  //abort transactions
  (*(interface.GetInterface()))->AbortPipe(interface.GetInterface(), pipeRef);


   IOReturn kresult = kIOReturnSuccess;
   #if defined(CLEAR_PIPE)
   {
      //calling clear pipe stall to clear the data toggle bit

      /* clear the data toggle bit */
      #if (InterfaceVersion < 190)
         kresult = (*(cInterface->interface))->ClearPipeStall(cInterface->interface, pipeRef);
      #else
         /* newer versions of darwin support clearing additional bits on the device's endpoint */
         kresult = (*(cInterface->interface))->ClearPipeStallBothEnds(cInterface->interface, pipeRef);
      #endif
   }
   #endif

  return darwin_to_libusb(kresult);
}


///////////////////////////////////////////////////////////////////////
// Other Functions
///////////////////////////////////////////////////////////////////////


IOKitError::Enum IOKitTransfer::EndpointToPipeRef(const IOKitDeviceHandle& dev_handle, uint8_t ep, uint8_t& pipep, uint8_t& ifcp)
{

  for(int8_t iface = 0; iface < IOKitDeviceHandle::MAX_INTERFACES; iface++)
  {
      const IOKitInterface& interface = dev_handle.interfaces[iface];
      if(!interface.IsClaimed())
         continue;

      for(int8_t i = 0; i < IOKitInterface::MAX_ENDPOINTS; i++)
      {
         if(interface.GetEndpoint(i) == ep)
         {
            pipep = i + 1;
            ifcp = iface;
            return IOKitError::SUCCESS;
         }
      }
   }

  return IOKitError::NOT_FOUND;
}


/* Similar to usbi_handle_transfer_completion() but exclusively for transfers
 * that were asynchronously cancelled. The same concerns w.r.t. freeing of
 * transfers exist here.
 */
TransferStatus::Enum IOKitTransfer::HandleTransferCancellation()
{

	//if the URB was cancelled due to timeout, report timeout to the user
	if(timedout == TRUE)
		return HandleTransferCompletion(TransferStatus::TIMED_OUT);

	//otherwise its a normal async cancel
	return HandleTransferCompletion(TransferStatus::CANCELLED);
}


/* Handle completion of a transfer (completion might be an error condition).
 * This will invoke the user-supplied callback function, which may end up
 * freeing the transfer. Therefore you cannot use the transfer structure
 * after calling this function, and you should free all backend-specific
 * data before calling it. */
TransferStatus::Enum IOKitTransfer::HandleTransferCompletion(TransferStatus::Enum status_)
{
   actual_length = transferred;

   return status_;
}





void IOKitTransfer::Callback(void* refcon, IOReturn result, void* arg0)
{
   IOKitTransfer* This = reinterpret_cast<IOKitTransfer*>(refcon);
   UInt32 message = USBMessage::ASYNC_IO_COMPLETE; /* send a completion message to the device's file descriptor */

   DSIThread_MutexLock(&(This->stMutexCriticalSection));
   if (!This->transfer_complete)
   {
      This->transfer_complete = TRUE;
      UInt32* io_size = reinterpret_cast<UInt32*>(arg0);  //!!should we copy the value instead?

      //printf(">>>Callback %X, %X, %X\n", err_get_system(result), err_get_sub(result), err_get_code(result) );

      write(This->return_pipe[1], &message, sizeof(message));
      write(This->return_pipe[1], &result, sizeof(IOReturn));
      write(This->return_pipe[1], &io_size, sizeof(UInt32));
   }
   DSIThread_MutexUnlock(&(This->stMutexCriticalSection));
   return;
}

#endif //defined(DSI_TYPES_MACINTOSH)