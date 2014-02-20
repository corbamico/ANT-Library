/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright (c) 2005 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
 /*
 **************************************************************************
 *
 * DESCRIPTION:
 *
 * Prototypes for functions in ant.c
 **************************************************************************
 */

//---------------------------------------------------------------------------

#ifndef ANT_H
#define ANT_H

#include "types.h"
#include "antdefines.h"
#include "antmessage.h"


//Port Types: these defines are used to decide what type of connection to connect over
#define PORT_TYPE_USB      0
#define PORT_TYPE_COM      1

//Framer Types: These are used to define which framing type to use
#define FRAMER_TYPE_BASIC            0

#if defined(DSI_TYPES_WINDOWS)
   #define EXPORT __declspec(dllexport)
#elif defined(DSI_TYPES_MACINTOSH) || defined(DSI_TYPES_LINUX)
   #define EXPORT __attribute__((visibility("default")))
#endif


// Application callback function pointer
typedef BOOL (*RESPONSE_FUNC)(UCHAR ucANTChannel, UCHAR ucResponseMsgID);
typedef BOOL (*CHANNEL_EVENT_FUNC)(UCHAR ucANTChannel, UCHAR ucEvent);


#ifdef __cplusplus
extern "C" {
#endif
////////////////////////////////////////////////////////////////////////////////////////
// The following functions are used to manage the USB connection to the module
////////////////////////////////////////////////////////////////////////////////////////


EXPORT BOOL ANT_GetDeviceUSBInfo(UCHAR ucUSBDeviceNum, UCHAR* pucProductString, UCHAR* pucSerialString);
EXPORT BOOL ANT_GetDeviceUSBPID(USHORT* pusPID_);
EXPORT BOOL ANT_GetDeviceUSBVID(USHORT* pusVID_);
EXPORT ULONG ANT_GetDeviceSerialNumber();

EXPORT BOOL ANT_Init(UCHAR ucUSBDeviceNum_, ULONG ulBaudrate_);  //Initializes and opens USB connection to the module
EXPORT BOOL ANT_Init_Special(UCHAR ucUSBDeviceNum_, ULONG ulBaudrate_, UCHAR ucPortType_, UCHAR ucSerialFramerType_);
EXPORT void ANT_Close();   //Closes the USB connection to the module
EXPORT const char* ANT_LibVersion(void); // Obtains the version number of the dynamic library

EXPORT void ANT_AssignResponseFunction(RESPONSE_FUNC pfResponse, UCHAR* pucResponseBuffer); // pucResponse buffer should be of size MESG_RESPONSE_EVENT_SIZE
EXPORT void ANT_AssignChannelEventFunction(UCHAR ucANTChannel,CHANNEL_EVENT_FUNC pfChannelEvent, UCHAR *pucRxBuffer);
EXPORT void ANT_UnassignAllResponseFunctions();	//Unassigns all response functions


////////////////////////////////////////////////////////////////////////////////////////
// Config Messages
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_UnAssignChannel(UCHAR ucANTChannel); // Unassign a Channel
EXPORT BOOL ANT_UnAssignChannel_RTO(UCHAR ucANTChannel, ULONG ulResponseTime_);

EXPORT BOOL ANT_AssignChannel(UCHAR ucANTChannel, UCHAR ucChanType, UCHAR ucNetNumber);
EXPORT BOOL ANT_AssignChannel_RTO(UCHAR ucANTChannel, UCHAR ucChannelType_, UCHAR ucNetNumber, ULONG ulResponseTime_);

EXPORT BOOL ANT_AssignChannelExt(UCHAR ucANTChannel, UCHAR ucChannelType_, UCHAR ucNetNumber, UCHAR ucExtFlags_);
EXPORT BOOL ANT_AssignChannelExt_RTO(UCHAR ucANTChannel, UCHAR ucChannelType_, UCHAR ucNetNumber, UCHAR ucExtFlags_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetChannelId(UCHAR ucANTChannel, USHORT usDeviceNumber, UCHAR ucDeviceType, UCHAR ucTransmissionType_);
EXPORT BOOL ANT_SetChannelId_RTO(UCHAR ucANTChannel_, USHORT usDeviceNumber_, UCHAR ucDeviceType_, UCHAR ucTransmissionType_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetChannelPeriod(UCHAR ucANTChannel, USHORT usMesgPeriod);
EXPORT BOOL ANT_SetChannelPeriod_RTO(UCHAR ucANTChannel_, USHORT usMesgPeriod_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetChannelSearchTimeout(UCHAR ucANTChannel, UCHAR ucSearchTimeout);   // Sets the search timeout for a give receive channel on the module
EXPORT BOOL ANT_SetChannelSearchTimeout_RTO(UCHAR ucANTChannel_, UCHAR ucSearchTimeout_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetChannelRFFreq(UCHAR ucANTChannel, UCHAR ucRFFreq);
EXPORT BOOL ANT_SetChannelRFFreq_RTO(UCHAR ucANTChannel_, UCHAR ucRFFreq_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetNetworkKey(UCHAR ucNetNumber, UCHAR *pucKey);
EXPORT BOOL ANT_SetNetworkKey_RTO(UCHAR ucNetNumber, UCHAR *pucKey, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetTransmitPower(UCHAR ucTransmitPower);
EXPORT BOOL ANT_SetTransmitPower_RTO(UCHAR ucTransmitPower_, ULONG ulResponseTime_);


////////////////////////////////////////////////////////////////////////////////////////
// Test Mode
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_InitCWTestMode(void);
EXPORT BOOL ANT_InitCWTestMode_RTO(ULONG ulResponseTime_);
EXPORT BOOL ANT_SetCWTestMode(UCHAR ucTransmitPower, UCHAR ucRFChannel);
EXPORT BOOL ANT_SetCWTestMode_RTO(UCHAR ucTransmitPower_, UCHAR ucRFChannel_, ULONG ulResponseTime_);

////////////////////////////////////////////////////////////////////////////////////////
// ANT Control messages
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_ResetSystem(void);

EXPORT BOOL ANT_OpenChannel(UCHAR ucANTChannel); // Opens a Channel
EXPORT BOOL ANT_OpenChannel_RTO(UCHAR ucANTChannel_, ULONG ulResponseTime_);

EXPORT BOOL ANT_CloseChannel(UCHAR ucANTChannel); // Close a channel
EXPORT BOOL ANT_CloseChannel_RTO(UCHAR ucANTChannel_, ULONG ulResponseTime_);

EXPORT BOOL ANT_RequestMessage(UCHAR ucANTChannel, UCHAR ucMessageID);
EXPORT BOOL ANT_WriteMessage(UCHAR ucMessageID, UCHAR* aucData, USHORT usMessageSize);

////////////////////////////////////////////////////////////////////////////////////////
// The following are the synchronous RF event functions used to update the synchronous data sent over a channel
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_SendBroadcastData(UCHAR ucANTChannel, UCHAR* pucData);   // Sends broadcast data to be sent on the channel's next synchronous message period
EXPORT BOOL ANT_SendAcknowledgedData(UCHAR ucANTChannel, UCHAR* pucData);  // Sends acknowledged data to be sent on the channel's next synchronous message period
EXPORT BOOL ANT_SendAcknowledgedData_RTO(UCHAR ucANTChannel_, UCHAR *pucData_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SendBurstTransferPacket(UCHAR ucANTChannelSeq, UCHAR* pucData);  // Sends acknowledged data to be sent on the channel's next synchronous message period
EXPORT BOOL ANT_SendBurstTransfer(UCHAR ucANTChannel, UCHAR *pucData, USHORT usNumDataPackets);
EXPORT BOOL ANT_SendBurstTransfer_RTO(UCHAR ucANTChannel_, UCHAR *pucData_, USHORT usNumDataPackets_, ULONG ulResponseTime_);


////////////////////////////////////////////////////////////////////////////////////////
// The following functions are used with version 2 modules
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_AddChannelID(UCHAR ucANTChannel, USHORT usDeviceNumber, UCHAR ucDeviceType, UCHAR ucTranmissionType_, UCHAR ucIndex);
EXPORT BOOL ANT_AddChannelID_RTO(UCHAR ucANTChannel_, USHORT usDeviceNumber_, UCHAR ucDeviceType_, UCHAR ucTransmissionType_, UCHAR ucListIndex_, ULONG ulResponseTime_);
EXPORT BOOL ANT_ConfigList(UCHAR ucANTChannel, UCHAR ucListSize, UCHAR ucExclude);
EXPORT BOOL ANT_ConfigList_RTO(UCHAR ucANTChannel_, UCHAR ucListSize_, UCHAR ucExclude_, ULONG ulResponseTime_);
EXPORT BOOL ANT_OpenRxScanMode();
EXPORT BOOL ANT_OpenRxScanMode_RTO(ULONG ulResponseTime_);

////////////////////////////////////////////////////////////////////////////////////////
// The following functions are used with AP2 modules (not AP1 or AT3)
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_ConfigFrequencyAgility(UCHAR ucANTChannel_, UCHAR ucFreq1_, UCHAR ucFreq2_, UCHAR ucFreq3_);
EXPORT BOOL ANT_ConfigFrequencyAgility_RTO(UCHAR ucANTChannel_, UCHAR ucFreq1_, UCHAR ucFreq2_, UCHAR ucFreq3_, ULONG ulResponseTime_);
EXPORT BOOL ANT_SetProximitySearch(UCHAR ucANTChannel_, UCHAR ucSearchThreshold_);
EXPORT BOOL ANT_SetProximitySearch_RTO(UCHAR ucANTChannel_, UCHAR ucSearchThreshold_, ULONG ulResponseTime_);
EXPORT BOOL ANT_SleepMessage();
EXPORT BOOL ANT_SleepMessage_RTO(ULONG ulResponseTime_);
EXPORT BOOL ANT_CrystalEnable();
EXPORT BOOL ANT_CrystalEnable_RTO(ULONG ulResponseTime_);

////////////////////////////////////////////////////////////////////////////////////////
// The following are NVM specific functions
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_NVM_Write(UCHAR ucSize, UCHAR *pucData);
EXPORT BOOL ANT_NVM_Write_RTO(UCHAR ucSize_, UCHAR *pucData_, ULONG ulResponseTime_);
EXPORT BOOL ANT_NVM_Clear(UCHAR ucSectNumber);
EXPORT BOOL ANT_NVM_Clear_RTO(UCHAR ucSectNumber_, ULONG ulResponseTime_);
EXPORT BOOL ANT_NVM_Dump();
EXPORT BOOL ANT_NVM_Dump_RTO(ULONG ulResponseTime_);
EXPORT BOOL ANT_NVM_SetDefaultSector(UCHAR ucSectNumber);
EXPORT BOOL ANT_NVM_SetDefaultSector_RTO(UCHAR ucSectNumber_, ULONG ulResponseTime_);
EXPORT BOOL ANT_NVM_EndSector();
EXPORT BOOL ANT_NVM_EndSector_RTO(ULONG ulResponseTime_);
EXPORT BOOL ANT_NVM_Lock();
EXPORT BOOL ANT_NVM_Lock_RTO(ULONG ulResponseTimeout_);

////////////////////////////////////////////////////////////////////////////////////////
// Rx Scan Additional Data Transmit Functions
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_SendExtBroadcastData(UCHAR ucANTChannel, UCHAR* pucData);     // Sends broadcast data to be sent on the channel's next synchronous message period
EXPORT BOOL ANT_SendExtAcknowledgedData(UCHAR ucANTChannel, UCHAR* pucData);  // Sends acknowledged data to be sent on the channel's next synchronous message period
EXPORT BOOL ANT_SendExtAcknowledgedData_RTO(UCHAR ucANTChannel_, UCHAR *pucData_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SendExtBurstTransferPacket(UCHAR ucANTChannelSeq, UCHAR* pucData);  // Sends acknowledged data to be sent on the channel's next synchronous message period
EXPORT USHORT ANT_SendExtBurstTransfer(UCHAR ucANTChannel, UCHAR *pucData, USHORT usNumDataPackets);
EXPORT USHORT ANT_SendExtBurstTransfer_RTO(UCHAR ucANTChannel_, UCHAR *pucData_, USHORT usDataPackets_, ULONG ulResponseTime_);

EXPORT BOOL ANT_RxExtMesgsEnable(UCHAR ucEnable);
EXPORT BOOL ANT_RxExtMesgsEnable_RTO(UCHAR ucEnable_, ULONG ulResponseTimeout_);
EXPORT BOOL ANT_SetLowPriorityChannelSearchTimeout(UCHAR ucANTChannel, UCHAR ucSearchTimeout);
EXPORT BOOL ANT_SetLowPriorityChannelSearchTimeout_RTO(UCHAR ucANTChannel_, UCHAR ucSearchTimeout_, ULONG ulResponseTime_);

EXPORT BOOL ANT_SetSerialNumChannelId(UCHAR ucANTChannel, UCHAR ucDeviceType, UCHAR ucTransmissionType);
EXPORT BOOL ANT_SetSerialNumChannelId_RTO(UCHAR ucANTChannel_, UCHAR ucDeviceType_, UCHAR ucTransmissionType_, ULONG ulResponseTime_);
EXPORT BOOL ANT_EnableLED(UCHAR ucEnable);
EXPORT BOOL ANT_EnableLED_RTO(UCHAR ucEnable_, ULONG ulResponseTime_);
EXPORT BOOL ANT_SetChannelTxPower(UCHAR ucANTChannel, UCHAR ucTransmitPower);
EXPORT BOOL ANT_SetChannelTxPower_RTO(UCHAR ucANTChannel_, UCHAR ucTransmitPower_, ULONG ulResponseTime_);

EXPORT BOOL ANT_RSSI_SetSearchThreshold(UCHAR ucANTChannel_, UCHAR ucThreshold_);
EXPORT BOOL ANT_RSSI_SetSearchThreshold_RTO(UCHAR ucANTChannel_, UCHAR ucThreshold_, ULONG ulResponseTime_);


////////////////////////////////////////////////////////////////////////////////////////
// The following are FIT1e specific functions
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL FIT_SetFEState(UCHAR ucFEState_);
EXPORT BOOL FIT_SetFEState_RTO(UCHAR ucFEState_, ULONG ulResponseTime_);
EXPORT BOOL FIT_AdjustPairingSettings(UCHAR ucSearchLv_, UCHAR ucPairLv_, UCHAR ucTrackLv_);
EXPORT BOOL FIT_AdjustPairingSettings_RTO(UCHAR ucSearchLv_, UCHAR ucPairLv_, UCHAR ucTrackLv_, ULONG ulResponseTime_);


////////////////////////////////////////////////////////////////////////////////////////
// The following are the Integrated ANTFS_Client functions
////////////////////////////////////////////////////////////////////////////////////////
//Memory Device Commands
EXPORT BOOL ANTFS_InitEEPROMDevice(USHORT usPageSize_, UCHAR ucAddressConfig_);

//File System Commands
EXPORT BOOL ANTFS_InitFSMemory();
EXPORT BOOL ANTFS_FormatFSMemory(USHORT usNumberOfSectors_, USHORT usPagesPerSector_);
EXPORT BOOL ANTFS_SaveDirectory();
EXPORT BOOL ANTFS_DirectoryRebuild();
EXPORT BOOL ANTFS_FileDelete(UCHAR ucFileHandle_);
EXPORT BOOL ANTFS_FileClose(UCHAR ucFileHandle_);
EXPORT BOOL ANTFS_SetFileSpecificFlags(UCHAR ucFileHandle_, UCHAR ucFlags_);
EXPORT UCHAR ANTFS_DirectoryReadLock(BOOL bLock_);
EXPORT BOOL ANTFS_SetSystemTime(ULONG ulTime_);

//File System Requests
EXPORT ULONG ANTFS_GetUsedSpace();
EXPORT ULONG ANTFS_GetFreeSpace();
EXPORT USHORT ANTFS_FindFileIndex(UCHAR ucFileDataType_, UCHAR ucFileSubType_, USHORT usFileNumber_);
EXPORT UCHAR ANTFS_ReadDirectoryAbsolute(ULONG ulOffset_, UCHAR ucSize_, UCHAR* pucBuffer_);
EXPORT UCHAR ANTFS_DirectoryReadEntry (USHORT usFileIndex_, UCHAR* ucFileDirectoryBuffer_);
EXPORT ULONG  ANTFS_DirectoryGetSize();
EXPORT USHORT ANTFS_FileCreate(USHORT usFileIndex_, UCHAR ucFileDataType_, ULONG ulFileIdentifier_, UCHAR ucFileDataTypeSpecificFlags_, UCHAR ucGeneralFlags);
EXPORT UCHAR ANTFS_FileOpen(USHORT usFileIndex_, UCHAR ucOpenFlags_);
EXPORT UCHAR ANTFS_FileReadAbsolute(UCHAR ucFileHandle_, ULONG ulOffset_, UCHAR ucReadSize_, UCHAR* pucReadBuffer_);
EXPORT UCHAR ANTFS_FileReadRelative(UCHAR ucFileHandle_, UCHAR ucReadSize_, UCHAR* pucReadBuffer_);
EXPORT UCHAR ANTFS_FileWriteAbsolute(UCHAR ucFileHandle_, ULONG ulFileOffset_, UCHAR ucWriteSize_, const UCHAR* pucWriteBuffer_, UCHAR* ucBytesWritten_);
EXPORT UCHAR ANTFS_FileWriteRelative(UCHAR ucFileHandle_, UCHAR ucWriteSize_, const UCHAR* pucWriteBuffer_, UCHAR* ucBytesWritten_);
EXPORT ULONG ANTFS_FileGetSize(UCHAR ucFileHandle_);
EXPORT ULONG ANTFS_FileGetSizeInMem(UCHAR ucFileHandle_);
EXPORT UCHAR ANTFS_FileGetSpecificFlags(UCHAR ucFileHandle_);
EXPORT ULONG ANTFS_FileGetSystemTime();

//FS-Crypto Commands
EXPORT UCHAR ANTFS_CryptoAddUserKeyIndex(UCHAR ucIndex_,  UCHAR* pucKey_);
EXPORT UCHAR ANTFS_CryptoSetUserKeyIndex(UCHAR ucIndex_);
EXPORT UCHAR ANTFS_CryptoSetUserKeyVal(UCHAR* pucKey_);

//FIT Commands
EXPORT UCHAR ANTFS_FitFileIntegrityCheck(UCHAR ucFileHandle_);

//ANT-FS Commands
EXPORT UCHAR ANTFS_OpenBeacon();
EXPORT UCHAR ANTFS_CloseBeacon();
EXPORT UCHAR ANTFS_ConfigBeacon(USHORT usDeviceType_, USHORT usManufacturer_, UCHAR ucAuthType_, UCHAR ucBeaconStatus_);
EXPORT UCHAR ANTFS_SetFriendlyName(UCHAR ucLength_, const UCHAR* pucString_);
EXPORT UCHAR ANTFS_SetPasskey(UCHAR ucLength_, const UCHAR* pucString_);
EXPORT UCHAR ANTFS_SetBeaconState(UCHAR ucBeaconStatus_);
EXPORT UCHAR ANTFS_PairResponse(BOOL bAccept_);
EXPORT UCHAR ANTFS_SetLinkFrequency(UCHAR ucChannelNumber_, UCHAR ucFrequency_);
EXPORT UCHAR ANTFS_SetBeaconTimeout(UCHAR ucTimeout_);
EXPORT UCHAR ANTFS_SetPairingTimeout(UCHAR ucTimeout_);
EXPORT UCHAR ANTFS_EnableRemoteFileCreate(BOOL bEnable_);
//ANT-FS Responses
EXPORT UCHAR ANTFS_GetCmdPipe(UCHAR ucOffset_, UCHAR ucReadSize_, UCHAR* pucReadBuffer_);
EXPORT UCHAR ANTFS_SetCmdPipe(UCHAR ucOffset_, UCHAR ucWriteSize_, const UCHAR* pucWriteBuffer_);

//GetFSResponse
EXPORT UCHAR ANTFS_GetLastError();



////////////////////////////////////////////////////////////////////////////////////////
// Threading
////////////////////////////////////////////////////////////////////////////////////////
EXPORT void ANT_Nap(ULONG ulMilliseconds_);

////////////////////////////////////////////////////////////////////////////////////////
// Logging
////////////////////////////////////////////////////////////////////////////////////////
EXPORT BOOL ANT_SetDebugLogDirectory(char* pcDirectory);

#ifdef __cplusplus
}
#endif

#endif // ANT_H
