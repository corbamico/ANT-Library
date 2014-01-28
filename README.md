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

   + libtool  
   + automake
   
Dependances :
   + official libant source in mac
     - get "ANT MacOSX library Package with source code" from http://www.thisisant.com/developer/resources/downloads

   + libusb1.0
<pre><code>sudo apt-get install libusb1.0-dev
</code></pre>


Compile :
---------
   + Download official source code from http://www.thisisant.com/developer/resources/downloads
   + Modify according to ChangeLog&Copy this github source code to official source code directory tree
     - configure.ac file should be an same level as "ANT_LIB" "Demo_LIB"
   + Compile:
<pre>
<code>
$autoreconf -i -m
</code>
or you can make debug version
<code>
$autoreconf -i
$mkdir debug && cd debug && ../configure CPPFLAGS=-DDEBUG_FILE -DSW_VER=\"0.0.1\" && make
</code>
</pre>
I generate Makefile&Compile source code in 
<code>
Linux 3.0.36+ armv7l GNU/Linux
</code>
  
You should autoreconf in your computer since maybe different CPU&different Linux version

--------
- GitHub : https://github.com/corbamico/ANT-Library


