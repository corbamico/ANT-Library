/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

#include "types.h"
#if defined(DSI_TYPES_MACINTOSH) || defined(DSI_TYPES_LINUX)


#include "dsi_thread.h"
#include "macros.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include <string.h>

//////////////////////////////////////////////////////////////////////////////////
// Private Definitions
//////////////////////////////////////////////////////////////////////////////////

typedef void *(*PTHREAD_START_ROUTINE)(void *);
void ExitHandler(int sig);


//////////////////////////////////////////////////////////////////////////////////
// Private Functions
//////////////////////////////////////////////////////////////////////////////////

void ExitHandler(int sig)
{
	pthread_exit(NULL);
}


//////////////////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_MutexInit(DSI_MUTEX *pstMutex_)
{
   pthread_mutexattr_t stAttribute;

   if (pthread_mutexattr_init(&stAttribute) != 0)
      return DSI_THREAD_EOTHER;

   if (pthread_mutexattr_settype(&stAttribute, PTHREAD_MUTEX_RECURSIVE) != 0)
      return DSI_THREAD_EOTHER;

   if (pthread_mutex_init(pstMutex_, &stAttribute) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_MutexDestroy(DSI_MUTEX *pstMutex_)
{
   if (pthread_mutex_destroy(pstMutex_) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_MutexLock(DSI_MUTEX *pstMutex_)
{
   if (pthread_mutex_lock(pstMutex_) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_MutexTryLock(DSI_MUTEX *pstMutex_)
{
   switch (pthread_mutex_trylock(pstMutex_))
   {
      case 0:
         return DSI_THREAD_ENONE;

      case EBUSY:
         return DSI_THREAD_EBUSY;

      default:
         return DSI_THREAD_EOTHER;
   }
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_MutexUnlock(DSI_MUTEX *pstMutex_)
{
   if (pthread_mutex_unlock(pstMutex_) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_CondInit(DSI_CONDITION_VAR *pstConditionVariable_)
{
   if (pthread_cond_init(pstConditionVariable_, (const pthread_condattr_t *) NULL) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_CondDestroy(DSI_CONDITION_VAR *pstConditionVariable_)
{
   if (pthread_cond_destroy(pstConditionVariable_) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_CondTimedWait(DSI_CONDITION_VAR *pstConditionVariable_, DSI_MUTEX *pstExternalMutex_, ULONG ulMilliseconds_)
{
   if (ulMilliseconds_ == DSI_THREAD_INFINITE)
   {
      if (pthread_cond_wait(pstConditionVariable_, pstExternalMutex_) == 0)
         return DSI_THREAD_ENONE;

      return DSI_THREAD_EOTHER;
   }
   //else (all options return above, so else isn't required).
   {
      // Get the current time of day.
      unsigned long long ullNanoseconds = (unsigned long long) ulMilliseconds_ * 1000000;
      struct timespec stTimeSpec;
#if defined(DSI_TYPES_MACINTOSH)
      // Mac doesn't support clock_gettime(), so we need to convert
      // from timeval to timespec.
      struct timeval stTimeValue;

      if (gettimeofday(&stTimeValue, NULL) != 0)
         return DSI_THREAD_EOTHER;

      // Now add our time..
      ullNanoseconds += stTimeValue.tv_usec * 1000 + (unsigned long long) stTimeValue.tv_sec * 1000000000;
#else
      if (clock_gettime(CLOCK_REALTIME, &stTimeSpec) != 0)
         return DSI_THREAD_EOTHER;

      // Now add our time..
      ullNanoseconds += stTimeSpec.tv_nsec + (unsigned long long) stTimeSpec.tv_sec * 1000000000;
#endif

      stTimeSpec.tv_nsec = (long) (ullNanoseconds % 1000000000);
      stTimeSpec.tv_sec = (time_t) (ullNanoseconds / 1000000000);

      switch (pthread_cond_timedwait(pstConditionVariable_, pstExternalMutex_, &stTimeSpec))
      {
         case 0:
            return DSI_THREAD_ENONE;

         case ETIMEDOUT:
            return DSI_THREAD_ETIMEDOUT;

         default:
            return DSI_THREAD_EOTHER;
      }
   }
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_CondSignal(DSI_CONDITION_VAR *pstConditionVariable_)
{
   if (pthread_cond_signal(pstConditionVariable_) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_CondBroadcast(DSI_CONDITION_VAR *pstConditionVariable_)
{
   if (pthread_cond_broadcast(pstConditionVariable_) != 0)
      return DSI_THREAD_EOTHER;

   return DSI_THREAD_ENONE;
}

///////////////////////////////////////////////////////////////////////
DSI_THREAD_ID DSIThread_CreateThread(DSI_THREAD_RETURN (*fnThreadStart_)(void *), void *pvParameter_)
{
   pthread_t iThread;

   if (
   pthread_create(
      &iThread,                                             // Store the thread ID here.
      NULL,                                                 // Use default attributes.
      (PTHREAD_START_ROUTINE) fnThreadStart_,
      pvParameter_)
      == 0)
      return iThread;

   return (DSI_THREAD_ID) NULL;
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_DestroyThread(DSI_THREAD_ID hThreadID_)
{
	struct sigaction act;
	act.sa_handler = ExitHandler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);

	UCHAR err;
	err = pthread_kill(hThreadID_, SIGTERM);
	switch(err)
	{
	case 0:
		return DSI_THREAD_ENONE;
	case ESRCH:
		return DSI_THREAD_ENOTFOUND;
	case EINVAL:
		return DSI_THREAD_EOTHER;
	default:
		return DSI_THREAD_EOTHER;
	}
}

///////////////////////////////////////////////////////////////////////
UCHAR DSIThread_ReleaseThreadID(DSI_THREAD_ID hThreadID)
{
	UCHAR err;
	err = pthread_detach(hThreadID);
	switch(err)
	{
	case 0:
		return DSI_THREAD_ENONE;
	case ESRCH:
		return DSI_THREAD_ENOTFOUND;
	case EINVAL:
		return DSI_THREAD_EINVALID;
	default:
		return DSI_THREAD_EOTHER;
	}
}

///////////////////////////////////////////////////////////////////////
DSI_THREAD_IDNUM DSIThread_GetCurrentThreadIDNum(void)
{
	return pthread_self();
}

///////////////////////////////////////////////////////////////////////
BOOL DSIThread_CompareThreads(DSI_THREAD_IDNUM hThreadIDNum1, DSI_THREAD_IDNUM hThreadIDNum2)
{
	return (pthread_equal(hThreadIDNum1, hThreadIDNum2) != 0);
}

///////////////////////////////////////////////////////////////////////
ULONG DSIThread_GetSystemTime(void)
{
   ULONG ulReturn = 0;
#if defined(DSI_TYPES_MACINTOSH)
   // Mac doesn't support clock_gettime(), so we need to convert
   // from timeval to timespec.
   struct timeval stTimeValue;

   if (gettimeofday(&stTimeValue, NULL) != 0)
      return 0;

   // Now convert our time..
   ulReturn = (stTimeValue.tv_usec / 1000) +  (stTimeValue.tv_sec * 1000);
#else
   struct timespec stTimeSpec;
   if (clock_gettime(CLOCK_REALTIME, &stTimeSpec) != 0)
      return DSI_THREAD_EOTHER;

      // Now convert our time..
   ulReturn = (stTimeSpec.tv_nsec / 1000000) + stTimeSpec.tv_sec * 1000;
#endif
   return ulReturn;
}

///////////////////////////////////////////////////////////////////////
BOOL DSIThread_GetWorkingDirectory(UCHAR* pucDirectory_, USHORT usLength_)
{
   if(pucDirectory_ == NULL)
      return FALSE;

   size_t uLength = usLength_-1;  //only usLength_-1 because we need room for the foreslash
   if(getcwd((char*)pucDirectory_, uLength) == NULL)
      return FALSE;

   SNPRINTF((char*)(&pucDirectory_[strlen((char*)pucDirectory_)]), 2, "/");
   return TRUE;
}

///////////////////////////////////////////////////////////////////////
void DSIThread_Sleep(ULONG ulMilliseconds_)
{
   usleep(ulMilliseconds_*1000);
   return;
}


#endif //defined(DSI_TYPES_MACINTOSH)
