/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright � 1998-2011 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
 

#if !defined(ANTFS_HOST_CHANNEL_HPP)
#define ANTFS_HOST_CHANNEL

#include "types.h"
#include "dsi_thread.h"
#include "dsi_timer.hpp"
#include "dsi_framer_ant.hpp"
#include "dsi_debug.hpp"

#include "dsi_response_queue.hpp"
#include "dsi_ant_message_processor.hpp"
#include "antfsmessage.h"

#include "antfs_host_interface.hpp"


//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////

#define DIRECT_TRANSFER_SIZE           ((MAX_USHORT + 1) * 8)
#define MAX_IGNORE_LIST_SIZE           2048
UCHAR const aucTransportFrequencyList[16] = {3 ,7 ,15,20,25,29,34,40,45,49,54,60,65,70,75,80};
#define TRANSPORT_FREQUENCY_LIST_SIZE  ((UCHAR)sizeof(aucTransportFrequencyList))
#define SEARCH_DEVICE_LIST_MAX_SIZE    512

typedef struct
{
   USHORT usHandle;
   ANTFS_DEVICE_PARAMETERS sDeviceParameters;
   ANTFS_DEVICE_PARAMETERS sDeviceSearchMask;
} DEVICE_PARAMETERS_ITEM;

typedef struct
{
   USHORT usID;
   USHORT usManufacturerID;
   USHORT usDeviceType;
   USHORT usTimeout;
} IGNORE_LIST_ITEM;

/////////////////////////////////////////////////////////////////
// This class implements the ANT-FS Host specification.
// It is intended to be used together with another class that manages
// the connection to an ANT USB stick (e.g. DSIANTDevice or
// .NET ANT_Device).
/////////////////////////////////////////////////////////////////
class ANTFSHostChannel : public ANTFSHostInterface, public DSIANTMessageProcessor
{
   private:

      //////////////////////////////////////////////////////////////////////////////////
      // Private Definitions
      //////////////////////////////////////////////////////////////////////////////////

      enum ENUM_ANTFS_REQUEST
      {
         ANTFS_REQUEST_NONE = 0,
         ANTFS_REQUEST_INIT,                                    // Init()
         ANTFS_REQUEST_SESSION_REQ,                             // RequestSession()
         ANTFS_REQUEST_SEARCH,                                  // SearchForDevice()
         ANTFS_REQUEST_ID,                                      // Request ID
         ANTFS_REQUEST_PING,                                    // Ping
         ANTFS_REQUEST_PAIR,                                    // Pair
         ANTFS_REQUEST_PROCEED,                                 // ProceedToTransport
         ANTFS_REQUEST_AUTHENTICATE,                            // Authenticate()
         ANTFS_REQUEST_CONNECT,                                 // Connect
         ANTFS_REQUEST_DISCONNECT,                              // Disconnect()
         ANTFS_REQUEST_CHANGE_LINK,                             // ChangeTransportChannelParameters()
         ANTFS_REQUEST_DOWNLOAD,                                // Download()
         ANTFS_REQUEST_UPLOAD,                                  // Upload()
         ANTFS_REQUEST_MANUAL_TRANSFER,                         // ManualTransfer()
         ANTFS_REQUEST_ERASE,                                   // EraseData()         
         ANTFS_REQUEST_CONNECTION_LOST,                         // Internal, keep these at the end of the list order is important
         ANTFS_REQUEST_HANDLE_SERIAL_ERROR,                     // Internal
         ANTFS_REQUEST_SERIAL_ERROR_HANDLED                     // Internal
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

      DSIResponseQueue<ANTFS_HOST_RESPONSE> clResponseQueue;

      volatile BOOL bIgnoreListRunning;
      volatile BOOL bForceFullInit;
      BOOL bInitFailed;
	   BOOL bPingEnabled;
      BOOL bForceUploadOffset;
      BOOL bRequestPageEnabled;

      UCHAR aucResponseBuf[MESG_MAX_SIZE_VALUE];
      UCHAR aucRxBuf[MESG_MAX_SIZE_VALUE];
      UCHAR aucTxBuf[MESG_MAX_SIZE_VALUE];

      UCHAR aucRemoteFriendlyName[FRIENDLY_NAME_MAX_LENGTH];
      UCHAR ucRemoteFriendlyNameLength;
      UCHAR ucAuthType;
      UCHAR aucTxPassword[TX_PASSWORD_MAX_LENGTH];
      volatile UCHAR ucTxPasswordLength;
      volatile UCHAR ucPasswordLength;

      UCHAR *pucUploadData;

      UCHAR *pucResponseBuffer;
      UCHAR *pucResponseBufferSize;
      ULONG ulAuthResponseTimeout;

      ULONG ulHostSerialNumber;
      ULONG ulFoundBeaconHostID;
      UCHAR ucChannelStatus;
      UCHAR ucRejectCode;

      UCHAR ucDisconnectType;
      UCHAR ucUndiscoverableTimeDuration;
      UCHAR ucUndiscoverableAppSpecificDuration;

      ANTFS_DEVICE_PARAMETERS stFoundDeviceParameters;
      volatile USHORT usFoundBeaconPeriod;
      volatile USHORT usFoundANTFSDeviceID;
      volatile USHORT usFoundANTFSManufacturerID;
      volatile USHORT usFoundANTFSDeviceType;
      volatile UCHAR ucFoundANTDeviceType;
      volatile UCHAR ucFoundANTTransmitType;

      volatile BOOL bFoundDeviceHasData;
      volatile BOOL bFoundDeviceUploadEnabled;
      volatile BOOL bFoundDeviceInPairingMode;
      volatile UCHAR ucFoundDeviceAuthenticationType;
      volatile UCHAR ucFoundDeviceState;
      volatile BOOL bFoundDevice;
      volatile BOOL bFoundDeviceIsValid;
	   volatile BOOL bNewRxEvent;

      // Download Data
      UCHAR *pucTransferBuffer;
      UCHAR *pucTransferBufferDynamic;
      UCHAR aucTransferBufferFixed[MAX_USHORT + 16];
      UCHAR aucSendDirectBuffer[8 + DIRECT_TRANSFER_SIZE];
      volatile ULONG ulTransferArrayIndex;
      volatile ULONG ulPacketCount;
      volatile ULONG ulUploadIndexProgress;
      volatile BOOL bTxError;
      volatile BOOL bRxError;
      volatile BOOL bReceivedBurst;
      volatile BOOL bReceivedResponse;
      volatile ULONG ulTransferTotalBytesRemaining;
      volatile ULONG ulTransferBytesInBlock;
      volatile BOOL bTransfer;

      ULONG ulHostBlockSize;

      DSI_THREAD_ID hANTFSThread;                           // Handle for the ANTFS thread.
      DSI_MUTEX stMutexResponseQueue;                       // Mutex used with the response queue
      DSI_MUTEX stMutexCriticalSection;                     // Mutex used with the wait condition
      DSI_MUTEX stMutexIgnoreListAccess;                    // Mutex used with the ignore list
      DSI_CONDITION_VAR stCondANTFSThreadExit;              // Event to signal the ANTFS thread has ended.
      DSI_CONDITION_VAR stCondRequest;                      // Event to signal there is a new request
      DSI_CONDITION_VAR stCondRxEvent;                      // Event to signal there is a new Rx message or failure
      DSI_CONDITION_VAR stCondWaitForResponse;              // Event to signal there is a new response to the application

      volatile USHORT usSerialWatchdog;
      volatile BOOL bKillThread;
	   volatile BOOL bANTFSThreadRunning;
      volatile BOOL bCancel;     // Internal cancel parameter to use if not configured
      volatile BOOL *pbCancel;

      DSIFramerANT *pclANT;
      
      // ANT Channel parameters
      UCHAR ucNetworkNumber;
      //UCHAR ucChannelNumber;
      USHORT usRadioChannelID;
      UCHAR ucTheDeviceType;            
      UCHAR ucTheTransmissionType;    
      USHORT usTheMessagePeriod;       
      UCHAR aucTheNetworkkey[8];     
      UCHAR ucTheProxThreshold;
      
      volatile USHORT usSearchManufacturerID;
      volatile USHORT usSearchDeviceType;

      volatile USHORT usBlackoutTime;
      volatile UCHAR ucTransportLayerRadioFrequency;

      UCHAR ucSearchRadioFrequency;

      USHORT usTransferDataFileIndex;
      volatile ULONG ulTransferDataOffset;
      volatile ULONG ulTransferByteSize;
      volatile ULONG ulTransferBufferSize;
      volatile BOOL bLargeData;

      // Ignore List variables
      volatile USHORT usListIndex;
      IGNORE_LIST_ITEM astIgnoreList[MAX_IGNORE_LIST_SIZE];
      BOOL bTimerThreadInitDone;
      DSITimer *pclQueueTimer;

      volatile UCHAR ucLinkResponseRetries;

      UCHAR ucStrikes;
      volatile UCHAR ucTransportBeaconTicks;

      UCHAR ucTransportChannelPeriodSelection;
      UCHAR ucTransportFrequencySelection;
      UCHAR ucTransportFrequencyStaleCount;
      UCHAR ucCurrentTransportFreqElement;
      UCHAR aucFrequencyTable[TRANSPORT_FREQUENCY_LIST_SIZE];

      DEVICE_PARAMETERS_ITEM asDeviceParametersList[SEARCH_DEVICE_LIST_MAX_SIZE];
      USHORT usDeviceListSize;

      volatile ANTFS_HOST_STATE eANTFSState;
      volatile ENUM_ANTFS_REQUEST eANTFSRequest;

      //////////////////////////////////////////////////////////////////////////////////
      // Private Function Prototypes
      //////////////////////////////////////////////////////////////////////////////////
      BOOL ReportDownloadProgress(void);
      BOOL ReInitDevice(void);
      void ResetHostState(void);

      RETURN_STATUS AttemptSearch(void);
      RETURN_STATUS AttemptConnect(void);
      RETURN_STATUS AttemptRequestSession(void);
      RETURN_STATUS AttemptSwitchFrequency(void);
      RETURN_STATUS AttemptDisconnect(void);
      void Ping(void);
      RETURN_STATUS AttemptDownload(void);
      RETURN_STATUS UploadLoop(void);
      RETURN_STATUS AttemptUpload(void);
      RETURN_STATUS AttemptManualTransfer(void);
      RETURN_STATUS Receive(void);
      RETURN_STATUS AttemptErase(void);
      RETURN_STATUS AttemptAuthenticate(void);

      // ANT Callback Functions
      BOOL FilterANTMessages(ANT_MESSAGE* pstMessage_, UCHAR ucANTChannel_);
      BOOL ANTProtocolEventProcess(UCHAR ucChannel_, UCHAR ucMessageCode_);
      BOOL ANTChannelEventProcess(UCHAR ucChannel_, UCHAR ucMessageCode_);

	   void QueueTimerCallback(void);
	   static DSI_THREAD_RETURN QueueTimerStart(void *pvParameter_);

      void HandleSerialError(void);

      void IncFreqStaleCount(UCHAR ucInc);
      void PopulateTransportFreqTable(void);
      UCHAR CheckForNewTransportFreq(void);

      void ANTFSThread(void);
      static DSI_THREAD_RETURN ANTFSThreadStart(void *pvParameter_);

      BOOL IsDeviceMatched(ANTFS_DEVICE_PARAMETERS *psDeviceParameters_, BOOL bPartialID_);
      void AddResponse(ANTFS_HOST_RESPONSE eResponse_);

public:
      //////////////////////////////////////////////////////////////////////////////////
      // Public Function Prototypes
      //////////////////////////////////////////////////////////////////////////////////
      
      ANTFSHostChannel();
      ~ANTFSHostChannel();      

      BOOL Init(DSIFramerANT* pclANT_, UCHAR ucChannel_);
      /////////////////////////////////////////////////////////////////
      // Begins to initialize the ANTFSHostChannel object.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Parameters:
      //    *pclANT_:         Pointer to a DSIFramerANT object.
      //    ucChannel_:       Channel number to use for the ANT-FS host
      // Operation:
      //    This function is used from a class managing the connection
      //    to ANT (e.g. DSIANTDevice or .NET ANT_Device), to
      //    initialize the ANTFSHostChannel object. It is not possible
      //    to change the channel number once ANT-FS is running.
      //    The function returns immediately, and the ANTFSHostChannel object
      //    will send a response of ANTFS_HOST_RESPONSE_INIT_PASS.
      //    IT IS NOT NECESSARY TO CALL THIS FUNCTION DIRECTLY FROM USER APPLICATIONS.
      /////////////////////////////////////////////////////////////////

      void Close(void);
      /////////////////////////////////////////////////////////////////
      // Stops any pending actions, closes all threads and cleans
      // up any dynamic memory being used by the library.
      // Operation:
      //    This function is used from a class managing the connection
      //    to ANT (e.g. DSIANTDevice or .NET ANT_Device), to
      //    clean up any resources in use by the ANT-FS host.
      //    IT IS NOT NECESSARY TO CALL THIS FUNCTION DIRECTLY FROM USER APPLICATIONS.
      /////////////////////////////////////////////////////////////////

      void ProcessMessage(ANT_MESSAGE* pstMessage_, USHORT usMesgSize_);
      /////////////////////////////////////////////////////////////////
      // Processes incoming ANT messages as per the ANT-FS Technology
      // Specification
      // Parameters:
      //    pstMessage_:      Pointer to an ANT message structure
      //    usMesgSize_:      ANT message size
      // Operation:
      //    This function is used from a class managing the connection
      //    to ANT (e.g. DSIANTDevice or .NET ANT_Device).
      //    IT IS NOT NECESSARY TO CALL THIS FUNCTION DIRECTLY FROM USER APPLICATIONS.
      /////////////////////////////////////////////////////////////////

      void ProcessDeviceNotification(ANT_DEVICE_NOTIFICATION eCode_, void* pvParameter_);
      /////////////////////////////////////////////////////////////////
      // Processes device level notifications
      // Parameters:
      //    eCode_:           Device notification event code
      //    pvParameter_:     Pointer to struct defining specific parameters related 
      //                      to the event code
      // Operation:
      //    This function si used from a class managing the connection
      //    to ANT (e.g. DSIANTDevice or .NET ANT_Device).
      //    IT IS NOT NECESSARY TO CALL THIS FUNCTION DIRECTLY FROM USER APPLICATIONS.
      /////////////////////////////////////////////////////////////////

      void Cancel(void);
      /////////////////////////////////////////////////////////////////
      // Cancels any pending actions and returns the library to the
      // appropriate ANTFS layer if possible.  ie if the library was
      // executing a download command in the transport layer, the
      // library would be returned to ANTFS_HOST_STATE_TRANSPORT after
      // execution of this function.
      /////////////////////////////////////////////////////////////////

      void SetChannelID(UCHAR ucDeviceType_, UCHAR ucTransmissionType_);
      /////////////////////////////////////////////////////////////////
      // Configures the ANT channel ID for the ANT-FS Host channel
      // Parameters:
      //    ucDeviceType_:    ANT Channel Device Type
      //    ucTransmissionType_: ANT Channel Transmission Type
      // Operation:
      //    Changes to the channel ID are only applied after calling
      //    SearchForDevice.
      /////////////////////////////////////////////////////////////////

      void SetChannelPeriod(USHORT usChannelPeriod_);
      /////////////////////////////////////////////////////////////////
      // Configures the ANT channel period for the ANT-FS Host channel
      // Parameters:
      //    usChannelPeriod_: Channel period to use while in link state
      // Operation:
      //    Changes to the channel ID are only applied after calling
      //    SearchForDevice.  If using ANT-FS broadcast, this is the
      //    channel period the host will return to when using the 
      //    Disconnect command with the Return To Broadcast option.
      //    To change the channel period while an ANT-FS session is open,
      //    use SwitchFrequency().
      /////////////////////////////////////////////////////////////////

      // TODO: Does this function make sense here, or should it just assign the network number?
      //       The ANTFSHost wrapper class could continue to provide this one for backwards compatibility
      void SetNetworkKey(UCHAR ucNetwork_, UCHAR ucNetworkkey[]);
      /////////////////////////////////////////////////////////////////
      // Configures the ANT network key for the ANT-FS channel
      // Parameters:
      //    ucNetwork_:       Network number (0 - 2)
      //    ucNetworkkey:     Array containing the 8 byte network key
      // Operation:
      //    Changes to the network key are only applied after calling
      //    SearchForDevice.
      /////////////////////////////////////////////////////////////////

      void SetProximitySearch(UCHAR ucSearchThreshold_);
      /////////////////////////////////////////////////////////////////
      // Sets the value for the proximity bin setting for searching. 
      // Note: If applying this value fails when attempting to start search,
      //    it is ignored to maintain compatibility with devices that
      //    do not support this feature. This means that a real failure can occur
      //    on a device that actually does support it, and it will be missed. The
      //    debug log will show if this command fails.
      // Parameters:
      //    ucSearchThreshold_:  Desired proximity bin from 0-10 (If value is 
      //                         set higher then 10, it is automatically capped at 10)
      // Operation:
      //    Changes to the proximity search bin are only applied after calling
      //    SearchForDevice.
      ///////////////////////////////////////////////////////////////// 



      void SetSerialNumber(ULONG ulSerialNumber_);
      /////////////////////////////////////////////////////////////////
      // Configures the host serial number.
      // Parameters:
      //    ulSerialNumber_:     4-byte host serial number
      /////////////////////////////////////////////////////////////////

      BOOL EnablePing(BOOL bEnable_);
      /////////////////////////////////////////////////////////////////
      // Enables ping message to be sent to the remote device periodically.
      // This can be used to keep the remote device from timing out during
      // operations that wait for user input (i.e. pairing)
      // Parameters:
      //    bEnable_:     Periodic ping enable.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      /////////////////////////////////////////////////////////////////      

      BOOL GetEnabled(void);
      /////////////////////////////////////////////////////////////////
      // Returns the current status of ANT-FS message processing
      // Returns TRUE if ANT-FS is enabled.  Otherwise, it returns FALSE.
      // Operation:
      //    This function is used from a class managing the connection
      //    to ANT (e.g. DSIANTDevice or .NET ANT_Device).
      //    IT IS NOT NECESSARY TO CALL THIS FUNCTION DIRECTLY FROM USER APPLICATIONS.
      /////////////////////////////////////////////////////////////////

      ANTFS_HOST_STATE GetStatus(void);
      /////////////////////////////////////////////////////////////////
      // Returns the current library status.
      /////////////////////////////////////////////////////////////////

      BOOL GetFoundDeviceParameters(ANTFS_DEVICE_PARAMETERS *pstDeviceParameters_, UCHAR *aucFriendlyName_, UCHAR *pucBufferSize_);
      /////////////////////////////////////////////////////////////////
      // Copies the parameters of the most recently found device to the
      // supplied parameter.
      // Parameters:
      //    *ptsDeviceParameters_:  A pointer to a structure that will
      //                      receive a copy of the device parameters.
      //    *aucFriendlyName_: A pointer to a buffer where the remote
      //                      device friendly name will be copied.
      //    *pucBufferSize_:  Pointer to a UCHAR variable that should contain the
      //                      maximum size of the buffer pointed to by aucFriendlyName_.
      //                      After the function returns, the UCHAR variable
      //                      will be set to reflect the size of friendly name string
      //                      that has been copied to aucFriendlyName_.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      /////////////////////////////////////////////////////////////////      
      
      BOOL GetFoundDeviceChannelID(USHORT *pusDeviceNumber_ = (USHORT *)NULL, UCHAR *pucDeviceType_ = (UCHAR *)NULL, UCHAR *pucTransmitType_ = (UCHAR *)NULL);
      /////////////////////////////////////////////////////////////////
      // Copies the ANT channel ID of the most recently found device to 
      // the supplied parameters.
      // Parameters:
      //    *pusDeviceNumber_:  A pointer to a USHORT variable that will
      //                      receive the ANT device number of the device.
      //    *pucDeviceType_: A pointer to a UCHAR variable that will
      //                      hold the device type of the found device.
      //    *pucTransmitType_:  Pointer to a UCHAR variable that will
      //                      receive the transmission type of the found 
      //                      device
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      /////////////////////////////////////////////////////////////////

      BOOL GetUploadStatus(ULONG *pulByteProgress_, ULONG *pulTotalLength_);
      /////////////////////////////////////////////////////////////////
      // Gets the transfer progress of a pending or a complete
      // upload.
      // Parameters:
      //    *pulByteProgress_: Pointer to a ULONG that will receive
      //                      the current byte progress of a pending or
      //                      complete upload.
      //    *pulTotalLength_: Pointer to a ULONG that will receive the
      //                      total length of the file being uploaded.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Operation:
      // A data upload occurs when information is requested to be sent to
      // a remote device.  This function may be called at any point
      // during an upload as a progress indicator.  After the upload
      // is complete, this information is valid until another request
      // for a data transfer is made.
      /////////////////////////////////////////////////////////////////

      BOOL GetDownloadStatus(ULONG *pulByteProgress_, ULONG *pulTotalLength_);
      /////////////////////////////////////////////////////////////////
      // Gets the transfer progress of a pending or a complete
      // download.
      // Parameters:
      //    *pulByteProgress_:   Pointer to a ULONG that will receive
      //                      the current byte progress of a pending or
      //                      complete download.
      //    *pulTotalLength_: Pointer to a ULONG that will receive the
      //                      total expected length of the download.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // Operation:
      // A data download occurs when information is requested from a
      // remote device.  This function may be called at any point
      // during a download as a progress indicator.  After the transfer
      // is complete, this information is valid until another request
      // for a data transfer is made.
      /////////////////////////////////////////////////////////////////

      BOOL GetTransferData(ULONG *pulDataSize_ , void *pvData_ = NULL);
      /////////////////////////////////////////////////////////////////
      // Gets the received data from a transfer (download/manual transfer).
      //
      // Parameters:
      //    *ulDataSize_     :   Pointer to a ULONG that will receive
      //                      the size of the data available in bytes.
      //    *pvData_        : Pointer to a buffer where the received data
      //                      will be copied.  NULL can be passed to this
      //                      parameter so that the size can be retried
	   //                      without copying any data.  The application
	   //                      can then call this function again to after it
	   //                      has allocated a buffer of sufficient size to
	   //                      handle the data.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      /////////////////////////////////////////////////////////////////

      UCHAR GetVersion(UCHAR *pucVersionString_, UCHAR ucBufferSize_);
      /////////////////////////////////////////////////////////////////
      // Copies at most ucBufferSize_ characters from the version
      // string into the supplied pucVersionString_ buffer.
      // Parameters:
      //    *pucVersionString_   Pointer to a buffer of characters into
      //                         which to receive the version string.
      //    ucBufferSize_        The maximum number of characters to
      //                         copy into *pucVersionString_.
      // Returns the number of characters copied to *pucVersionString_.
      // Operation:
      //    If the version string has fewer than ucBufferSize_
      //    characters, the *pucVersionString_ will be padded with a
      //    '\n'.
      /////////////////////////////////////////////////////////////////

      ANTFS_RETURN RequestSession(UCHAR ucBroadcastRadioFrequency_ = ANTFS_RF_FREQ, UCHAR ucConnectRadioFrequency_ = ANTFS_DEFAULT_TRANSPORT_FREQ);
      /////////////////////////////////////////////////////////////////
      // Requests an ANT-FS session from currently connected broadcast
      // device
      // Parameters:
      //    ucBroadcastRadioFrequency_:   This specifies the frequency on
      //                      which the broadcast device is transmitting.
      //                      This frequency is calculated as
      //                      (ucBroadcastRadioFrequency_ * 1 MHz +
      //                      2400 MHz).  MAX_UCHAR (0xFF) is reserved.
      //    ucConnectRadioFrequency_:   This specifies the frequency
      //                      on which the connection communication
      //                      will occur.  This frequency is calculated
      //                      as (ucConnectRadioFrequency_ * 1 MHz +
      //                      2400 MHz).  If ucConnectRadioFrequency_
      //                      is set to ANTFS_AUTO_FREQUENCY_SELECTION
      //                      (0xFF), then the library will use an
      //                      adaptive frequency-hopping scheme.
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once this function is called successfully, the application
      // must wait for the response from the ANTFSHost object.
      // Possible responses are:
      //    ANTFS_HOST_RESPONSE_CONNECT_PASS
      //    ANTFS_HOST_RESPONSE_REQUEST_SESSION_FAIL,
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      // To return to broadcast state, use the Disconnect() function.
      /////////////////////////////////////////////////////////////////
      
      ANTFS_RETURN SearchForDevice(UCHAR ucSearchRadioFrequency_ = ANTFS_RF_FREQ, UCHAR ucConnectRadioFrequency_ = ANTFS_DEFAULT_TRANSPORT_FREQ, USHORT usRadioChannelID_ = 0, BOOL bUseRequestPage_ = FALSE);
      /////////////////////////////////////////////////////////////////
      // Begins a search for ANTFS remote devices.
      // Parameters:
      //    ucSearchRadioFrequency_:   This specifies the frequency on
      //                      which to search for devices.  This
      //                      frequency is calculated as
      //                      (ucSearchRadioFrequency_ * 1 MHz +
      //                      2400 MHz).  MAX_UCHAR (0xFF) is reserved.
      //    ucConnectRadioFrequency_:   This specifies the frequency
      //                      on which the connection communication
      //                      will occur.  This frequency is calculated
      //                      as (ucConnectRadioFrequency_ * 1 MHz +
      //                      2400 MHz).  If ucConnectRadioFrequency_
      //                      is set to ANTFS_AUTO_FREQUENCY_SELECTION
      //                      (0xFF), then the library will use an
      //                      adaptive frequency-hopping scheme.
      //    usRadioChannelID_: This specifies the channel ID for the
      //                      the ANT channel search. Set to zero to
      //                      wildcard.
      //    bUseRequestPage_: Specifies whether the search will include
      //                      ANT-FS broadcast devices, using a request
      //                      page to begin an ANT-FS session
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once this function is called successfully, the application
      // must wait for the response from the ANTFSHost object.
      // Possible responses are:
      //    ANTFS_HOST_RESPONSE_CONNECT_PASS
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      // The library will continue to search for devices until a device
      // is found, the Cancel() function is called, an error occurs, or
      // the library is closed.
      /////////////////////////////////////////////////////////////////
      
      ANTFS_RETURN Authenticate(UCHAR ucAuthenticationType_, UCHAR *pucAuthenticationString_, UCHAR ucLength_, UCHAR *aucResponseBuffer_, UCHAR *pucResponseBufferSize_, ULONG ulResponseTimeout_);
      /////////////////////////////////////////////////////////////////
      // Request to establish an authenticated session with the connected 
      // remote device.
      // Parameters:
      //    ucAuthenticationType_:  The type of authentication to 
      //                      execute on the remote device.
      //    *pucAuthenticationString_: A string that may be used in
      //                      conjunction with the particular
      //                      authentication type in use (e.g. a 
      //                      password or a friendly name.
      //    ucLength_:        The length of the authentication string,
      //                      including any '\n' terminator.  This
      //                      parameter may be set to zero if an
      //                      authentication string is not required.
      //    *pucResponseBuffer_: Pointer to the buffer where additional 
      //                      data from the response will be saved.  This will
      //                      include data such as passwords and friendly names.
      //                      This memory must be allocated by the application.
      //    *pucResponseBufferSize_: Pointer to UCHAR varible that contains the 
      //                      size of the buffer pointed to by pucResponseBuffer_.
      //                      After the response has be recived, the UCHAR variable       
      //                      will be set to reflect the size of the new data received
      //                      in pucResponseBuffer_.
      //    ulResponseTimeout_: Number of miliseconds to wait for a response after 
      //                      the authenticate command is sent.
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once the request is posted, the application must wait for the
      // response from the ANTFSHost object.  Possible responses are:
      //    ANTFS_HOST_RESPONSE_AUTHENTICATE_NA
      //    ANTFS_HOST_RESPONSE_AUTHENTICATE_PASS
      //    ANTFS_HOST_RESPONSE_AUTHENTICATE_REJECT
      //    ANTFS_HOST_RESPONSE_AUTHENTICATE_FAIL
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      // Upon receiving the ANTFS_HOST_RESPONSE_AUTHENTICATE_PASS, an
      // authentication string provided by the remoted device may be
      // available in the response buffer.  This depends on the 
      // authentication type used.  The transport
      // layer connection is also only established after receiving 
      // ANTFS_HOST_RESPONSE_AUTHENTICATE_PASS .
      /////////////////////////////////////////////////////////////////
      
      ANTFS_RETURN Download(USHORT usFileIndex_, ULONG ulDataOffset_, ULONG ulMaxDataLength_, ULONG ulMaxBlockSize_ = 0);
      /////////////////////////////////////////////////////////////////
      // Request a download of a file from the authenticated device.
      // Parameters:
      //    usFileIndex_:     The file number to be downloaded. Some
      //                      file numbers are reserved for special
      //                      purposes, such as the device directory
      //                      (0).  Consult the ANT_FS specification
      //                      and any docs for specific device types.
      //    ulDataOffset_:    The byte offset from where to begin
      //                      transferring the data.      
      //    ulMaxDataLength_: ULONG varible that contains the maximum
      //                      number of bytes to download.  Set to 0 if
      //                      if the host does not wish to limit the
      //                      size of the download.
      //    ulMaxBlockSize_:  Maximum number of bytes that the host
      //                      wishes to download in a single block.
      //                      Set to zero to disable.
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once the request is posted, the application must wait for the
      // response from the ANTFSHost object.  Possible responses are:
      //    ANTFS_HOST_RESPONSE_DOWNLOAD_PASS
      //    ANTFS_HOST_RESPONSE_DOWNLOAD_REJECT
      //    ANTFS_HOST_RESPONSE_DOWNLOAD_FAIL
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      // Upon receiving ANTFS_HOST_RESPONSE_DOWNLOAD_PASS the downloaed data
      // will be available in the transfer buffer.  See GetTransferData().
      /////////////////////////////////////////////////////////////////      
      
      ANTFS_RETURN Upload(USHORT usFileIndex_, ULONG ulDataOffset_, ULONG ulDataLength_, void *pvData_, BOOL bForceOffset_ = TRUE, ULONG ulMaxBlockSize_ = 0);
      /////////////////////////////////////////////////////////////////
      // Request an upload of a file to the authenticated device.
      // Parameters:
      //    usFileIndex_:     The file number to be uploaded.  Some
      //                      file numbers are reserved for special
      //                      purposes, such as the device directory
      //                      (0).  Consult the ANT_FS specification
      //                      and any docs for specific device types.
      //    ulDataOffset_:    The byte offset from where to begin
      //                      transferring the data.
      //    ulDataLength_:    The byte length of data to be uploaded
      //                      to the remote device.
      //                      Return value:
      //    *pvData_:         Pointer to the location where the data
      //                      to be uploaded is stored.
      //    bForceOffset_:    Set to TRUE (default) to enforce the 
      //                      provided byte data offset.  Set to FALSE
      //                      to continue a transfer, indicating that
      //                      the host will continue the upload at the
      //                      last data offset specified by the client
      //                      in the Upload Response.
      //    ulMaxBlockSize_:  Maximum block size that the host can send 
      //                      in a single block of data.  Set to zero
      //                      to disable
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once the request is posted, the application must wait for the
      // response from the ANTFSHost object.  Possible responses are:
      //    ANTFS_HOST_RESPONSE_UPLOAD_PASS
      //    ANTFS_HOST_RESPONSE_UPLOAD_REJECT
      //    ANTFS_HOST_RESPONSE_UPLOAD_FAIL
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      /////////////////////////////////////////////////////////////////

      ANTFS_RETURN ManualTransfer(USHORT usFileIndex_, ULONG ulDataOffset_, ULONG ulDataLength_, void *pvData_);
      /////////////////////////////////////////////////////////////////
      // Send data directly to the device without requesting first.
      // This is especially useful for communicating small pieces of
      // data to the device.  Another use is the implementation of a
      // higher-level communication protocol.  Care must be taken to
      // ensure the device can handle the amount of data being sent
      // using this method.
      //    usFileIndex_:     The file number to be uploaded.  Some
      //                      file numbers are reserved for special
      //                      purposes, such as the device directory
      //                      (0).  Consult the ANT_FS specification
      //                      and any docs for specific device types.
      //    ulDataOffset_:    The byte offset from where to begin
      //                      transferring the data.  Note that this
      //                      value will always get rounded up to the
      //                      next higher multiple of 8.  Under normal
      //                      use, this parameter should always be set
      //                      to zero, and the only time it would be
      //                      non-zero is for retrying ManualTransfer()
      //                      from a known offset.
      //    ulDataLength_:    The byte length of data to be sent to
      //                      the remote device.
      //    *pvData_:         The Pointer to a buffer where the
      //                      data to be sent is stored.      
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once the request is posted, the application must wait for the
      // response from the ANTFSHost object.  Possible responses are:
      //    ANTFS_HOST_RESPONSE_MANUAL_TRANSFER_PASS
      //    ANTFS_HOST_RESPONSE_MANUAL_TRANSFER_TRANSMIT_FAIL
      //    ANTFS_HOST_RESPONSE_MANUAL_TRANSFER_RESPONSE_FAIL
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      // Upon receiving ANTFS_HOST_RESPONSE_MANUAL_TRANSFER_PASS the downloaed data
      // will be available in the transfer buffer.  See GetTransferData().
      /////////////////////////////////////////////////////////////////

      ANTFS_RETURN EraseData(USHORT usFileIndex_);
      /////////////////////////////////////////////////////////////////
      // Request the erasure of a file on the authenticated remote
      // device.
      // Parameters:
      //    usFileIndex_:     The file number of the file to be erased.
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once the request is posted, the application must wait for the
      // response from the ANTFSHost object.  Possible responses are:
      //    ANTFS_HOST_RESPONSE_ERASE_PASS
      //    ANTFS_HOST_RESPONSE_ERASE_FAIL
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      /////////////////////////////////////////////////////////////////

      ANTFS_RETURN Disconnect(USHORT usBlackoutTime_, UCHAR ucDisconnectType_ = 0, UCHAR ucTimeDuration_ = 0, UCHAR ucAppSpecificDuration_ = 0);
      /////////////////////////////////////////////////////////////////
      // Disconnect from a remote device.  Optionally put that device
      // on a blackout list for a period of time.    The application can
      // also request the remote device to become undiscoverable for a 
      // specified time/application specific duration. 
      // Parameters:
      //    usBlackoutTime_:  The number of seconds the device ID
      //                      should remain on the blackout list. If
      //                      set to zero (0), then the device will
      //                      not be put on the blackout list.  If set
      //                      to MAX_USHORT (0xFFFF), the device will
      //                      remain on the blackout list until
      //                      explicitly removed, or until the blackout
      //                      list is reset.
      //    ucDisconnectType_: Specifies whether the client should 
      //                      return to LINK state (default) or broadcast mode.
      //                      Values from 128 - 255 can be used to specify
      //                      application specific disconnect behavior.
      //    ucTimeDuration_:  Time, in 30 seconds increments, the client
      //                      device will remain undiscoverable after
      //                      disconnect has been requested. Set to 0 to
      //                      disable.
      //    ucAppSpecificDuration_: Application specific duration the client
      //                      shall remain undiscoverable after disconnection.
      //                      This field is left to the developer, or defined
      //                      in an ANT+ Device Profile. Set to 0 to disable.
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // Once the request is posted, the application must wait for the
      // response from the ANTFSHost object.  Possible responses are:
      //    ANTFS_HOST_RESPONSE_DISCONNECT_PASS
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      // The remote device will not show up in any search results for
      // the duration specified in usBlackoutTime_.  This allows the
      // host to more easily find other devices in crowded
      // environments.  The host can also request the remote device to
      // become undiscoverable, to more easily find other devices, however
      // not all client devices might implement this feature.  Not all
      // client devices support a broadcast mode.
      // If no disconnect type is specified, the default behavior will be to
      // return to the LINK state, and the host will close the
      // channel after disconnecting.  If the disconnect type is set to 1,
      // the host will return to broadcast mode, and will keep its channel
      // open.
      /////////////////////////////////////////////////////////////////

      // TODO: Need to define a response for this?
      ANTFS_RETURN SwitchFrequency(UCHAR ucRadioFrequency_, UCHAR ucChannelPeriod_ = BEACON_PERIOD_KEEP);
      /////////////////////////////////////////////////////////////////
      // Request the connected remote device to switch to the specified
      // radio frequency and channel period
      // Parameters:
      //    ucRadioFrequency_:   This specifies the frequency
      //                      on which the connection communication
      //                      will occur.  This frequency is calculated
      //                      as (ucRadioFrequency_ * 1 MHz + 2400 MHz).
      //    ucBeaconPeriod_:  This specifies the beacon period on which
      //                      the communication will continue.  Use
      //                      BEACON_PERIOD_KEEP to only change the radio
      //                      frequency.
      // Returns ANTFS_RETURN_PASS if successful.  Otherwise, it returns 
      // ANTFS_RETURN_FAIL if the library is in the wrong state, or 
      // ANTFS_RETURN_BUSY if the library is busy with another request.
      // Operation:
      // This function can only be used in transport state, to change
      // the channel parameters. 
      // Once this function is called successfully, the application
      // must wait for the response from the ANTFSHost object.
      // Possible responses are:
      //    ANTFS_HOST_RESPONSE_CONNECTION_LOST
      //    ANTFS_HOST_RESPONSE_SERIAL_FAIL
      /////////////////////////////////////////////////////////////////

      USHORT AddSearchDevice(ANTFS_DEVICE_PARAMETERS *pstDeviceSearchMask_, ANTFS_DEVICE_PARAMETERS *pstDeviceParameters_);
      /////////////////////////////////////////////////////////////////
      // Adds a set of parameters for which to search to the internal
      // search device list.
      // Parameters:
      //    *pstDeviceSearchMask_:  A pointer to an
      //                      ANTFS_DEVICE_PARAMETERS structure.  Set a
      //                      member to zero (0) to wildcard search for
      //                      it.  Otherwise, set the bits that you want
      //                      to be matched to 1 in each member.  Members
      //                      that are integer values will be treated the
      //                      same as bit fields for the purposes for the mask.
      //    *pstDeviceParameters_:  A pointer to an
      //                      ANTFS_DEVICE_PARAMETERS structure.  Set
      //                      the member to the desired search value.
      //                      A member in this structure is ignored if
      //                      the associated member in the
      //                      *pstDeviceSearchMask_ parameter is set
      //                      to zero (0) for wildcard.
      // Returns a handle the the search device entry.  If the return
      // value is NULL, the function failed adding the device entry.
      // This means that the device list is already full.
      // Operation:
      // Note that the default search masks should normally be applied
      // to the ucStatusByte1 and ucStatusByte2 members of the
      // *pstDeviceSearchMask_ parameter.  Eg;
      //    pstMyDeviceSearchMask->ucStatusByte1 =
      //       ANTFS_STATUS1_DEFAULT_SEARCH_MASK & ucMyStatus1Criteria;
      // Setting bits outside the masks, especially reserved bits, may
      // lead to undesired behaviour.
      /////////////////////////////////////////////////////////////////
      
      void RemoveSearchDevice(USHORT usHandle_);
      /////////////////////////////////////////////////////////////////
      // Removes a device entry from the internal search list.
      // Parameters:
      //    usHandle_:        Handle to the device entry to be removed
      //                      from the list.
      /////////////////////////////////////////////////////////////////

      void ClearSearchDeviceList(void);
      /////////////////////////////////////////////////////////////////
      // Clears the internal search device list.
      /////////////////////////////////////////////////////////////////

      BOOL Blackout(ULONG ulDeviceID_, USHORT usManufacturerID_, USHORT usDeviceType_, USHORT usBlackoutTime_);
      /////////////////////////////////////////////////////////////////
      // Put the device on a blackout list for a period of time.
      // Parameters:
      //    ulDeviceID_:      The device ID of a specific device.
      //    usManufacturerID_:   The specific manufacturer ID.
      //    usDeviceType_:    The specific device type.
      //    usBlackoutTime_:  The number of seconds the device ID
      //                      should remain on the blackout list. If
      //                      set to zero (0), then the device will
      //                      not be put on the blackout list.  If set
      //                      to MAX_USHORT (0xFFFF), the device will
      //                      remain on the blackout list until
      //                      explicitly removed, or until the blackout
      //                      list is reset.
      // Returns TRUE if successful.  Otherwise, it returns FALSE.
      // A wildcard parameter (0) is not allowed in any of the device
      // ID parameters and will result in returning FALSE.
      // Operation:
      // The remote device will not show up in any search results for
      // the duration specified in usBlackoutTime_.  This allows the
      // host to more easily find other devices in crowded
      // environments.
      /////////////////////////////////////////////////////////////////
      
      BOOL RemoveBlackout(ULONG ulDeviceID_, USHORT usManufacturerID_, USHORT usDeviceType_);
      /////////////////////////////////////////////////////////////////
      // Remove the device from the blackout list.
      // Parameters:
      //    ulDeviceID_:      The device ID of a specific device.
      //    usManufacturerID_:   The specific manufacturer ID.
      //    usDeviceType_:    The specific device type.
      // Returns TRUE if successful.  Returns FALSE if the device is
      // not found in the blackout list.  A wildcard parameter (0) is
      // not allowed in any of the device ID parameters and will result
      // in returning FALSE.
      /////////////////////////////////////////////////////////////////

      void ClearBlackoutList(void);
      /////////////////////////////////////////////////////////////////
      // Clears the blackout list.
      /////////////////////////////////////////////////////////////////
      
      ANTFS_HOST_RESPONSE WaitForResponse(ULONG ulMilliseconds_);
      /////////////////////////////////////////////////////////////////
      // Wait for a response from the ANTFS host library
      // Parameters:
      //    ulMilliseconds_:  Set this value to the minimum time to
      //                      wait before returning.  If the value is
      //                      0, the function will return immediately.
      //                      If the value is DSI_THREAD_INFINITE, the
      //                      function will not time out.
      // If one or more responses are pending before the timeout
      // expires the function will return the first response that
      // occured.  If no response is pending at the time the timeout
      // expires, ANTFS_HOST_RESPONSE_NONE is returned.
      /////////////////////////////////////////////////////////////////
};


#endif // ANTFS_HOST_CHANNEL