

#define T_OUT 20
MPU6050 accel;

void MPU605_init();
void MPU6050_work();

void MPU6050_init()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  accel.initialize();
  esp_task_wdt_reset();
  accel.CalibrateAccel(2);
  accel.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);  
  Serial.println(accel.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  
}

void MPU6050_work()
{
  int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw, ax_mod, ay_mod;
  float ax_f, ay_f, az_f;
    
  accel.getAcceleration (&ax_raw, &ay_raw, &az_raw);
  
  ax_f = ax_raw; ay_f = ay_raw; az_f = az_raw;
  
  ax_f = ax_f/32768 * 2;
  ay_f = ay_f/32768 * 2;
  az_f = az_f/32768 * 2;
 
  accel_f = sqrt(pow(ax_f, 2) + pow(ay_f, 2) + pow(az_f, 2)); 

  accel_temperature = accel.getTemperature();
  System_flags |= UBLOX_MPU6050;
}
