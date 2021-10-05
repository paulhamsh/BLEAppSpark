# BLEAppSpark
Server and client on M5 Core 2 to talk to Spark app and Spark amp with NimBLE

Currently doesn't work!



I have copied code to get a client and a server running, but in the same file they don't work - I get an abort() error.

I am scanning, connecting to a client, building the server then it crashes just as I advertise the server:

  pAdvertising->start(); // start advertising

I had to move

  pServer    = NimBLEDevice::createServer();

to the start of setup() else it failed on an ASSERT when it ran that.


abort() was called at PC 0x400dcad4 on core 1

Backtrace: 0x40094110:0x3ffc6b20 0x40094341:0x3ffc6b40 0x400dcad4:0x3ffc6b60 0x400d9aed:0x3ffc6bb0 0x400d228a:0x3ffc6be0 0x400f1adb:0x3ffc6c70 0x40090529:0x3ffc6c90

Rebooting...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0018,len:4
load:0x3fff001c,len:1044
load:0x40078000,len:8896
load:0x40080400,len:5816
entry 0x400806ac

