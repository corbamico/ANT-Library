/*
 * demo_antfs.h
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

#ifndef DEMO_ANTFS_H_INCLUDED
#define DEMO_ANTFS_H_INCLUDED

// FR-50 Specific
#define FR_50_CHANNEL_DEVICE_ID     ((USHORT) 0)      // Wild card
#define FR_50_CHANNEL_DEVICE_TYPE   ((UCHAR) 1)       // Watch
#define FR_50_CHANNEL_TRANS_TYPE    ((UCHAR) 0)       // Wild card
#define FR_50_SEARCH_FREQ           ((UCHAR) 50)      // ANT-FS default
#define FR_50_LINK_FREQ             ((UCHAR) 255)     // Random
#define FR_50_MESG_PERIOD           ((USHORT) 4096)   // 8Hz
#define FR_50_CLIENT_MFG_ID         ((USHORT) 1)      // Garmin
#define FR_50_CLIENT_DEVICE_TYPE    ((USHORT) 782)    // FR-50

// FR-60 Specific
#define FR_60_CHANNEL_DEVICE_ID     ((USHORT) 0)      // Wild card
#define FR_60_CHANNEL_DEVICE_TYPE   ((UCHAR) 1)       // Watch
#define FR_60_CHANNEL_TRANS_TYPE    ((UCHAR) 0)       // Wild card
#define FR_60_SEARCH_FREQ           ((UCHAR) 50)      // ANT-FS default
#define FR_60_LINK_FREQ             ((UCHAR) 255)     // Random
#define FR_60_MESG_PERIOD           ((USHORT) 4096)   // 8Hz
#define FR_60_CLIENT_MFG_ID         ((USHORT) 1)      // Garmin
#define FR_60_CLIENT_DEVICE_TYPE    ((USHORT) 988)    // FR-60

// FR-310XT Specific
#define FR_310_CHANNEL_DEVICE_ID     ((USHORT) 0)      // Wild card
#define FR_310_CHANNEL_DEVICE_TYPE   ((UCHAR) 1)       // Watch
#define FR_310_CHANNEL_TRANS_TYPE    ((UCHAR) 0)       // Wild card
#define FR_310_SEARCH_FREQ           ((UCHAR) 50)      // ANT-FS default
#define FR_310_LINK_FREQ             ((UCHAR) 255)       // Random
#define FR_310_MESG_PERIOD           ((USHORT) 4096)   // 8Hz
#define FR_310_CLIENT_MFG_ID         ((USHORT) 1)      // Garmin
#define FR_310_CLIENT_DEVICE_TYPE    ((USHORT) 1018)   // FR-310XT

// FR-410 Specific
#define FR_410_CHANNEL_DEVICE_ID     ((USHORT) 0)      // Wild card
#define FR_410_CHANNEL_DEVICE_TYPE   ((UCHAR) 1)       // Watch
#define FR_410_CHANNEL_TRANS_TYPE    ((UCHAR) 0)       // Wild card
#define FR_410_SEARCH_FREQ           ((UCHAR) 50)      // ANT-FS default
#define FR_410_LINK_FREQ             ((UCHAR) 255)       // Random
#define FR_410_MESG_PERIOD           ((USHORT) 4096)   // 8Hz
#define FR_410_CLIENT_MFG_ID         ((USHORT) 1)      // Garmin
#define FR_410_CLIENT_DEVICE_TYPE    ((USHORT) 0)   // Wild card

#endif // DEMO_ANTFS_H_INCLUDED
