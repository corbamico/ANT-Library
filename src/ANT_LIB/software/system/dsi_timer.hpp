/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */ 

#if !defined(DSI_TIMER_HPP)
#define DSI_TIMER_HPP


#include "types.h"
#include "dsi_thread.h"


//////////////////////////////////////////////////////////////////////////////////
// Public Definitions
//////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

class DSITimer 
{
   private:
      DSI_THREAD_RETURN (*fnTimerFunc)(void *);
      void *pvTimerFuncParameter;
      BOOL bClosing;
      DSI_MUTEX stMutexCriticalSection;	  
      DSI_CONDITION_VAR stCondTimerWait;
      DSI_CONDITION_VAR stCondTimerThreadExit;
      DSI_THREAD_ID hTimerThread;
      BOOL bRecurring;
      ULONG ulInterval;

      static DSI_THREAD_RETURN TimerThreadStart(void *pvParameter_);    
      void TimerThread(void);

   public:  

      // Constuctor and Destructor      
	   DSITimer(DSI_THREAD_RETURN (*fnTimerFunc_)(void *) = (DSI_THREAD_RETURN(*)(void *))NULL, void *pvTimerFuncParameter_ = NULL, ULONG ulInterval_ = 0, BOOL bRecurring_ = FALSE);
      ~DSITimer();      

      BOOL NoError(void);
};

#endif //DSI_TIMER_HPP

