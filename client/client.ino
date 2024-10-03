#include "BLEDevice.h"

//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "BME280_ESP32"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID mpuServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");

// BLE Characteristics
// Pitch Characteristic
static BLEUUID pitchCharacteristicUUID("cba1d466-344c-4be3-ab3f-189f80dd7518");

// button Characteristic
static BLEUUID buttonCharacteristicUUID("ca73b3ba-39f6-4ab3-91ae-186dc9577d99");

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;
 
//Characteristicd that we want to read
static BLERemoteCharacteristic* pitchCharacteristic;
static BLERemoteCharacteristic* buttonCharacteristic;

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

//Variables to store temperature and humidity
char* pitchChar;
char* buttonChar;

//Flags to check whether new pitch and button readings are available
boolean newPitch = false;
boolean newbutton = false;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(mpuServiceUUID);
  if (pRemoteService == nullptr) {
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  pitchCharacteristic = pRemoteService->getCharacteristic(pitchCharacteristicUUID);
  buttonCharacteristic = pRemoteService->getCharacteristic(buttonCharacteristicUUID);

  if (pitchCharacteristic == nullptr || buttonCharacteristic == nullptr) {
    return false;
  }
 
  //Assign callback functions for the Characteristics
  pitchCharacteristic->registerForNotify(pitchNotifyCallback);
  buttonCharacteristic->registerForNotify(buttonNotifyCallback);
  return true;
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
    }
  }
};
 
//When the BLE Server sends a new temperature reading with the notify property
static void pitchNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  pitchChar = (char*)pData;
  newPitch = true;
}

//When the BLE Server sends a new humidity reading with the notify property
static void buttonNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  buttonChar = (char*)pData;
  newbutton = true;
}

//function that prints the latest sensor readings
void printReadings(){
  Serial.print(pitchChar);
  Serial.println(buttonChar); 
}

void setup() {
  //Start serial communication
  Serial.begin(115200);

  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      //Activate the Notify property of each Characteristic
      pitchCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      buttonCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  //if new pitch readings are available, print
  if (newPitch && newbutton){
    newPitch = false;
    newbutton = false;
    printReadings();
  }
  delay(20);
}
