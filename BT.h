#define BT_FLAG 0x7E
BLEServer *pServer = NULL;
BLEAdvertising *pAdvertising = pServer->getAdvertising();

BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;



// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "c4913d0c-65d6-11eb-ae93-0242ac130002" // UART service UUID
#define CHARACTERISTIC_UUID_RX "c4913f50-65d6-11eb-ae93-0242ac130002"
#define CHARACTERISTIC_UUID_TX "c4914036-65d6-11eb-ae93-0242ac130002"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("\n\rConnected!");
      System_flags |= BT_CONNECT;
      deviceConnected = true;
      delay(500);
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("DisConnected!");
      System_flags &= ~(BT_CONNECT);
      deviceConnected = false;
      delay(1000);
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void BT_init()
{
  // Create the BLE Device
  //BLEDevice::init("SVR BLE");
  BLEDevice::init("SVR BLE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  //pServer->getAdvertising()->start();
  //m_bleAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("Waiting a client connection to notify...");
  System_flags |= BT_INIT;
}

void BT_dispatcher()
{
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      //delay(500); // give the bluetooth stack the chance to get things ready
      //pServer->startAdvertising(); // restart advertising
      
      pAdvertising->addServiceUUID(SERVICE_UUID);
      pAdvertising->start();
      
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
  // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }
}

// Packet to BT :
// |-- 1 byte FLAG --|-- 1 byte FIX --|-- 1 byte SAT --|-- 4 byte SPEED --|-- 4 byte TIME --|-- 8 byte LATITUDE --|-- 8 byte LONGITUDE --|-- 4 byte ALTITUDE --|-- 4 byte ACCEL --|-- 1 byte CS         --|
// |      0x7E       |   00 - 02      |   00 - 99      |   00 - 255       |  HH MM SS mm    |    69.12344         |      72.4327         |       450.3         |                  |XOR all bytes before CS|
void BT_send_packet()
{
  byte _lat, _lon, _alt, _spd, _acl;
  unsigned char * ptr;
 
  bt_rx_packet_out_ptr = 0;
  // flag
  bt_pack_cs = BT_FLAG;
  BT_packet[bt_rx_packet_out_ptr++] = BT_FLAG;
  // fix
  bt_pack_cs ^= fix_b;
  BT_packet[bt_rx_packet_out_ptr++] = fix_b;
  // sat
  bt_pack_cs ^= sat_b;
  BT_packet[bt_rx_packet_out_ptr++] = sat_b;

  // speed
  ptr = (unsigned char *) &speed_f;
  _spd = *(ptr+3);
  bt_pack_cs ^= _spd;
  BT_packet[bt_rx_packet_out_ptr++] = _spd;
  // speed
  _spd = *(ptr+2);
  bt_pack_cs ^= _spd;
  BT_packet[bt_rx_packet_out_ptr++] = _spd;
  // speed
  _spd = *(ptr+1);
  bt_pack_cs ^= _spd;
  BT_packet[bt_rx_packet_out_ptr++] = _spd;
  // speed
  _spd = *(ptr);
  bt_pack_cs ^= _spd;
  BT_packet[bt_rx_packet_out_ptr++] = _spd;

  // HH
  bt_pack_cs ^= hour_b;
  BT_packet[bt_rx_packet_out_ptr++] = hour_b;
  // MM
  bt_pack_cs ^= minut_b;
  BT_packet[bt_rx_packet_out_ptr++] = minut_b;
  //SS
  bt_pack_cs ^= second_b;
  BT_packet[bt_rx_packet_out_ptr++] = second_b;
  //mm
  // check packet sequensly
  if(pack_noseq_next != msecond_b)
  {
   pack_noseq_cnt++;
  }
  pack_noseq_next = msecond_b + 10;
  if(pack_noseq_next == 100) pack_noseq_next = 0;
 //    
  bt_pack_cs ^= msecond_b;
  BT_packet[bt_rx_packet_out_ptr++] = msecond_b;
  
  // lat
  ptr = (unsigned char *) &latitude_d;
  _lat =  *(ptr+7);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr+6);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr+5);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr+4);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr+3);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr+2);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr+1);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lat
  _lat =  *(ptr);
  bt_pack_cs ^= _lat;
  BT_packet[bt_rx_packet_out_ptr++] = _lat;
  // lon
  ptr = (unsigned char *) &longitude_d;
  _lon = *(ptr+7);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr+6);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr+5);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr+4);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr+3);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr+2);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr+1);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // lon
  _lon = *(ptr);
  bt_pack_cs ^= _lon;
  BT_packet[bt_rx_packet_out_ptr++] = _lon;
  // alt
  ptr = (unsigned char *) &altitude_f;
  _alt = *(ptr+3);
  bt_pack_cs ^= _alt;
  BT_packet[bt_rx_packet_out_ptr++] = _alt;
  // alt
  _alt = *(ptr+2);
  bt_pack_cs ^= _alt;
  BT_packet[bt_rx_packet_out_ptr++] = _alt;
  // alt
  _alt = *(ptr+1);
  bt_pack_cs ^= _alt;
  BT_packet[bt_rx_packet_out_ptr++] = _alt;
  // alt
  _alt = *(ptr);
  bt_pack_cs ^= _alt;
  BT_packet[bt_rx_packet_out_ptr++] = _alt;
  
  // accel
  ptr = (unsigned char *) &accel_f_max;
  _acl = *(ptr+3);
  bt_pack_cs ^= _acl;
  BT_packet[bt_rx_packet_out_ptr++] = _acl;
  // accel
  _acl = *(ptr+2);
  bt_pack_cs ^= _acl;
  BT_packet[bt_rx_packet_out_ptr++] = _acl;
  // accel
  _acl = *(ptr+1);
  bt_pack_cs ^= _acl;
  BT_packet[bt_rx_packet_out_ptr++] = _acl;
  // accel
  _acl = *(ptr);
  bt_pack_cs ^= _acl;
  BT_packet[bt_rx_packet_out_ptr++] = _acl;
  accel_f_max = 0;
  // BAT level
  bt_pack_cs ^= ADC_value;
  BT_packet[bt_rx_packet_out_ptr++] = ADC_value;

  // CS
  BT_packet[bt_rx_packet_out_ptr++] = bt_pack_cs;
  // send all packet
  /*
  byte BT_packet1[BT_PACK_SIZE] = {'1','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','5','6'};

  BT_packet1[35] += huynya;
  huynya++;
  */
  pTxCharacteristic->setValue(BT_packet, BT_PACK_SIZE);
  pTxCharacteristic->notify();

  //SerialBT.write(BT_packet, BT_PACK_SIZE);
  UBLOX_last_message = false;
}
