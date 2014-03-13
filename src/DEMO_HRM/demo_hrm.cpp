/*
 * demo_hrm.cpp
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
#include "ant.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <glog/logging.h>


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

#define ANTPLUS_NETWORK_KEY  {0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45}
#define HRM_DEVICETYPE   0x78
#define HRM_RFFREQUENCY  0x39   //Set the RF frequency to channel 57 - 2.457GHz
#define HRM_MESSAGEPERIOD  8070    //Set the message period to 8070 counts specific for the HRM


//this class for ANT+ Slave
class CANTSlave
{
public:
    BOOL init();
    BOOL run();

    static void *mainloop_helper(void *context)
    {
        pThis = static_cast<CANTSlave*>(context);
        return pThis->mainloop();
    }
public:
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


    static BOOL hrm_init(UCHAR ucMessageId_,UCHAR ucResult_);//called by response_callback

    static inline UCHAR decode_hrm_msg(UCHAR data[8]);

private:
    static UCHAR aucChannelBuffer[MAX_CHANNEL_EVENT_SIZE];
    static UCHAR aucResponseBuffer[MAX_RESPONSE_SIZE];
    static BOOL bExit;

    static CANTSlave* pThis;
};
UCHAR CANTSlave::aucChannelBuffer[]= {};
UCHAR CANTSlave::aucResponseBuffer[]= {};
BOOL  CANTSlave::bExit=FALSE;
CANTSlave* CANTSlave::pThis = NULL;

BOOL CANTSlave::init()
{
    ANT_Init(0,57600);
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
    UCHAR aucNetKey[8] = ANTPLUS_NETWORK_KEY;

    //STEP1 ANT_SetNetworkKey
    ANT_SetNetworkKey(0, aucNetKey);
    while(!bExit)
        sleep(5);

    return NULL;
}

BOOL CANTSlave::channel_callback(UCHAR ucChannel_, UCHAR ucEvent_)
{
    UCHAR ucBPM=0;
    LOG(INFO) << "Rx Channel Event:"<<(int)ucEvent_<<",channel:"<<(int)ucChannel_;

    switch(ucEvent_)
    {
    case EVENT_RX_FLAG_ACKNOWLEDGED:
    case EVENT_RX_FLAG_BURST_PACKET:
    case EVENT_RX_FLAG_BROADCAST:
        LOG(INFO) << "Rx Channel [FLAG]:"<<(int)ucEvent_<<",channel:"<<(int)ucChannel_;
        break;

    case EVENT_RX_ACKNOWLEDGED:
    case EVENT_RX_BURST_PACKET:
    case EVENT_RX_BROADCAST:
        LOG(INFO) << "Rx Channel [DATA]:"<<(int)ucEvent_<<",channel:"<<(int)ucChannel_;
        ucBPM = decode_hrm_msg(aucChannelBuffer+MESSAGE_BUFFER_DATA2_INDEX);

        if(ucBPM)
        {
            LOG(INFO) << "BPM:" << (int)ucBPM;
        }

        break;

    case EVENT_RX_EXT_ACKNOWLEDGED:
    case EVENT_RX_EXT_BURST_PACKET:
    case EVENT_RX_EXT_BROADCAST:
        LOG(INFO) << "Rx Channel [EX DATA]:"<<(int)ucEvent_<<",channel:"<<(int)ucChannel_;
        break;
    }

    return TRUE;
}
BOOL CANTSlave::response_callback(UCHAR ucChannel_, UCHAR ucMessageId_)
{
    LOG(INFO) << "Rx Call Response:"<<(int)ucMessageId_<<",channel:"<<(int)ucChannel_;
    switch(ucMessageId_)
    {
    case MESG_RESPONSE_EVENT_ID:
    {
        hrm_init(aucResponseBuffer[MESSAGE_BUFFER_DATA2_INDEX],aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
        break;
    }
    default:
        break;
    }
    return TRUE;
}

BOOL CANTSlave::hrm_init(UCHAR ucMessageId_,UCHAR ucResult_)
{
    LOG(INFO) << "Rx Call Response(MESG_RESPONSE_EVENT_ID):"<<(int)ucMessageId_<<",Result:"<<(int)ucResult_;

    if(RESPONSE_NO_ERROR!=ucResult_)
    {
        return FALSE;
    }

    switch(ucMessageId_)
    {
        //step 1 setneworkkey
    case MESG_NETWORK_KEY_ID:
        ANT_AssignChannel(0/*channel*/, 0/*slave*/, 0/*network_num*/);
        break;

        //step2 assignchannelid
    case MESG_ASSIGN_CHANNEL_ID:
        ANT_SetChannelId(0, 0, HRM_DEVICETYPE, 0);
        break;

        //step3 ANT_SetChannelId
    case MESG_CHANNEL_ID_ID:
        ANT_SetChannelRFFreq(0, HRM_RFFREQUENCY/*USER_RADIOFREQ*/);
        break;

        //step4 ANT_SetChannelRFFreq
    case MESG_CHANNEL_RADIO_FREQ_ID:
        ANT_SetChannelPeriod(0,HRM_MESSAGEPERIOD);
        break;

        //step5 ANT_SetChannelPeriod
    case  MESG_CHANNEL_MESG_PERIOD_ID:
        ANT_OpenChannel(0);
        break;

        //step6 ANT_OpenChannel
    case MESG_OPEN_CHANNEL_ID:
        //we success do it!
        break;

    default:
        break;
    }
    return TRUE;
}

/***
*BIT 6 Heart Beat Count
*BIT 7 Computed Heart Rate
*/
UCHAR CANTSlave::decode_hrm_msg(UCHAR data[8])
{
    static UCHAR ucHeartBeatCount = 0;
    if(ucHeartBeatCount!= data[6])
    {
        ucHeartBeatCount = data[6];
        return data[7];
    }
    return 0;
}

int main(int argc,char * argv[])
{
	FLAGS_alsologtostderr = true;
	google::InitGoogleLogging(argv[0]);

    {
        CANTSlave slave;
        slave.init();
        slave.run();
    }
    return 0;
}
