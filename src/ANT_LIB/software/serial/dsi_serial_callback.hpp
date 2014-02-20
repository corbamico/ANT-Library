/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright © 1998-2008 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */
#if !defined(DSI_SERIAL_CALLBACK_HPP)
#define DSI_SERIAL_CALLBACK_HPP

#include "types.h"


//////////////////////////////////////////////////////////////////////////////////
// Public Class Prototypes
//////////////////////////////////////////////////////////////////////////////////

class DSISerialCallback
{
   public:
      virtual ~DSISerialCallback(){}

      virtual void ProcessByte(UCHAR ucByte_) = 0;
      /////////////////////////////////////////////////////////////////
      // Processes a single received byte.
      // Parameters:
      //    ucByte_:          The byte to process.
      /////////////////////////////////////////////////////////////////

      virtual void Error(UCHAR ucError_) = 0;
      /////////////////////////////////////////////////////////////////
      // Signals an error.
      // Parameters:
      //    ucError_:         The error that has occured.  This error
      //                      is defined by the DSISerial class
      //                      implementation.
      /////////////////////////////////////////////////////////////////
};

#endif // !defined(DSI_SERIAL_CALLBACK_HPP)

