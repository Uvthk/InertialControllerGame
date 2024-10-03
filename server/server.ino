#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

#define bleServerName "BME280_ESP32"

Adafruit_MPU6050 mpu; // I2C

float meanAx = 0, meanAy = 0, meanAz = 0;
float X, Y, Z;
int pitch, button;
int numReadings = 100;
int pin = 4;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 20;

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

// Pitch Characteristic and Descriptor
BLECharacteristic mpuPitchCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor mpuPitchDescriptor(BLEUUID((uint16_t)0x2902));

// button Characteristic and Descriptor
BLECharacteristic buttonCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor buttonDescriptor(BLEUUID((uint16_t)0x2903));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void initMPU(){
  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1);
  }
}

void calibration(){
  for(int i = 0; i < numReadings; i++){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    meanAx += a.acceleration.x;
    meanAy += a.acceleration.y;
    meanAz += a.acceleration.z;
    delay(10);
  }
  meanAx /= numReadings;
  meanAy /= numReadings;
  meanAz /= numReadings;
}

void setup() {
  // Start serial communication 
  Serial.begin(115200);
  pinMode(pin, INPUT);

  // Init BME Sensor
  initMPU();

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *mpuService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  // Pitch
  mpuService->addCharacteristic(&mpuPitchCharacteristics);
  mpuPitchDescriptor.setValue("MPU Pitch");
  mpuPitchCharacteristics.addDescriptor(&mpuPitchDescriptor);

  // Button
  mpuService->addCharacteristic(&buttonCharacteristics);
  buttonDescriptor.setValue("Button");
  buttonCharacteristics.addDescriptor(new BLE2902());
  
  // Start the service
  mpuService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  calibration();
  delay(100);
}

void loop() {
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      // Get new sensor events with the readings
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      // Read MPU6050 acceleration
      X = a.acceleration.x - meanAx;
      Y = a.acceleration.y - meanAy;
      Z = a.acceleration.z - meanAz + 9.807;
      // Calculate Pitch and button
      pitch = atan2(-X, sqrt(Y * Y + Z * Z)) * 180.0 / PI;
      button = digitalRead(pin);
      
      //Notify Pitch reading from MPU sensor
      static char pitchTemp[6];
      dtostrf(pitch, 6, 2, pitchTemp);
      //Set pitch Characteristic value and notify connected client
      mpuPitchCharacteristics.setValue(pitchTemp);
      mpuPitchCharacteristics.notify();
      Serial.print("Pitch: ");
      Serial.print(pitch);
      Serial.print(" deg");
      
      //Notify button reading
      static char buttonTemp[6];
      dtostrf(button, 6, 2, buttonTemp);
      //Set button Characteristic value and notify connected client
      buttonCharacteristics.setValue(buttonTemp);
      buttonCharacteristics.notify();   
      Serial.print(" - button: ");
      Serial.println(button);
      
      lastTime = millis();
    }
  }
}
