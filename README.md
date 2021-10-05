# BLEAppSpark
Server and client on M5 Core 2 to talk to Spark app and Spark amp with NimBLE

Currently doesn't work!



I have copied code to get a client and a server running, but in the same file they don't work - I get an abort() error.

I am scanning, connecting to a client, building the server then it crashes just as I advertise the server:
```
  pAdvertising->start(); // start advertising
```
I had to move
```
  pServer    = NimBLEDevice::createServer();
```
to the start of setup() else it failed on an ASSERT when it ran that.

```
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
```

And with debugging turned on to Debug level

```

I NimBLEScan: "Updated advertiser: 5d:0f:6c:da:e9:cc"
I NimBLEScan: "Updated advertiser: 2d:e3:41:35:be:c4"
I NimBLEScan: "Updated advertiser: 65:31:54:bf:77:b7"
I NimBLEScan: "Updated advertiser: dc:93:fb:68:4b:f1"
D NimBLEScan: "discovery complete; reason=0"
------------------------------
Name 
Name 
Name 
Name  Spark 40 BLE
D NimBLEUUID: "Comparing UUIDs; type 16 to 16; UUID 0xffc0 to 0xffc0"
Found Spark - trying to connect....D NimBLEClient: ">> connect(65:31:54:bf:77:b7)"
D NimBLEClient: "Got Client event "
I NimBLEClient: "Connected event"
D NimBLEClient: "Got Client event "
I NimBLEClient: "mtu update event; conn_handle=0 mtu=255"
I NimBLEClient: "Connection established"
D NimBLEClient: ">> deleteServices"
D NimBLEClient: "<< deleteServices"
D NimBLEClientCallbacks: "onConnect: default"
D NimBLEClient: "<< connect()"
connected
D NimBLEClient: ">> getService: uuid: 0xffc0"
D NimBLEClient: ">> retrieveServices"
D NimBLEClient: "Service Discovered >> status: 0 handle: 57"
D NimBLERemoteService: ">> NimBLERemoteService()"
D NimBLERemoteService: "<< NimBLERemoteService(): 0xffc0"
D NimBLEClient: "Service Discovered >> status: 14 handle: -1"
D NimBLEClient: "<< << Service Discovered"
D NimBLEClient: "<< retrieveServices"
D NimBLERemoteService: ">> getCharacteristic: uuid: 0xffc1"
D NimBLERemoteService: ">> retrieveCharacteristics() for service: 0xffc0"
D NimBLERemoteService: "Characteristic Discovered >> status: 0 handle: 62"
D NimBLERemoteCharacteristic: ">> NimBLERemoteCharacteristic()"
D NimBLERemoteCharacteristic: "<< NimBLERemoteCharacteristic(): 0xffc1"
D NimBLERemoteService: "Characteristic Discovered >> status: 14 handle: -1"
D NimBLERemoteService: "<< Characteristic Discovered"
D NimBLERemoteService: "<< retrieveCharacteristics()"
D NimBLERemoteService: ">> getCharacteristic: uuid: 0xffc2"
D NimBLEUUID: "Comparing UUIDs; type 16 to 16; UUID 0xffc1 to 0xffc2"
D NimBLERemoteService: ">> retrieveCharacteristics() for service: 0xffc0"
D NimBLERemoteService: "Characteristic Discovered >> status: 0 handle: 59"
D NimBLERemoteCharacteristic: ">> NimBLERemoteCharacteristic()"
D NimBLERemoteCharacteristic: "<< NimBLERemoteCharacteristic(): 0xffc2"
D NimBLERemoteService: "Characteristic Discovered >> status: 14 handle: -1"
D NimBLERemoteService: "<< Characteristic Discovered"
D NimBLERemoteService: "<< retrieveCharacteristics()"
D NimBLERemoteCharacteristic: ">> setNotify(): Characteristic: uuid: 0xffc2, handle: 59 0x003b, props:  0x12, 01"
D NimBLERemoteCharacteristic: ">> getDescriptor: uuid: 0x2902"
D NimBLERemoteCharacteristic: ">> retrieveDescriptors() for characteristic: 0xffc2"
D NimBLERemoteCharacteristic: "Descriptor Discovered >> status: 0 handle: 60"
D NimBLERemoteCharacteristic: "Descriptor Found"
D NimBLERemoteDescriptor: ">> NimBLERemoteDescriptor()"
D NimBLERemoteDescriptor: "<< NimBLERemoteDescriptor(): 0x2902"
D NimBLERemoteCharacteristic: "<< Descriptor Discovered. status: 0"
D NimBLERemoteCharacteristic: "<< setNotify()"
D NimBLERemoteDescriptor: ">> Descriptor writeValue: Descriptor: uuid: 0x2902, handle: 60"
I NimBLERemoteDescriptor: "Write complete; status=0 conn_handle=0"
D NimBLERemoteDescriptor: "<< Descriptor writeValue, rc: 0"
D NimBLEScan: ">> stop()"
D NimBLEScan: "<< stop()"
Done Spark....
D NimBLEServer: ">> createService - 0xffc0"
D NimBLEServer: "<< createService"
D NimBLEUUID: "Comparing UUIDs; type 16 to 16; UUID 0xffc1 to 0xffc2"
D NimBLEService: ">> start(): Starting service: UUID: 0xffc0, handle: 0xffff"
D NimBLEService: "Adding 2 characteristics for service UUID: 0xffc0, handle: 0xffff"
E NimBLEService: "ble_gatts_add_svcs, rc= 15, "
WAS OK TO HERE 8
D NimBLEAdvertising: ">> Advertising start: customAdvData: 0, customScanResponseData: 0"
E NimBLEServer: "ble_gatts_start; rc=15, "
abort() was called at PC 0x400debb7 on core 1

Backtrace: 0x40094198:0x3ffc6b60 0x400943c9:0x3ffc6b80 0x400debb7:0x3ffc6ba0 0x400da646:0x3ffc6bf0 0x400d295e:0x3ffc6c30 0x400f51c3:0x3ffc6cc0 0x400905b1:0x3ffc6ce0

Rebooting...
ets Jul 29 2019 12:21:46


```


