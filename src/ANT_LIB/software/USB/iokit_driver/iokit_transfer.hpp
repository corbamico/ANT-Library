 /*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */


#ifndef IOKIT_TRANSFER_HPP
#define IOKIT_TRANSFER_HPP

#include "types.h"

#include "iokit_types.hpp"

#include "usb_standard_types.hpp"
#include "dsi_timer.hpp"
#include "dsi_thread.h"

//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////


struct TransferStatus
{
   enum Enum
   {
      //Transfer completed without error. Note that this does not indicate
      //that the entire amount of requested data was transferred.
      COMPLETED,

      //Transfer failed
      ERROR,

      //Transfer timed out
      TIMED_OUT,

      //Transfer was cancelled
      CANCELLED,

      //For bulk/interrupt endpoints: halt condition detected (endpoint
      //  stalled). For control endpoints: control request not supported.
      STALL,

      //Device was disconnected
      NO_DEVICE,

      //Device sent more data than requested
      OVERFLOWED,

      //transfer was interrupted by a system signal
      INTERRUPTED
   };

   private: TransferStatus();

};


struct TransferType
{
   enum Enum
   {
      //Control endpoint
      CONTROL = 0,

      //Isochronous endpoint
      ISOCHRONOUS = 1,

      //Bulk endpoint
      BULK = 2,

      //Interrupt endpoint
      INTERRUPT = 3
   };

   private: TransferType();

};

struct USBMessage  //!!Can be private!
{
   enum Enum
   {
      DEVICE_GONE,
      ASYNC_IO_COMPLETE
   };

   private: USBMessage();
};


//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

class IOKitDeviceHandle;


//NOTE: If this is an isochronous transfer, status may read COMPLETED even
//if there were errors in the frames. Use the
//\ref libusb_iso_packet_descriptor::status "status" field in each packet
//to determine if errors occurred.

//!!All input data should be const!
//!!Make sure all data is initialized!
//!!NOTE: This transfer is expected to only be sent once!

class IOKitTransfer
{

   public:
      static IOKitTransfer* MakeControlTransfer(const IOKitDeviceHandle& dev_handle_, struct libusb_control_setup& setup_,
                             const int pipe[2],
                             unsigned char data_buffer_[],
                             unsigned int timeout_);

      static IOKitTransfer* MakeInterruptTransfer(const IOKitDeviceHandle& dev_handle_, unsigned char endpoint_,
                             const int pipe[2],
                             unsigned char data_buffer_[], unsigned long data_length_,
                             unsigned int timeout_);

      static IOKitTransfer* MakeBulkTransfer(const IOKitDeviceHandle& dev_handle_, unsigned char endpoint_,
                             const int pipe[2],
                             unsigned char data_buffer_[], unsigned long data_length_,
                             unsigned int timeout_);


      IOKitError::Enum Submit();
      TransferStatus::Enum Receive();
      IOKitError::Enum Cancel();

      unsigned char* GetDataBuffer() const { return buffer; }  //can someone change our buffer pointer unless we return unsigned char* const?
      int GetDataLength() const { return actual_length; }

      virtual ~IOKitTransfer();


   protected:
      // Control Transfer //
      IOKitTransfer(const IOKitDeviceHandle& dev_handle_, struct libusb_control_setup& setup_,
                             const int pipe[2],
                             unsigned char data_buffer_[],
                             unsigned char type_, unsigned int timeout_);
      // Interrupt/Bulk Transfer //
      IOKitTransfer(const IOKitDeviceHandle& dev_handle_,
                             const int pipe[2], unsigned char endpoint_,
                             unsigned char data_buffer_[], unsigned long data_length_,
                             unsigned char type_, unsigned int timeout_);

   private:

      ///////////
      // SUBMIT
      ///////////
      IOKitError::Enum SubmitControlTransfer();
      IOKitError::Enum SubmitBulkTransfer();


      ///////////
      // RECEIVE
      ///////////
      TransferStatus::Enum HandleEvent(struct pollfd& poll_fd);

      TransferStatus::Enum HandleCallback(kern_return_t result, UInt32 io_size);
      TransferStatus::Enum ControlCallback(kern_return_t result, UInt32 io_size);
      TransferStatus::Enum BulkCallback(kern_return_t result, UInt32 io_size);

      TransferStatus::Enum HandleTransferCancellation();
      TransferStatus::Enum HandleTransferCompletion(TransferStatus::Enum status_);

      ///////////
      // CANCEL
      ///////////
      IOKitError::Enum CancelControlTransfer() const;
      IOKitError::Enum AbortTransfer() const;
      static DSI_THREAD_RETURN TimeoutTransfer(void *pvParameter_);
      BOOL transfer_complete; //used to ensure a transfer abort does not discard properly transferred contents

      ///////////
      // OTHER
      ///////////


      static void Callback(void* refcon, IOReturn result, void* arg0);  //IOAsyncCallback1 function
      static IOKitError::Enum EndpointToPipeRef(const IOKitDeviceHandle& dev_handle, uint8_t ep, uint8_t& pipep, uint8_t& ifcp);


      int device_pipe[2];
      int return_pipe[2];
      DSI_MUTEX stMutexCriticalSection;

      //SUBMIT DATA
      const unsigned char endpoint;  //Address of the endpoint where this transfer will be sent.
      const unsigned char type; //Type of the endpoint from \ref libusb_transfer_type
      const unsigned int timeout;  //Timeout for this transfer in millseconds. A value of 0 indicates no timeout.
      const libusb_control_setup control_setup;  //setup for control transfers
      DSITimer *pclTimer; //used to initiate USB transfer timeouts, instead of making timeouts the responsibility of the IOKit, which generates kernel warnings

      //RECEIVE DATA
      int transferred;  //length of data transferred (the variable that is not read by user)
      int actual_length;  //length of data that was transferred (the variable that is read by user)

      //SUBMIT/RECEIVE
      unsigned char* const buffer;  //Length of full data buffer  //!!Should we have our own buffer?  Should there be a read and const write buffer?
      const int length;             //Length of the data buffer
      const IOKitDeviceHandle& dev_handle; 		//Handle of the device that this transfer will be submitted to

      ////////////////////
      // usbi_transfer
      ////////////////////
      unsigned long time_done;  //system time the transfer should be done before or else it times out
      BOOL timedout;  //whether transfer timed out


};

#endif // !defined(USB_TRANSFER_HPP)

