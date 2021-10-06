# BLEAppSpark
Server and client on M5 Core 2 to talk to Spark app and Spark amp with NimBLE.

*** IOS ONLY ***

Works perfectly - a pass-thru from app to amp using BLE on a single M5Stack Core 2

Turn on, turn on amp, turn on pedal (iRig Blueboard in the case of NimBLEAppSpark3), wait for connections, turn on app on IOS and press connect.

It should show something like this on M5 Stack:

```
Name  Spark 40 BLE
Found Spark - trying to connect....
Spark ok
Available for app to connect...
App active
Client ID: 2 Address: 70:fb:b6:35:ad:00 Subscribed to notfications for 0xffc2
0xffc1: onWrite(), value:  1 FE 0 0 53 FE 17 0 0 0 0 0 0 0 0 0 F0 1 1 0 2 2F F7
Wrote to amp
Client ID: 2 Address: 70:fb:b6:35:ad:00 Unsubscribed to 0xffc2

```
The last two lines show the failure to connect because I didn't have the real Spark there!

With a lot of help from h2zero (see the github issue I created and the responses! https://github.com/h2zero/NimBLE-Arduino/issues/298)
