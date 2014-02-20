/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2007 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
#if !defined(DSI_CONVERT_H)
#define DSI_CONVERT_H

#include "types.h"

#if defined(__cplusplus)
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////
// Public Function Prototypes
//////////////////////////////////////////////////////////////////////////////////

//For all functions below ucByte0 is LSB
void Convert_USHORT_To_Bytes(USHORT usNum, UCHAR* ucByte1, UCHAR* ucByte0);
USHORT Convert_Bytes_To_USHORT(UCHAR ucByte1, UCHAR ucByte0);
void Convert_ULONG_To_Bytes(ULONG ulNum, UCHAR* ucByte3, UCHAR* ucByte2, UCHAR* ucByte1, UCHAR* ucByte0);
ULONG Convert_Bytes_To_ULONG(UCHAR ucByte3, UCHAR ucByte2, UCHAR ucByte1, UCHAR ucByte0);

#if defined(__cplusplus)
}
#endif

#endif // !defined(DSI_DEBUG_H)

