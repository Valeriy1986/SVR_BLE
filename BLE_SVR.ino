#include <HardwareSerial.h>
#include <esp_task_wdt.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEAdvertising.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "common.h"
#include "terminal.h"
#include "accel.h"
#include "measure.h"
#include "system.h"
#include "BT.h"
#include "UBLOX.h"

// comment for git test


void setup() {
  // serial and WD init
  System_init();
  // BT init
  BT_init();
  // UBLOX init
  UBLOX_init();
  // Init MPU6050
  MPU6050_init();
}

void loop() {

  t_p = millis();
  
  UBLOX_mes_RX(); // recive and parse messages from GPS receiver
  if ((deviceConnected) && (UBLOX_last_message)) BT_send_packet(); // send data to BT

  if(t_next_100ms<t_p) // all 100 ms print
  {
    t_next_100ms = t_p + T_100MS;
    esp_task_wdt_reset();
    Terminal_work(); 
  }

  if(t_next_500ms<t_p) // all 500 ms print
  {
    t_next_500ms = t_p + T_500MS;
    Battary_control();
    //measurement();
  }

  BT_dispatcher();
  getAccel();
  Led_Control();
}
