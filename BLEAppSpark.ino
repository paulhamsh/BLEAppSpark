#include <M5Core2.h> 
#include "NimBLEDevice.h"

#define SERVICEID "ffc0"
#define CLIENTID1 "ffc1"
#define CLIENTID2 "ffc2"

NimBLEServer *pServer;
NimBLEClient *pClient_sp;

NimBLEAdvertising *pAdvertising;
  
NimBLEScan *pScan;
NimBLEScanResults pResults;
NimBLEAdvertisedDevice device;

NimBLERemoteService *pService_sp;
NimBLERemoteCharacteristic *pReceiver_sp;
NimBLERemoteCharacteristic *pSender_sp;

void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;

  Serial.print("Spark: ");
  for (i = 0; i < length; i++) {
    Serial.print(pData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
      
        int j, l;
        const char *p;
        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          Serial.print(" ");
          Serial.print(p[j], HEX); 
        }
        Serial.println();
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

  // Create server to act as Spark
  NimBLEDevice::init("Spark 40 BLE");
  
  pServer    = NimBLEDevice::createServer();
  pClient_sp = NimBLEDevice::createClient();
  pScan      = NimBLEDevice::getScan();
  
  // Connect to Spark
  connected_sp = false;
  while (!connected_sp) {
    pResults = pScan->start(4);
    NimBLEUUID SPserviceUuid(SERVICEID);

    Serial.println("------------------------------");
    for(i = 0; i < pResults.getCount() && !connected_sp; i++) {
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
    
    if (connected_sp) {
      pService_sp = pClient_sp->getService(SPserviceUuid);
      if (pService_sp != nullptr) {
        pSender_sp   = pService_sp->getCharacteristic(CLIENTID1);
        pReceiver_sp = pService_sp->getCharacteristic(CLIENTID2);
        if (pReceiver_sp && pReceiver_sp->canNotify()) {
          if (!pReceiver_sp->subscribe(true, notifyCB_sp, true)) {
            connected_sp = false;
            pClient_sp->disconnect();
          }
        }
      }
    }
  }
  pScan->stop();

  delay(1000);
  
  Serial.println("Done Spark....");

  NimBLEService *pService = pServer->createService("FFC0");
  NimBLECharacteristic *pCharacteristic_receive = pService->createCharacteristic("FFC1", 
                  NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  NimBLECharacteristic *pCharacteristic_send = pService->createCharacteristic("FFC2", 
                  NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);  
  pCharacteristic_receive->setCallbacks(&chrCallbacks_r);
  pCharacteristic_send->setCallbacks(&chrCallbacks_s);
  
  pService->start();
  pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
  pAdvertising->addServiceUUID(pService->getUUID()); // tell advertising the UUID of our service
  pAdvertising->setScanResponse(true);
    Serial.println("WAS OK TO HERE 8"); 
  pAdvertising->start(); // start advertising
    Serial.println("WAS OK TO HERE 9"); 
}


void loop() {
  // put your main code here, to run repeatedly:
  M5.update();
}
