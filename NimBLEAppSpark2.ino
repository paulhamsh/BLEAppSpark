#include <M5Core2.h> 
#include "NimBLEDevice.h"

#define C_SERVICE "ffc0"
#define C_CHAR1   "ffc1"
#define C_CHAR2   "ffc2"

#define S_SERVICE "ffc0"
#define S_CHAR1   "ffc1"
#define S_CHAR2   "ffc2"

NimBLEServer *pServer;
NimBLEService *pService;
NimBLECharacteristic *pCharacteristic_receive;
NimBLECharacteristic *pCharacteristic_send;

NimBLEClient *pClient_sp;

NimBLEAdvertising *pAdvertising;
  
NimBLEScan *pScan;
NimBLEScanResults pResults;
NimBLEAdvertisedDevice device;

NimBLERemoteService *pService_sp;
NimBLERemoteCharacteristic *pReceiver_sp;
NimBLERemoteCharacteristic *pSender_sp;


bool triggered_to_amp, triggered_to_app;
int cmd_size, cmd_size_app;
byte cmd[1000], cmd_app[1000];

void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  byte b;

  Serial.print("Spark: ");
  for (i = 0; i < length; i++) {
    b = pData[i];
    Serial.print(b, HEX);
    Serial.print(" ");
    cmd_app[i] = b;
  }
  Serial.println();
  cmd_size_app = i;
  triggered_to_app = true;

}

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead, value: ");
        int j, l;
        const char *p;
        byte b;
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
          cmd[j] = b;
        }
        Serial.println();
        cmd_size = j;
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

        Serial.println(str);
    };
};
    
/** Handler class for descriptor actions */    
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


/** Define callback instances globally to use for multiple Charateristics \ Descriptors */ 
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks_s, chrCallbacks_r;

bool connected_sp;
int i;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("BLE App Spark");
  Serial.println("Started");

  triggered_to_app = false;
  triggered_to_amp = false;

  // Create server to act as Spark
  NimBLEDevice::init("Spark 40 BLE");
  pClient_sp = NimBLEDevice::createClient();
  pScan      = NimBLEDevice::getScan();
    
  // Set up server
  pServer = NimBLEDevice::createServer();
  pService = pServer->createService(S_SERVICE);
  pCharacteristic_receive = pService->createCharacteristic(S_CHAR1, 
                  NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristic_send = pService->createCharacteristic(S_CHAR2, 
                  NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);  
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
  while (!connected_sp) {
    pResults = pScan->start(4);
    NimBLEUUID SPserviceUuid(C_SERVICE);

    Serial.println("------------------------------");
    for(i = 0; i < pResults.getCount()  && !connected_sp; i++) {
      device = pResults.getDevice(i);

      // Print info
      Serial.print("Name ");
      Serial.println(device.getName().c_str());

      if (device.isAdvertisingService(SPserviceUuid)) {
        Serial.print("Found Spark - trying to connect....");
        if(pClient_sp->connect(&device)) {
          connected_sp = true;
          Serial.println("connected");
        }
      }
    }

    // Set up client
    if (connected_sp) {
      pService_sp = pClient_sp->getService(SPserviceUuid);
      if (pService_sp != nullptr) {
        pSender_sp   = pService_sp->getCharacteristic(C_CHAR1);
        pReceiver_sp = pService_sp->getCharacteristic(C_CHAR2);
        if (pReceiver_sp && pReceiver_sp->canNotify()) {
          if (!pReceiver_sp->subscribe(true, notifyCB_sp, true)) {
            connected_sp = false;
            Serial.println("Spark not ok");
            //pClient_sp->disconnect();
            NimBLEDevice::deleteClient(pClient_sp);
          }
        }
      }
    }

  }

  Serial.println("Done Spark....");
  

  pAdvertising->start(); // start advertising

}


void loop() {
  // put your main code here, to run repeatedly:
  M5.update();

  if (triggered_to_amp) {
    triggered_to_amp = false;
    if (connected_sp) {
      pSender_sp->writeValue(cmd, cmd_size, false);
      Serial.println("Wrote to amp");
    }
  }

  if (triggered_to_app) {
    triggered_to_app = false;
    pCharacteristic_send->setValue(cmd_app, cmd_size_app);
    pCharacteristic_send->notify(true);
    Serial.println("Wrote to app");
  }
}
