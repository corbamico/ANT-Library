/*
 * demo_antfs.h
 * 
 * Copyright 2014 corbamico <corbamico@163.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
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
