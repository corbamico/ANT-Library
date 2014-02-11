===============================================================================
#ANT library Linux Port
===============================================================================
ANT/ANTFS library port to linux. ANT+ is the wireless technology, used by (garmin or other health monitoring devices)

Notes:
-------
Source under development,only can be compiled as empty framework, NOT working.


General
-------
From http://www.thisisant.com
ANT+ is the wireless technology that allows your monitoring devices to talk to each other. Leading brands design ANT+ into top products to ensure that you get the data you want -  when and where you want it. Fundamentally, ANT+ gives you the simplest, most expandable and most reliable user experience possible.

ANT+ stands for interoperability which means that ANT+ products from multiple brands work together. Plus, because devices are compatible, you can always add to or update your monitoring system.

Library in Linux is not officially supported by Dynastream Innovations Inc. I try to port it to Linux from mac libant source (download from thisisant.com)

 I am not sure I can upload the modified source code or not, statement from Dynastream Innovations Inc
"Openly available software posted on our downloads page comes with the Apache 2.0 license or the FIT protocol license. We’d like you to use the code, recognizing the value of maintaining interoperability."


Download ANT Library :
-------
   + git clone https://github.com/corbamico/ANT-Library


COMPILE REQUIREMENTS:
-------
Software :

   + codeblocks
<pre><code>sudo apt-get install codeblocks
</code></pre>

Dependances :
   + official libant source in mac
     - get "ANT MacOSX library Package with source code" from http://www.thisisant.com/developer/resources/downloads

   + libusb1.0
<pre><code>sudo apt-get install libusb1.0-dev
</code></pre>


Compile :
---------
   + Download official source code from http://www.thisisant.com/developer/resources/downloads
   + Copy this repo code into directoy ANT_LIB
   + Modify Code in antfsmessage.h
   <code>#define NETWORK_KEY                    {0xa8, 0xa4, 0x23, 0xb9, 0xf5, 0x5e, 0x63, 0xc1} </code>
   + Modify code in dsi_thread_posix.c
   <code>#if defined(DSI_TYPES_MACINTOSH) || defined(DSI_TYPES_LINUX)</code>
   + Compile:
     - open libant_static.cbp/libant_dylib.cbp by codeblocks
     - build it

You should autoreconf in your computer since maybe different CPU&different Linux version

More words on Garmin/ANT
---------
*
 + https://github.com/mvillalba/python-ant
 + https://github.com/braiden/python-ant-downloader/
 + https://github.com/Tigge/Garmin-Forerunner-610-Extractor
 + http://sourceforge.net/projects/frant/
gant aka garmin-ant-downloader originaly hosted at cgit.open-get.org (now dead) and every bodies clone of it https://github.com/DanAnkers/garmin-ant-downloader

**
 1. USB serial: i.e. Garmin serial/USB protocol (garmin) first documented in gpsbable
 2. USB as a mass storage device Edge 800 -- these are the easiest to work with on linux
 3. ANT - garmin protocal 310XT ect
 4. ANT-FS devices 600 910XT ect

***
forerunner 410 (USB-stick(PID=0x1008) USBm-stick(PID=0x1009) can use python-ant-downloader


--------
- GitHub : https://github.com/corbamico/ANT-Library


