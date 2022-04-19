void Led_Off()
{
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}

void Led_Control()
{
  if(!Common_gps_fix_status)
  { 
    if(t_next_200ms<t_p)
    {
      t_next_200ms = t_p + T_200MS;
      goto _leds_work_label;
    }
  }
  else
  {
    if(!(System_flags & BT_CONNECT))
    {
      if(t_next_1000ms<t_p)
      {
        t_next_1000ms = t_p + T_1000MS;
        goto _leds_work_label;
      }
    }
    else
    {
      if(t_next_5000ms<t_p)
      {
        if(Leds_ON_OFF_state != 0)
        {
          t_next_5000ms = t_p + T_5000MS;
        }
        else
        {
          t_next_5000ms = t_p + T_500MS;
        }
        goto _leds_work_label;
      }
    }
  }  
  return;

_leds_work_label:
  
  Leds_ON_OFF_state ^= 0x01;
  
  if(Leds_ON_OFF_state == 0) 
  {
    Led_Off();
    return;
  }
  
  if(System_flags == 0)
  {
    Led_Off();
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
  }

  if(System_flags & (BT_INIT|UBLOX_INIT|UBLOX_MPU6050))
  {
    Led_Off();
    digitalWrite(RED_LED, HIGH);
  }
  if(System_flags & UBLOX_RX_OK)  
  {
    System_flags &= ~(UBLOX_RX_OK);
    Led_Off();
    digitalWrite(GREEN_LED, HIGH);
  }
  if (System_flags & BT_CONNECT)
  {
    Led_Off();
    digitalWrite(BLUE_LED, HIGH);
  }
}

void System_init()
{
  System_flags = 0;
  ADC_value = 0;
  // Serials init
  Serial.begin(115200);
  delay(100);
  Serial2.begin(115200,SERIAL_8N1,RXD2,TXD2);
  delay(100);
  // WD init
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  // Common init
  pinMode(LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  Led_Control();
  Serial.printf("Software version:  %d \n\r", SW);

  Serial.printf("Xtal:  %d \n\r", getXtalFrequencyMhz());
  Serial.printf("Cpu:  %d \n\r", getCpuFrequencyMhz());
  Serial.printf("Apb:  %d \n\r", getApbFrequency());
}

void Battary_control()
{ // max = 3.8 // min = 3.2
  float adc_value, temp;
  adc_value = analogRead(ADC_1_PIN);
  adc_value *= 0.0018;
  
  if (adc_value > MAX_BAT)
    ADC_value = 100;
  else if(adc_value < MIN_BAT)
    //ADC_value = 0;  
    return;
  else
  {
    ADC_value_avg += ((adc_value - MIN_BAT)/ (MAX_BAT - MIN_BAT) )*100;
    ADC_cnt++;
    if (ADC_cnt == ADC_AVG_ATTEMPT)
    {
      ADC_value = (int)(ADC_value_avg/ADC_AVG_ATTEMPT);
      ADC_value_avg = 0;
      ADC_cnt = 0;
    }
    else if(ADC_value == 0)
    {
      ADC_value = ADC_value_avg;
    }
  }  
}
