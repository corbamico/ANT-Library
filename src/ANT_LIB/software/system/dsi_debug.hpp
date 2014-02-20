/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2007 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#ifndef DSIDEBUG_H_
#define DSIDEBUG_H_

// Debug configuration
//#define DEBUG_FILE
//#define DEBUG_VERBOSE                                     // Creates big debug file!
//#define DEBUG_RX


#if defined(DEBUG_FILE)


#include "types.h"

#define DSI_DEBUG_MAX_STRLEN        ((USHORT)1088) //Max size of ANT message is 255, each byte is 4 printed chars, plus 68 additional room for timestamps and header label (a larger number probably indicates garbage in serialWrite())

class DSIDebug
{
 public:
   static BOOL Init();
   static void Close();

   static BOOL ThreadInit(const char* pucName_);
   static BOOL ThreadWrite(const char* pcMessage_);
   static BOOL ThreadEnable(BOOL bEnable_);

   static BOOL SerialWrite(UCHAR ucPortNum_, const char* pcHeader_, UCHAR* pucData_, USHORT usSize_);
   static BOOL SerialEnable(UCHAR ucPortNum_, BOOL bEnable_);

   static BOOL ResetTime();
   static BOOL SetDirectory(const char* pcDirectory_ = "");
   static void SetDebug(BOOL bDebugOn_);

 private:
   static BOOL bInitialized;
};


#endif /* DEBUG_FILE */

#endif /* DSIDEBUG_H_ */
