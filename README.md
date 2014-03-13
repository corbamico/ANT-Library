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

   + cmake
<pre><code>sudo apt-get install cmake
</code></pre>

Dependances :
   + official libant source in mac
     - get "ANT MacOSX library Package with source code" from http://www.thisisant.com/developer/resources/downloads

   + libusb1.0
<pre><code>sudo apt-get install libusb1.0-dev</code></pre>


Compile :
---------
   + Compile:
     - cd build
     - make

DEMO
---------
   + DEMO_HRM(console program) act as ANT+ Slave which recieve data from Heart Rate Monitor strap (Garmin Heart Rate Monitor Soft Belt)
it's working,bpm show in command line:
<pre>
<code>
LOG4CXX_INFO(logger,"BPM:" << (int)ucBPM);
</code>
</pre>

   + DEMO_ANTFS(console program) act as ANT+ Slave which recieve data from Garmin Forerunner 410 as showing information as following:
<pre><code>
[ 12:03:31] [0x465ff420] INFO  demo antfs - Found Device: Forerunner 410 China
[ 12:03:31] [0x465ff420] INFO  demo antfs -    ANT Device Number: 
[ 12:03:31] [0x465ff420] INFO  demo antfs -    Device ID:  
[ 12:03:31] [0x465ff420] INFO  demo antfs -    Manufacturer ID:  2
[ 12:03:31] [0x465ff420] INFO  demo antfs -    Device Type: 1
[ 12:03:31] [0x465ff420] INFO  demo antfs -    Authentication Type: 3
[ 12:03:31] [0x465ff420] INFO  demo antfs -      Has New Data: 0
[ 12:03:31] [0x465ff420] INFO  demo antfs -      Has Paring: 0
[ 12:03:31] [0x465ff420] INFO  demo antfs -      Has Upload Enable: 0
[ 12:03:31] [0x465ff420] INFO  demo antfs -      Beacon Channel Period: 1Hz
[ 12:03:31] [0x465ff420] INFO  demo antfs -      Client status: (0)Link State
</code></pre>


Known BUG
--------
   + libusb_ref after libusb_open antfs device avoid double free occur in ANT_Close.


More words on Garmin/ANT
---------
**
 + https://github.com/mvillalba/python-ant
 
 + https://github.com/braiden/python-ant-downloader/        
   (ANTFS + Garmin Protocal: works for my fr410)
 
 + https://github.com/Tigge/Garmin-Forerunner-610-Extractor 
   (extract fit file via ANTFS,works for fr60/fr70/405cx/310xt/610/910xt/Swim)
 
 + http://sourceforge.net/projects/frant/
gant aka garmin-ant-downloader originaly hosted at cgit.open-get.org (now dead) and every bodies clone of it https://github.com/DanAnkers/garmin-ant-downloader



**
 1. USB serial: i.e. Garmin serial/USB protocol (garmin) first documented in gpsbable
 2. USB as a mass storage device Edge 800 -- these are the easiest to work with on linux
 3. ANT - garmin protocal 310XT ect
 4. ANT-FS devices 600 910XT ect
 5. ANTFS + Garmin Protocal: Forerunner 410 use ANTFS for link/auth, use Garmin Protocal for application via ANTFS extention(CommandID=0x0D which is not described in <ANT File Share Technology.pdf>)
 

**
forerunner 410 (USB-stick(PID=0x1008) USBm-stick(PID=0x1009) can use python-ant-downloader

**
 1. Garmin Protocol  http://www8.garmin.com/support/commProtocol.html
 2. ANT/ANTFS Document  http://www.thisisant.com/developer/resources/downloads#documents_tab



More Words about ANTFS/Garmin Protocol for Dev
--------
 + Garmin Protocal use ManualTransfer via ANTFS by ManualTransfer(USHORT usFileIndex_=0xFFFF, ULONG ulDataOffset_, ULONG ulDataLength_, void *pvData_);
which sent (0x44,0x0D,0xFF....) to watch. ANTFS_SEND_DIRECT_ID=0x0D is undocument, but in libant src.


--------
- GitHub : https://github.com/corbamico/ANT-Library


