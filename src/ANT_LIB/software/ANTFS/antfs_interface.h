/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2011 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */


#if !defined(ANTFS_INTERFACE_H)
#define ANTFS_INTERFACE_H

//////////////////////////////////////////////////////////////////////////////////
// Public ANT-FS Definitions
//////////////////////////////////////////////////////////////////////////////////

// String lengths
#define FRIENDLY_NAME_MAX_LENGTH       255
#define TX_PASSWORD_MAX_LENGTH         255
#define PASSWORD_MAX_LENGTH            255

#define ANTFS_AUTO_FREQUENCY_SELECTION    ((UCHAR) MAX_UCHAR)
#define ANTFS_DEFAULT_TRANSPORT_FREQ      ANTFS_AUTO_FREQUENCY_SELECTION

typedef enum
{
   ANTFS_RETURN_FAIL = 0,
   ANTFS_RETURN_PASS,
   ANTFS_RETURN_BUSY
} ANTFS_RETURN;

#endif // ANTFS_INTERFACE_H