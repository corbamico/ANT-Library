/*
 * demo_antfs.cpp
 *
 * Copyright 2014 corbamico <corbamico@163.com>
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "antfs_host.hpp"
#include "antfs_directory.h"
#include "demo_antfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//include log4cxx header files.
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("demo antfs"));


class CANTFSHost
{
public:
    BOOL init();
    BOOL run();


    void display_watch_info();
    void display_product_info(ULONG ulLength,UCHAR *pData);

private:


    void* ReceiveThread();
    static void * ReceiveThread_helper(void* context)
    {
        CANTFSHost* pHost = (CANTFSHost*)context;
        return pHost->ReceiveThread();
    }
    BOOL getFoundDeviceInfo();

private:
    static UCHAR aucNetworkkey[8];
    ANTFSHost hostImp_;

    //GET FROM if response_connect_pass
    ANTFS_DEVICE_PARAMETERS  stDeviceParameters_;
    UCHAR                    aucFriendlyName_[FRIENDLY_NAME_MAX_LENGTH];
    USHORT usDeviceNumber_;
    UCHAR  ucDeviceType_;
    UCHAR  ucTransmitType_;

    //
    DSI_THREAD_ID  dsiThreadID_;


};

static const char* const szANTFS_RESPONSE[] =
{
    "ANTFS_RESPONSE_NONE",
    "ANTFS_RESPONSE_OPEN_PASS",
    "ANTFS_RESPONSE_SERIAL_FAIL",
    "ANTFS_RESPONSE_REQUEST_SESSION_FAIL",
    "ANTFS_RESPONSE_CONNECT_PASS",
    "ANTFS_RESPONSE_DISCONNECT_PASS",
    "ANTFS_RESPONSE_CONNECTION_LOST",
    "ANTFS_RESPONSE_AUTHENTICATE_NA",
    "ANTFS_RESPONSE_AUTHENTICATE_PASS",
    "ANTFS_RESPONSE_AUTHENTICATE_REJECT",
    "ANTFS_RESPONSE_AUTHENTICATE_FAIL",
    "ANTFS_RESPONSE_DOWNLOAD_PASS",
    "ANTFS_RESPONSE_DOWNLOAD_REJECT",
    "ANTFS_RESPONSE_DOWNLOAD_INVALID_INDEX",
    "ANTFS_RESPONSE_DOWNLOAD_FILE_NOT_READABLE",
    "ANTFS_RESPONSE_DOWNLOAD_NOT_READY",
    "ANTFS_RESPONSE_DOWNLOAD_FAIL",
    "ANTFS_RESPONSE_UPLOAD_PASS",
    "ANTFS_RESPONSE_UPLOAD_REJECT",
    "ANTFS_RESPONSE_UPLOAD_INVALID_INDEX",
    "ANTFS_RESPONSE_UPLOAD_FILE_NOT_WRITEABLE",
    "ANTFS_RESPONSE_UPLOAD_INSUFFICIENT_SPACE",
    "ANTFS_RESPONSE_UPLOAD_FAIL",
    "ANTFS_RESPONSE_ERASE_PASS",
    "ANTFS_RESPONSE_ERASE_FAIL",
    "ANTFS_RESPONSE_MANUAL_TRANSFER_PASS",
    "ANTFS_RESPONSE_MANUAL_TRANSFER_TRANSMIT_FAIL",
    "ANTFS_RESPONSE_MANUAL_TRANSFER_RESPONSE_FAIL",
    "ANTFS_RESPONSE_CANCEL_DONE"
};

static const char* const szANTFS_CLIENT_DEVICE_STATUS[] =
{
    "Link State",
    "Authentication State",
    "Transport State",
    "Busy State"
};

BOOL  CANTFSHost::getFoundDeviceInfo()
{
    UCHAR ucFriendlyNameLength=FRIENDLY_NAME_MAX_LENGTH-1;
    aucFriendlyName_[0] = '\0';

    hostImp_.GetFoundDeviceParameters(&stDeviceParameters_, aucFriendlyName_, &ucFriendlyNameLength);
    hostImp_.GetFoundDeviceChannelID(&usDeviceNumber_,&ucDeviceType_,&ucTransmitType_);

    aucFriendlyName_[ucFriendlyNameLength] = '\0';

    return TRUE;
}

void CANTFSHost::display_watch_info()
{
    LOG4CXX_INFO(logger,"Found Device: " << aucFriendlyName_);
    LOG4CXX_INFO(logger,"   ANT Device Number: " << (int)usDeviceNumber_);
    LOG4CXX_INFO(logger,"   Device ID:  "        << (unsigned int)(stDeviceParameters_.ulDeviceID));
    LOG4CXX_INFO(logger,"   Manufacturer ID:  "  << int(stDeviceParameters_.usManufacturerID));
    LOG4CXX_INFO(logger,"   Device Type: "       << int(stDeviceParameters_.usDeviceType));
    LOG4CXX_INFO(logger,"   Authentication Type: " << int(stDeviceParameters_.ucAuthenticationType));
    LOG4CXX_INFO(logger,"     Has New Data: " << int(stDeviceParameters_.ucStatusByte1 & ANTFS_STATUS1_DATA_AVAILABLE_BIT ));
    LOG4CXX_INFO(logger,"     Has Paring: " << int(stDeviceParameters_.ucStatusByte1 & ANTFS_STATUS1_PAIRING_ENABLED_BIT ));
    LOG4CXX_INFO(logger,"     Has Upload Enable: " << int(stDeviceParameters_.ucStatusByte1 & ANTFS_STATUS1_UPLOAD_ENABLED_BIT ));
    LOG4CXX_INFO(logger,"     Beacon Channel Period: " << int(stDeviceParameters_.ucStatusByte1 & 0x07 ) << "Hz");
    LOG4CXX_INFO(logger,"     Client status: (" << int(stDeviceParameters_.ucStatusByte2 & 0x03 ) << ")" << szANTFS_CLIENT_DEVICE_STATUS[(stDeviceParameters_.ucStatusByte2 & 0x03 )]);
}

typedef struct
{
    unsigned  product_id;
    unsigned  software_version;
    char      product_description[];
}Product_Data_Type;

void CANTFSHost::display_product_info(ULONG ulLength,UCHAR *pData)
{
    Product_Data_Type* pProduct = (Product_Data_Type*)pData;
    LOG4CXX_INFO(logger,"Garmin Product Device: " << aucFriendlyName_);
    LOG4CXX_INFO(logger,"   ID: " << (int)pProduct->product_id);
    LOG4CXX_INFO(logger,"   software_version:  "        << (unsigned int)(pProduct->software_version));
    LOG4CXX_INFO(logger,"   product_description:  "     << (pProduct->product_description));
    LOG4CXX_INFO(logger,"   last word is:  "     << (char *)&pData[ulLength-2]);


}


static const char* const szANTFS_RETURN[] =
{
    "ANTFS_RETURN_FAIL",
    "ANTFS_RETURN_PASS",
    "ANTFS_RETURN_BUSY"
};
void* CANTFSHost::ReceiveThread()
{
    BOOL bExit=FALSE;
    BOOL bReturn;
    ANTFS_RESPONSE response;
    ULONG ulLength,ulNumberOfEntries;
    UCHAR ucBuffer[1024*100];
    ANTFS_DIRECTORY_HEADER* stHeader;

    UCHAR szFriendHostName[]="g8picuntu";
    UCHAR szResponseKey[255];
    UCHAR ucResponseKeySize = 254;

    static UCHAR szPassKey[] = {0x20,0x4f,0x61,0xa4,0x9d,0x2a,0xf9,0x9b};

    static UCHAR szPid_Product_Rqst[]={254};




    ANTFS_DEVICE_PARAMETERS stSearchMask;
    ANTFS_DEVICE_PARAMETERS stSearchParameters;

    //Get search device parameters
    ULONG ulDeviceID = 0;
    USHORT usManufacturerID = 0;
    USHORT usDeviceType = 1;

    //Search Mask
    stSearchMask.ulDeviceID = ulDeviceID ? 0xFFFFFFFF : 0;
    stSearchMask.usManufacturerID = usManufacturerID ? 0xFFFF : 0;
    stSearchMask.usDeviceType = usDeviceType ? 0xFFFF : 0;
    stSearchMask.ucAuthenticationType = 0;
    stSearchMask.ucStatusByte1 = 0;//ANTFS_STATUS1_DEFAULT_SEARCH_MASK;
    stSearchMask.ucStatusByte2 = 0;//ANTFS_STATUS2_DEFAULT_SEARCH_MASK;

    //Search Parameters
    stSearchParameters.ulDeviceID = ulDeviceID;
    stSearchParameters.usManufacturerID = usManufacturerID;
    stSearchParameters.usDeviceType = usDeviceType;
    stSearchParameters.ucAuthenticationType = 0;
    stSearchParameters.ucStatusByte1 = 0;
    stSearchParameters.ucStatusByte2 = 0;

    ULONG ulSpace;

    ANTFS_RETURN eReturn;

    while(!bExit)
    {
        response = hostImp_.WaitForResponse(997);
        switch(response)
        {
        case ANTFS_RESPONSE_NONE:
            break;

            //open success,then go to SearchForDevice
        case ANTFS_RESPONSE_OPEN_PASS:
            //hostImp_.EnablePing(TRUE);
            hostImp_.SetChannelID(FR_410_CHANNEL_DEVICE_TYPE,FR_410_CHANNEL_TRANS_TYPE);
            hostImp_.SetMessagePeriod(FR_410_MESG_PERIOD);
            hostImp_.SetProximitySearch(5);
            hostImp_.SetNetworkkey(aucNetworkkey);

            hostImp_.ClearSearchDeviceList();
            hostImp_.AddSearchDevice(&stSearchMask,&stSearchParameters);

            eReturn = hostImp_.SearchForDevice(FR_410_SEARCH_FREQ,FR_410_LINK_FREQ,0,TRUE);
            LOG4CXX_DEBUG(logger,"Search for device, Return:" << szANTFS_RETURN[eReturn]);
            break;

            //connected,then get info,and authenticate
        case ANTFS_RESPONSE_CONNECT_PASS:
            getFoundDeviceInfo();
            display_watch_info();

            //go to auth AUTH_COMMAND_PAIR
            eReturn = hostImp_.Authenticate(AUTH_COMMAND_PAIR,szFriendHostName,sizeof(szFriendHostName),szResponseKey,&ucResponseKeySize,9999);
            LOG4CXX_DEBUG(logger,"ANTFS Authenticate(AUTH_COMMAND_PAIR) Return:" << szANTFS_RETURN[eReturn]);


//          if (eReturn==ANTFS_RETURN_FAIL)
            {
                eReturn = hostImp_.Authenticate(AUTH_COMMAND_PASSKEY,szResponseKey,ucResponseKeySize,NULL,NULL,9999);
                //eReturn = hostImp_.Authenticate(AUTH_COMMAND_PASSKEY,szPassKey,sizeof(szPassKey),NULL,NULL,9999);
                LOG4CXX_DEBUG(logger,"ANTFS Authenticate(AUTH_COMMAND_PASSKEY) Return:" << szANTFS_RETURN[eReturn]);
            }
            break;

        case ANTFS_RESPONSE_AUTHENTICATE_NA:
        case ANTFS_RESPONSE_AUTHENTICATE_REJECT:
        case ANTFS_RESPONSE_AUTHENTICATE_FAIL:

            LOG4CXX_DEBUG(logger,"ANTFS Response Authenticate:(" << (int)response <<")"<< szANTFS_RESPONSE[response]);
            break;

            //Authenticate success,then go to download
        case ANTFS_RESPONSE_AUTHENTICATE_PASS:
            eReturn = hostImp_.ManualTransfer(0xffff,0,1,szPid_Product_Rqst);
            LOG4CXX_DEBUG(logger,"ANTFS Function ManualTransfer Return:" << szANTFS_RETURN[eReturn]);

            break;
        case ANTFS_RESPONSE_MANUAL_TRANSFER_PASS:
            LOG4CXX_DEBUG(logger,"ANTFS Response:(" << (int)response <<")"<< szANTFS_RESPONSE[response]);

            bReturn = hostImp_.GetTransferData(&ulLength,ucBuffer);
            display_product_info(ulLength,ucBuffer);

            bReturn = hostImp_.GetTransferData(&ulLength,ucBuffer);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Return:" << (int)bReturn);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Length:" << ulLength);

            bReturn = hostImp_.GetTransferData(&ulLength,ucBuffer);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Return:" << (int)bReturn);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Length:" << ulLength);

            bReturn = hostImp_.GetTransferData(&ulLength,ucBuffer);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Return:" << (int)bReturn);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Length:" << ulLength);

            bReturn = hostImp_.GetTransferData(&ulLength,ucBuffer);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Return:" << (int)bReturn);
            LOG4CXX_DEBUG(logger,"ANTFS GetTransferData Length:" << ulLength);



            break;
        case ANTFS_RESPONSE_DOWNLOAD_PASS:
            hostImp_.GetTransferData(&ulLength);
            hostImp_.GetTransferData(&ulLength,ucBuffer);
            stHeader = (ANTFS_DIRECTORY_HEADER*)ucBuffer;
            ulNumberOfEntries = (ulLength - sizeof(ANTFS_DIRECTORY_HEADER))/sizeof(ANTFSP_DIRECTORY);


            LOG4CXX_DEBUG(logger,"ANTFS Directory Version: " << (int)(stHeader->ucVersion) );
            LOG4CXX_DEBUG(logger,"ANTFS Directory Number Entry: " << (int)(ulNumberOfEntries) );

            break;

        case ANTFS_RESPONSE_SERIAL_FAIL:
        case ANTFS_RESPONSE_CONNECTION_LOST:
            hostImp_.Disconnect(5);
            bExit = TRUE;

        default:
            LOG4CXX_DEBUG(logger,"ANTFS Response:(" << (int)response <<")"<< szANTFS_RESPONSE[response]);
        }
    }
    return NULL;
}

//ANTFS Network Key
UCHAR CANTFSHost::aucNetworkkey[]= {0xA8, 0xA4, 0x23, 0xB9, 0xF5, 0x5E, 0x63, 0xC1};

BOOL CANTFSHost::init()
{

    BOOL bReturn;
    ANTFS_RETURN eReturn;

    log4cxx::PropertyConfigurator::configure("log4cxx.properties");

#ifdef DEBUG_FILE
    hostImp_.SetDebug(TRUE);
#endif

    bReturn = hostImp_.Init(0,57600);
    LOG4CXX_DEBUG(logger,"ANTFS Init Return:" << (int)bReturn);

    dsiThreadID_ = DSIThread_CreateThread(ReceiveThread_helper,this);

    return TRUE;
}
BOOL CANTFSHost::run()
{
    int ch = getchar();
    while((ch!='q')&&(ch!='Q'))
        ch = getchar();

    hostImp_.Close();
    DSIThread_DestroyThread(dsiThreadID_);
    return TRUE;
}

int main(void)
{

    {
        CANTFSHost host;
        host.init();
        host.run();
    }
    return 0;
}
