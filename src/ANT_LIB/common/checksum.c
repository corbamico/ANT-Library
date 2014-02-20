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
#include "checksum.h"


//////////////////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
UCHAR CheckSum_Calc8(const volatile void *pvDataPtr_, USHORT usSize_)
{
   const UCHAR *pucDataPtr = (UCHAR *)pvDataPtr_;
   UCHAR ucCheckSum = 0;
   int i;
   
   // Calculate the CheckSum value (XOR of all bytes in the buffer).
   for (i = 0; i < usSize_; i++)
      ucCheckSum ^= pucDataPtr[i];
   
   return ucCheckSum;
}
