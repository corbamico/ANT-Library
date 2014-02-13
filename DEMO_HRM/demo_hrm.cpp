#include "ant.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//include log4cxx header files.
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>


#define MAX_CHANNEL_EVENT_SIZE   (MESG_MAX_SIZE_VALUE)     // Channel event buffer size, assumes worst case extended message size
#define MAX_RESPONSE_SIZE        (MESG_MAX_SIZE_VALUE)     // Protocol response buffer size

#define USER_ANTCHANNEL 0

// Indexes into message recieved from ANT
#define MESSAGE_BUFFER_DATA1_INDEX ((UCHAR) 0)
#define MESSAGE_BUFFER_DATA2_INDEX ((UCHAR) 1)
#define MESSAGE_BUFFER_DATA3_INDEX ((UCHAR) 2)
#define MESSAGE_BUFFER_DATA4_INDEX ((UCHAR) 3)
#define MESSAGE_BUFFER_DATA5_INDEX ((UCHAR) 4)
#define MESSAGE_BUFFER_DATA6_INDEX ((UCHAR) 5)
#define MESSAGE_BUFFER_DATA7_INDEX ((UCHAR) 6)
#define MESSAGE_BUFFER_DATA8_INDEX ((UCHAR) 7)
#define MESSAGE_BUFFER_DATA9_INDEX ((UCHAR) 8)
#define MESSAGE_BUFFER_DATA10_INDEX ((UCHAR) 9)
#define MESSAGE_BUFFER_DATA11_INDEX ((UCHAR) 10)
#define MESSAGE_BUFFER_DATA12_INDEX ((UCHAR) 11)
#define MESSAGE_BUFFER_DATA13_INDEX ((UCHAR) 12)
#define MESSAGE_BUFFER_DATA14_INDEX ((UCHAR) 13)

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("demo"));



//this class for ANT+ Slave
class CANTSlave
{
public:
    BOOL init();
    BOOL run();

    static void *mainloop_helper(void *context)
    {
        return ((CANTSlave*)context)->mainloop();
    }
public:
    CANTSlave(){
        ANT_Init(0,57600);
    }
    ~CANTSlave()
    {
        ANT_UnassignAllResponseFunctions();
        ANT_Nap(2000);
        ANT_Close();
    }

private:
    void* mainloop(void);
    static BOOL channel_callback(UCHAR ucChannel_, UCHAR ucEvent_);
    static BOOL response_callback(UCHAR ucChannel_, UCHAR ucMessageId_);

private:
    static UCHAR aucChannelBuffer[MAX_CHANNEL_EVENT_SIZE];
    static UCHAR aucResponseBuffer[MAX_RESPONSE_SIZE];
};
UCHAR CANTSlave::aucChannelBuffer[]={};
UCHAR CANTSlave::aucResponseBuffer[]={};

BOOL CANTSlave::init()
{

    ANT_AssignResponseFunction(CANTSlave::response_callback, CANTSlave::aucResponseBuffer);
    ANT_AssignChannelEventFunction(USER_ANTCHANNEL,CANTSlave::channel_callback, CANTSlave::aucChannelBuffer);
    ANT_ResetSystem();
    ANT_Nap(2000);
    return TRUE;
}
BOOL CANTSlave::run()
{
    pthread_t t;
    void *    pthread_return;

    pthread_create(&t,NULL,&CANTSlave::mainloop_helper,this);
    pthread_join(t,&pthread_return);

    return TRUE;
}
void* CANTSlave::mainloop(void)
{
    ///TODO
    ///ANT_xxxxx
    ///ANT_xxxxx
    UCHAR aucNetKey[8] = {0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45};
    ANT_SetNetworkKey(0, aucNetKey);
    sleep(10);
    return NULL;
}

BOOL CANTSlave::channel_callback(UCHAR ucChannel_, UCHAR ucEvent_)
{
    LOG4CXX_DEBUG(logger,"Rx Channel Event:"<<ucEvent_<<",channel:"<<ucChannel_);
    return TRUE;
}
BOOL CANTSlave::response_callback(UCHAR ucChannel_, UCHAR ucMessageId_)
{
    LOG4CXX_DEBUG(logger,"Rx Call Response:"<<(int)ucMessageId_<<",channel:"<<(int)ucChannel_);
    return TRUE;
}

int main(void)
{
    log4cxx::PropertyConfigurator::configure("./log4cxx.properties");
    {
        CANTSlave slave;
        slave.init();
        slave.run();
    }
    return 0;
}
