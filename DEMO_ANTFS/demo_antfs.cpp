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
    UCHAR                    aucFriendlyName_[1024];
    USHORT usDeviceNumber_;
    UCHAR  ucDeviceType_;
    UCHAR  ucTransmitType_;


};

static char* szANTFS_RESPONSE[] = {
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

BOOL  CANTFSHost::getFoundDeviceInfo()
{
    UCHAR ucFriendlyNameLength=1023;
    aucFriendlyName_[0] = '\0';

    hostImp_.GetFoundDeviceParameters(&stDeviceParameters_, aucFriendlyName_, &ucFriendlyNameLength);
    hostImp_.GetFoundDeviceChannelID(&usDeviceNumber_,&ucDeviceType_,&ucTransmitType_);
}

void CANTFSHost::display_watch_info()
{
    LOG4CXX_INFO(logger,"Found Device: " << aucFriendlyName_);
    LOG4CXX_INFO(logger,"   ANT Device Number: " << (int)usDeviceNumber_);
    LOG4CXX_INFO(logger,"   Device ID:  "        << int(stDeviceParameters_.ulDeviceID));
    LOG4CXX_INFO(logger,"   Manufacturer ID:  "  << int(stDeviceParameters_.usManufacturerID));
    LOG4CXX_INFO(logger,"   Device Type: "       << int(stDeviceParameters_.usDeviceType));
    LOG4CXX_INFO(logger,"   Authentication Type: " << int(stDeviceParameters_.ucAuthenticationType));
    LOG4CXX_INFO(logger,"   Authentication Type: " << int(stDeviceParameters_.ucAuthenticationType));

}

void* CANTFSHost::ReceiveThread()
{
    BOOL bExit=FALSE;
    ANTFS_RESPONSE response;
    while(!bExit)
    {
        response = hostImp_.WaitForResponse(999);
        switch(response)
        {
            case ANTFS_RESPONSE_NONE:
                break;
            case ANTFS_RESPONSE_OPEN_PASS:
                break;
            case ANTFS_RESPONSE_CONNECT_PASS:
                getFoundDeviceInfo();
                display_watch_info();
                break;
            default:
                LOG4CXX_DEBUG(logger,"ANTFS Response:(" << (int)response <<")"<< szANTFS_RESPONSE[response]);
        }
    }
    return NULL;
}


UCHAR CANTFSHost::aucNetworkkey[]={0xA8, 0xA4, 0x23, 0xB9, 0xF5, 0x5E, 0x63, 0xC1};

BOOL CANTFSHost::init()
{

    BOOL bReturn;
	ANTFS_RETURN eReturn;
	ANTFS_DEVICE_PARAMETERS stSearchMask;
	ANTFS_DEVICE_PARAMETERS stSearchParameters;

	pthread_t  t;

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
    LOG4CXX_DEBUG(logger,"ANTFS Init:" << (int)bReturn);

    pthread_create(&t,NULL,ReceiveThread_helper,this);

    sleep(10);


    hostImp_.SetChannelID(FR_410_CHANNEL_DEVICE_TYPE,FR_410_CHANNEL_TRANS_TYPE);
    hostImp_.SetMessagePeriod(FR_410_MESG_PERIOD);
    hostImp_.SetProximitySearch(5);
    hostImp_.SetNetworkkey(aucNetworkkey);

    hostImp_.ClearSearchDeviceList();
    hostImp_.AddSearchDevice(&stSearchMask,&stSearchParameters);

    eReturn = hostImp_.SearchForDevice(FR_410_SEARCH_FREQ,FR_410_LINK_FREQ,0,TRUE);
    LOG4CXX_DEBUG(logger,"search for device:" << (int)eReturn);

//    eReturn = hostImp_.Authenticate(AUTH_COMMAND_PAIR);
//    eReturn = hostImp_.Authenticate(AUTH_COMMAND_PASSKEY);

    return TRUE;
}
BOOL CANTFSHost::run()
{
    int ch = getchar();
    while(ch='q')
        ch = getchar();

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




/**************
picuntu@g8picuntu:~/github/ANT-Library/project/bin$ ant-downloader -v
[MainThread]	2014-02-14 15:15:41,043	DEBUG	Executing Command. RESET_SYSTEM()
[MainThread]	2014-02-14 15:15:41,044	DEBUG	SEND: a4014a00ef
[MainThread]	2014-02-14 15:15:42,050	DEBUG	Querying ANT capabilities
[MainThread]	2014-02-14 15:15:42,051	DEBUG	Executing Command. REQUEST_MESSAGE(channel_number=0, msg_id=84)
[MainThread]	2014-02-14 15:15:42,052	DEBUG	SEND: a4024d0054bf
[Thread-1]	2014-02-14 15:15:42,053	DEBUG	RECV: a40654080300ba360071
[Thread-1]	2014-02-14 15:15:42,054	DEBUG	Processing reply. CAPABILITIES(max_channels=8, max_networks=3, standard_opts=0, advanced_opts1=186)
[MainThread]	2014-02-14 15:15:42,055	DEBUG	Device Capabilities: CAPABILITIES(max_channels=8, max_networks=3, standard_opts=0, advanced_opts1=186)
[MainThread]	2014-02-14 15:15:42,057	INFO	Searching for ANT devices.
[MainThread]	2014-02-14 15:15:42,059	DEBUG	Executing Command. RESET_SYSTEM()
[MainThread]	2014-02-14 15:15:42,060	DEBUG	SEND: a4014a00ef
[Thread-1]	2014-02-14 15:15:42,066	DEBUG	RECV: a4016f20ea
[Thread-1]	2014-02-14 15:15:42,066	DEBUG	Processing reply. STARTUP_MESSAGE(startup_message=32)
[MainThread]	2014-02-14 15:15:43,067	DEBUG	Executing Command. SET_NETWORK_KEY(network_number=0, network_key='\xa8\xa4#\xb9\xf5^c\xc1')
[MainThread]	2014-02-14 15:15:43,071	DEBUG	SEND: a4094600a8a423b9f55e63c174
[Thread-1]	2014-02-14 15:15:43,079	DEBUG	RECV: a40340004600a1
[Thread-1]	2014-02-14 15:15:43,081	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=70, msg_code=0)
[MainThread]	2014-02-14 15:15:43,087	DEBUG	Executing Command. ASSIGN_CHANNEL(channel_number=0, channel_type=0, network_number=0)
[MainThread]	2014-02-14 15:15:43,088	DEBUG	SEND: a40342000000e5
[Thread-1]	2014-02-14 15:15:43,091	DEBUG	RECV: a40340004200a5
[Thread-1]	2014-02-14 15:15:43,092	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=66, msg_code=0)
[MainThread]	2014-02-14 15:15:43,095	DEBUG	Executing Command. SET_CHANNEL_ID(channel_number=0, device_number=0, device_type_id=0, trans_type=0)
[MainThread]	2014-02-14 15:15:43,096	DEBUG	SEND: a405510000000000f0
[Thread-1]	2014-02-14 15:15:43,099	DEBUG	RECV: a40340005100b6
[Thread-1]	2014-02-14 15:15:43,100	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=81, msg_code=0)
[MainThread]	2014-02-14 15:15:43,104	DEBUG	Executing Command. SET_CHANNEL_PERIOD(channel_number=0, messaging_period=4096)
[MainThread]	2014-02-14 15:15:43,106	DEBUG	SEND: a40343000010f4
[Thread-1]	2014-02-14 15:15:43,110	DEBUG	RECV: a40340004300a4
[Thread-1]	2014-02-14 15:15:43,111	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=67, msg_code=0)
[MainThread]	2014-02-14 15:15:43,113	DEBUG	Executing Command. SET_CHANNEL_SEARCH_TIMEOUT(channel_number=0, search_timeout=255)
[MainThread]	2014-02-14 15:15:43,115	DEBUG	SEND: a4024400ff1d
[Thread-1]	2014-02-14 15:15:43,122	DEBUG	RECV: a40340004400a3
[Thread-1]	2014-02-14 15:15:43,124	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=68, msg_code=0)
[MainThread]	2014-02-14 15:15:43,130	DEBUG	Executing Command. SET_CHANNEL_RF_FREQ(channel_number=0, rf_freq=50)
[MainThread]	2014-02-14 15:15:43,132	DEBUG	SEND: a402450032d1
[Thread-1]	2014-02-14 15:15:43,136	DEBUG	RECV: a40340004500a2
[Thread-1]	2014-02-14 15:15:43,137	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=69, msg_code=0)
[MainThread]	2014-02-14 15:15:43,140	DEBUG	Executing Command. SET_SEARCH_WAVEFORM(channel_number=0, waveform=83)
[MainThread]	2014-02-14 15:15:43,142	DEBUG	SEND: a40349005300bd
[Thread-1]	2014-02-14 15:15:43,147	DEBUG	RECV: a40340004900ae
[Thread-1]	2014-02-14 15:15:43,148	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=73, msg_code=0)
[MainThread]	2014-02-14 15:15:43,150	DEBUG	Executing Command. OPEN_CHANNEL(channel_number=0)
[MainThread]	2014-02-14 15:15:43,152	DEBUG	SEND: a4014b00ee
[Thread-1]	2014-02-14 15:15:43,156	DEBUG	RECV: a40340004b00ac
[Thread-1]	2014-02-14 15:15:43,157	DEBUG	Processing reply. CHANNEL_EVENT(channel_number=0, msg_id=75, msg_code=0)
[MainThread]	2014-02-14 15:15:43,160	DEBUG	Executing Command. ReadData(channel_number=0)
[MainThread]	2014-02-14 15:15:43,161	DEBUG	SEND: a4024d0052b9
[Thread-1]	2014-02-14 15:15:43,164	DEBUG	RECV: a402520002f6
[Thread-1]	2014-02-14 15:15:43,165	DEBUG	Processing reply. CHANNEL_STATUS(channel_number=0, channel_status=2)
[Thread-1]	2014-02-14 15:15:55,419	DEBUG	RECV: a4094e004301000301000200a1
[Thread-1]	2014-02-14 15:15:55,420	DEBUG	Processing reply. RECV_BROADCAST_DATA(channel_number=0, data='C\x01\x00\x03\x01\x00\x02\x00')
[MainThread]	2014-02-14 15:15:55,430	DEBUG	Executing Command. REQUEST_MESSAGE(channel_number=0, msg_id=81)
[MainThread]	2014-02-14 15:15:55,431	DEBUG	SEND: a4024d0051ba
[Thread-1]	2014-02-14 15:15:55,433	DEBUG	RECV: a405510026700105a2
[Thread-1]	2014-02-14 15:15:55,434	DEBUG	Processing reply. CHANNEL_ID(channel_number=0, device_number=28710, device_type_id=1, man_id=5)
[MainThread]	2014-02-14 15:15:55,437	DEBUG	Got ANT-FS Beacon. device_number=0x7026 Beacon{'auth_type': 3, 'pairing_enabled': 0, 'data_available': 0, 'device_state': 0, 'period': 1, 'descriptor': 131073, 'upload_enabled': 0, 'data_page_id': 67, 'data': '', 'status_2': 0, 'status_1': 1}
[MainThread]	2014-02-14 15:15:55,437	INFO	Found device, but no data available for download.
[MainThread]	2014-02-14 15:15:55,438	DEBUG	SEND: a4094f004403000000000000a5
[MainThread]	2014-02-14 15:15:55,439	DEBUG	Executing Command. RESET_SYSTEM()
[MainThread]	2014-02-14 15:15:55,440	DEBUG	SEND: a4014a00ef
[Thread-1]	2014-02-14 15:15:55,442	DEBUG	RECV: a4016f20ea
[Thread-1]	2014-02-14 15:15:55,443	DEBUG	Processing reply. STARTUP_MESSAGE(startup_message=32)
**************/
