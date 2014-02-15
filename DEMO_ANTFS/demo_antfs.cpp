/*
 * demo_antfs.cpp
 *
 * Copyright 2014 corbamico <corbamico@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
#include "antfs_host.hpp"
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

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("demo antfs"));


class CANTFSHost
{
public:
    BOOL init();
    BOOL run();


    void display_watch_info();

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

void* CANTFSHost::ReceiveThread()
{
    BOOL bExit=FALSE;
    ANTFS_RESPONSE response;
    while(!bExit)
    {
        response = hostImp_.WaitForResponse(997);
        switch(response)
        {
        case ANTFS_RESPONSE_NONE:
            break;

        case ANTFS_RESPONSE_CONNECT_PASS:
            getFoundDeviceInfo();
            display_watch_info();
            break;

        case ANTFS_RESPONSE_OPEN_PASS:
        case ANTFS_RESPONSE_CONNECTION_LOST:
            break;
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


    log4cxx::PropertyConfigurator::configure("log4cxx.properties");


    hostImp_.SetDebug(TRUE);

    bReturn = hostImp_.Init(0,57600);
    LOG4CXX_DEBUG(logger,"ANTFS Init Return:" << (int)bReturn);

    //pthread_create(&t,NULL,ReceiveThread_helper,this);

    dsiThreadID_ = DSIThread_CreateThread(ReceiveThread_helper,this);

    sleep(10);

    hostImp_.EnablePing(TRUE);
    hostImp_.SetChannelID(FR_410_CHANNEL_DEVICE_TYPE,FR_410_CHANNEL_TRANS_TYPE);
    hostImp_.SetMessagePeriod(FR_410_MESG_PERIOD);
    hostImp_.SetProximitySearch(5);
    hostImp_.SetNetworkkey(aucNetworkkey);

    hostImp_.ClearSearchDeviceList();
    hostImp_.AddSearchDevice(&stSearchMask,&stSearchParameters);

    eReturn = hostImp_.SearchForDevice(FR_410_SEARCH_FREQ,FR_410_LINK_FREQ,0,TRUE);
    LOG4CXX_DEBUG(logger,"Search for device, Return:" << (int)eReturn);

//    eReturn = hostImp_.Authenticate(AUTH_COMMAND_PAIR);
//    eReturn = hostImp_.Authenticate(AUTH_COMMAND_PASSKEY);

    return TRUE;
}
BOOL CANTFSHost::run()
{
    int ch = getchar();
    while((ch!='q')&&(ch!='Q'))
        ch = getchar();

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
