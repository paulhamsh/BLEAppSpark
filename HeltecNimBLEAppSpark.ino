#include "heltec.h"   
#include "NimBLEDevice.h"

#define C_SERVICE "ffc0"
#define C_CHAR1   "ffc1"
#define C_CHAR2   "ffc2"

#define S_SERVICE "ffc0"
#define S_CHAR1   "ffc1"
#define S_CHAR2   "ffc2"

#define PEDAL_SERVICE    "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define PEDAL_CHAR       "7772e5db-3868-4112-a1a9-f2669d106bf3"

NimBLEServer *pServer;
NimBLEService *pService;
NimBLECharacteristic *pCharacteristic_receive;
NimBLECharacteristic *pCharacteristic_send;

NimBLEAdvertising *pAdvertising;
  
NimBLEScan *pScan;
NimBLEScanResults pResults;
NimBLEAdvertisedDevice device;

NimBLEClient *pClient_sp;
NimBLERemoteService *pService_sp;
NimBLERemoteCharacteristic *pReceiver_sp;
NimBLERemoteCharacteristic *pSender_sp;

NimBLEClient *pClient_pedal;
NimBLERemoteService *pService_pedal;
NimBLERemoteCharacteristic *pReceiver_pedal;
NimBLERemoteCharacteristic *pSender_pedal;

const int preset_cmd_size = 26;

byte preset_cmd[] = {
  0x01, 0xFE, 0x00, 0x00,
  0x53, 0xFE, 0x1A, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0xF0, 0x01, 0x24, 0x00,
  0x01, 0x38, 0x00, 0x00,
  0x00, 0xF7
};

bool triggered_pedal;
int  curr_preset;

bool triggered_to_amp, triggered_to_app;
int  pass_size_amp,    pass_size_app;
byte pass_amp[1000],   pass_app[1000];

void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  byte b;

  Serial.print("Spark: ");
  for (i = 0; i < length; i++) {
    b = pData[i];
    Serial.print(b, HEX);
    Serial.print(" ");
    pass_app[i] = b;
  }
  Serial.println();
  pass_size_app = i;
  triggered_to_app = true;

}


void notifyCB_pedal(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  
  Serial.print("Pedal: ");
  for (i = 0; i < length; i++) {
    Serial.print(pData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // In mode B the BB gives 0x80 0x80 0x90 0xNN 0x64 or 0x80 0x80 0x80 0xNN 0x00 for on and off
  // In mode C the BB gives 0x80 0x80 0xB0 0xNN 0x7F or 0x80 0x80 0xB0 0xNN 0x00 for on and off
  
  if (pData[2] == 0x90 || (pData[2] == 0xB0 && pData[4] == 0x7F)) {
    switch (pData[3]) {
      case 0x3C:
      case 0x14:
        curr_preset = 0;
        break;
      case 0x3E:
      case 0x15:
        curr_preset = 1;
        break;
      case 0x40:
      case 0x16:
        curr_preset = 2;
        break;
      case 0x41:
      case 0x17:
        curr_preset = 3;
        break;
    }
    triggered_pedal = true;
  }
}

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        int j, l;
        const char *p;
        byte b;
        
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead, value: ");

        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          b = p[j];
          Serial.print(" ");
          Serial.print(b, HEX); 
        };
        Serial.println();
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
      
        int j, l;
        const char *p;
        byte b;
        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          b = p[j];
          Serial.print(" ");
          Serial.print(b, HEX); 
          pass_amp[j] = b;
        }
        Serial.println();
        pass_size_amp = j;
        triggered_to_amp = true;
    };
   
    
    /** Called before notification or indication is sent, 
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", "; 
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        if (subValue == 1 && strcmp(std::string(pCharacteristic->getUUID()).c_str(), "0xffc2") == 0) {
          Serial.println("App active");
        }

        Serial.println(str);
    };
};
    
/** Handler class for descriptor actions 
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
        std::string dscVal((char*)pDescriptor->getValue(), pDescriptor->getLength());
        Serial.print("Descriptor witten value:");
        Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
        Serial.print(pDescriptor->getUUID().toString().c_str());
        Serial.println(" Descriptor read");
    };
};

*/

/** Define callback instances globally to use for multiple Charateristics \ Descriptors */ 
// static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks_s, chrCallbacks_r;

bool connected_pedal, connected_sp;
int i;

void setup() {
  // put your setup code here, to run once:
  Heltec.display->init();
  Heltec.display->flipScreenVertically();

  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0, 0, "Spark BLE");
  Heltec.display->setFont(ArialMT_Plain_10);

  Heltec.display->display();
  
  Serial.begin(115200);                       // Start serial debug console monitoring via ESP32
  while (!Serial);

  triggered_to_app = false;
  triggered_to_amp = false;
  triggered_pedal = false;

  curr_preset = 0;

  // Create server to act as Spark
  NimBLEDevice::init("Spark 40 BLE");
  pClient_sp = NimBLEDevice::createClient();
  pScan      = NimBLEDevice::getScan();
    
  // Set up server
  pServer = NimBLEDevice::createServer();
  pService = pServer->createService(S_SERVICE);
  pCharacteristic_receive = pService->createCharacteristic(S_CHAR1, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristic_send = pService->createCharacteristic(S_CHAR2, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);  
  pCharacteristic_receive->setCallbacks(&chrCallbacks_r);
  pCharacteristic_send->setCallbacks(&chrCallbacks_s);

  pService->start();
  pServer->start();

  pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
  pAdvertising->addServiceUUID(pService->getUUID()); // tell advertising the UUID of our service
  pAdvertising->setScanResponse(true);  

  Serial.println("Service set up");

  
  // Connect to Spark
  connected_sp = false;
  connected_pedal = false;
  
  while (!connected_sp /* || !connected_pedal*/ ) {
    pResults = pScan->start(4);
    NimBLEUUID SpServiceUuid(C_SERVICE);
    NimBLEUUID PedalServiceUuid(PEDAL_SERVICE);

    Serial.println("------------------------------");
    for(i = 0; i < pResults.getCount()  && (!connected_sp /* || !connected_pedal */); i++) {
      device = pResults.getDevice(i);

      // Print info
      Serial.print("Name ");
      Serial.println(device.getName().c_str());

      if (device.isAdvertisingService(SpServiceUuid)) {
        Serial.println("Found Spark - trying to connect....");
        if(pClient_sp->connect(&device)) {
          connected_sp = true;
          Serial.println("Spark connected");
          Heltec.display->drawString(0, 30, "Spark connected");
          Heltec.display->display();
        }
      }

      if (strcmp(device.getName().c_str(),"iRig BlueBoard") == 0) {
        Serial.println("Found pedal by name - trying to connect....");
        pClient_pedal = NimBLEDevice::createClient();
        if(pClient_pedal->connect(&device)) {
          connected_pedal = true;
          Serial.println("Pedal connected");
          Heltec.display->drawString(0, 40, "Pedal connected");
          Heltec.display->display();
        }
      }
    }

    // Set up client
    if (connected_sp) {
      pService_sp = pClient_sp->getService(SpServiceUuid);
      if (pService_sp != nullptr) {
        pSender_sp   = pService_sp->getCharacteristic(C_CHAR1);
        pReceiver_sp = pService_sp->getCharacteristic(C_CHAR2);
        if (pReceiver_sp && pReceiver_sp->canNotify()) {
          if (!pReceiver_sp->subscribe(true, notifyCB_sp, true)) {
            connected_sp = false;
            Serial.println("Spark disconnected");
            //pClient_sp->disconnect();
            NimBLEDevice::deleteClient(pClient_sp);
          }
        }
      }
    }

    if (connected_pedal) {
      pService_pedal = pClient_pedal->getService(PedalServiceUuid);
      if (pService_pedal != nullptr) {
        pReceiver_pedal = pService_pedal->getCharacteristic(PEDAL_CHAR);
        if (pReceiver_pedal && pReceiver_pedal->canNotify()) {
          if (!pReceiver_pedal->subscribe(true, notifyCB_pedal, true)) {
            connected_pedal = false;
            pClient_pedal->disconnect();
          }
        }
      }
    }

  }

  Serial.println("Available for app to connect...");
  
  // start advertising
  pAdvertising->start(); 

}


void loop() {
  // put your main code here, to run repeatedly:

  if (triggered_to_amp) {
    triggered_to_amp = false;
    if (connected_sp) {
      pSender_sp->writeValue(pass_amp, pass_size_amp, false);
      Serial.println("Wrote to amp");
    }
  }

  if (triggered_to_app) {
    triggered_to_app = false;
    pCharacteristic_send->setValue(pass_app, pass_size_app);
    pCharacteristic_send->notify(true);
    Serial.println("Wrote to app");
  }

  if (triggered_pedal) {
    triggered_pedal = false;
    if (connected_sp) {
      preset_cmd[preset_cmd_size-2] = curr_preset;
      pSender_sp->writeValue(preset_cmd, preset_cmd_size, false);
      Serial.println("Sending preset command to Spark");
    }
  }
}
