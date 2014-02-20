/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
#if !defined(CHECKSUM_H)
#define CHECKSUM_H

#include "types.h"

//////////////////////////////////////////////////////////////////////////////////
// Public Function Prototypes
//////////////////////////////////////////////////////////////////////////////////

#define CHECKSUM_VERSION   "AQB0.009"


//////////////////////////////////////////////////////////////////////////////////
// Public Function Prototypes
//////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
   extern "C" {
#endif

UCHAR CheckSum_Calc8(const volatile void *pvDataPtr_, USHORT usSize_);

#if defined(__cplusplus)
   }
#endif

#endif // !defined(CHECKSUM_H)
