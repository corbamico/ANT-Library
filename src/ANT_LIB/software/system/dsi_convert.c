/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright � 1998-2007 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
#include "types.h"
#include "dsi_convert.h"

//////////////////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void Convert_USHORT_To_Bytes(USHORT usNum, UCHAR* ucByte1, UCHAR* ucByte0)
{
	if(ucByte0 != NULL)
		*ucByte0 = (UCHAR)(usNum);

	if(ucByte1 != NULL)
		*ucByte1 = (UCHAR)(usNum >> 8);

	return;
}

///////////////////////////////////////////////////////////////////////
USHORT Convert_Bytes_To_USHORT(UCHAR ucByte1, UCHAR ucByte0)
{
   USHORT usReturn;

	usReturn = (ucByte0)+(ucByte1 << 8);

	return usReturn;
}

///////////////////////////////////////////////////////////////////////
void Convert_ULONG_To_Bytes(ULONG ulNum, UCHAR* ucByte3, UCHAR* ucByte2, UCHAR* ucByte1, UCHAR* ucByte0)
{
	if(ucByte0 != NULL)
		*ucByte0 = (UCHAR)(ulNum);

	if(ucByte1 != NULL)
		*ucByte1 = (UCHAR)(ulNum >> 8);

	if(ucByte2 != NULL)
		*ucByte2 = (UCHAR)(ulNum >> 16);

	if(ucByte3 != NULL)
		*ucByte3 = (UCHAR)(ulNum >> 24);

	return;
}

///////////////////////////////////////////////////////////////////////
ULONG Convert_Bytes_To_ULONG(UCHAR ucByte3, UCHAR ucByte2, UCHAR ucByte1, UCHAR ucByte0)
{
   ULONG ulReturn;	
	
   ulReturn = (ucByte0)+(ucByte1 << 8)+(ucByte2 << 16)+(ucByte3 << 24);

	return ulReturn;
}