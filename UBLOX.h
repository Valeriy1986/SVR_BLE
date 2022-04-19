
#define UBLOX_MESSAGE_START 0x24 // 0x24 = "$" in HEX
#define UBLOX_MESSAGE_END   0x2A // 0x2A * end of message
#define UBLOX_MES_SYNC_CNT  20
#define UBLOX_1_SYNC_STATE 0
#define UBLOX_2_SYNC_STATE 1
#define UBLOX_3_SYNC_STATE 2
#define UBLOX_MES_COUNT    6
#define UBLOX_MESSAGE_TEMP_PACK_MAX  127

static const uint32_t InitUART_brate[] = { 115200, 57600, 38400, 19200, 9600 };
static const uint8_t INITUART_BRATE_LEN = sizeof(InitUART_brate)/sizeof(InitUART_brate[0]);
byte GPS_port_speed = 0;
byte UBLOX_sync_cnt;
byte UBLOX_init_flg = 0;
static const uint8_t ubloxInit[] = {
    0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x7A, 0x12, // set rate to 10Hz (measurement period: 100ms, navigation rate: 1 cycle)
    };
static const uint8_t UBLOX_INIT_ARR_LEN = sizeof(ubloxInit)/sizeof(ubloxInit[0]);

static const char UBLOX_messages_type[UBLOX_MES_COUNT][4] = {
                                         "GLL",
                                         "RMC",
                                         "VTG",
                                         "GGA",
                                         "GSA",
                                         "GSV"
};

enum {GLL, RMC, VTG, GGA, GSA, GSV};
char UBLOX_message_temp_pack[UBLOX_MESSAGE_TEMP_PACK_MAX+1];
int UBLOX_message_temp_pack_ptr = 0;
byte UBLOX_message_ex_state = 0; // 0 - UBLOX_NO_SYNC_STATE (not found "$" and CS) // 1 - UBLOX_PRE_SYNC_STATE found one more time "$" sumbol and CS // 2- UBLOX_YES_SYNC_STATE - sumbol "$" and CS finded UBLOX_MES_SYNC_CNT
void UBLOX_mes_RX();
byte UBLOX_mes_CS_calc();
void UBLOX_pack_parse();

void UBLOX_mes_RX()
{
  int i,j;

  if (Serial2.available()) 
  { // Receive from UART2
    Curcular_UART2_buffer[curcular_uart2_in_ptr++] = Serial2.read();
    curcular_uart2_in_ptr &= CURCULAR_BUFF_UART2_SIZE;
  }
  //Serial.println("RX message start");
  //digitalWrite(LED, HIGH);

  // If UART2 buffer not empty, send data from UART2 to UART0
  if(curcular_uart2_in_ptr != curcular_uart2_out_ptr)
  {
    if(curcular_uart2_in_ptr > curcular_uart2_out_ptr)
    {
      diff_UART2 = curcular_uart2_in_ptr - curcular_uart2_out_ptr;
    }
    else if(curcular_uart2_out_ptr > curcular_uart2_in_ptr)
    {
      diff_UART2 = (curcular_uart2_in_ptr + CURCULAR_BUFF_UART2_SIZE + 1) - curcular_uart2_out_ptr;
    }

    // find $ -  start of NMEA 
    // $########*CS0x0d0x0a$
    for(i=0;i<diff_UART2;i++)
    {
      //===============================================
      if(UBLOX_message_ex_state == UBLOX_1_SYNC_STATE)
      {
        if(Curcular_UART2_buffer[curcular_uart2_out_ptr] == 0x24) // $ start of NMEA message
        {
          UBLOX_message_ex_state = UBLOX_2_SYNC_STATE;
          curcular_uart2_out_ptr++;
          curcular_uart2_out_ptr &= CURCULAR_BUFF_UART2_SIZE;          
        }
        else
        {
          curcular_uart2_out_ptr++;
          curcular_uart2_out_ptr &= CURCULAR_BUFF_UART2_SIZE;          
        }
      }
      //===============================================
      else if(UBLOX_message_ex_state == UBLOX_2_SYNC_STATE)
      {
        if(Curcular_UART2_buffer[curcular_uart2_out_ptr] == 0x24) //$ start of NMEA message
        {
          curcular_uart2_out_ptr++;
          curcular_uart2_out_ptr &= CURCULAR_BUFF_UART2_SIZE;        
          /*
          for(j=0;j<UBLOX_message_temp_pack_ptr;j++)
          {
            Serial.write(UBLOX_message_temp_pack[j]);
          }
          */  
          if(UBLOX_mes_CS_calc())
          {
            UBLOX_wrong_CS_cnt = 0;
            if(UBLOX_sync_cnt >= UBLOX_MES_SYNC_CNT)
            {
              UBLOX_message_ex_state = UBLOX_3_SYNC_STATE;
            }
            else
            {
              UBLOX_sync_cnt++;
              UBLOX_message_temp_pack_ptr = 0;
            }
          }
          else
          {
            UBLOX_sync_cnt = 0;
            curcular_uart2_out_ptr = 0;
            UBLOX_message_temp_pack_ptr = 0;
            UBLOX_message_ex_state = UBLOX_1_SYNC_STATE;
            Serial.println("Wrong CS etap 2!");
            UBLOX_wrong_CS_cnt++;
            if(UBLOX_wrong_CS_cnt > UBLOX_WRONG_CS) ESP.restart(); // can't sunc with UBLOX attemp to restart
          }
        }
        else
        {
          UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr++] = Curcular_UART2_buffer[curcular_uart2_out_ptr];
          curcular_uart2_out_ptr++;
          curcular_uart2_out_ptr &= CURCULAR_BUFF_UART2_SIZE;          
        }
      }
      //===============================================
      else if(UBLOX_message_ex_state == UBLOX_3_SYNC_STATE)
      {
        /*
        // PRINT transparent message from UBLOX
        for(j=0;j<UBLOX_message_temp_pack_ptr;j++)
        {
          Serial.write(UBLOX_message_temp_pack[j]);
        }
        */
        //digitalWrite(LED, HIGH);
        UBLOX_pack_parse();
        UBLOX_message_ex_state = UBLOX_2_SYNC_STATE;
        //digitalWrite(LED, LOW);
      }
    }
  }
  //digitalWrite(LED, LOW);
  //Serial.println("RX message end");
}

byte UBLOX_mes_CS_calc()
{
  byte i;
  byte cs_in, cs_calc;

  //Serial.println("Calc CS start");
  // calculate CS
  // $########*CS0x0d0x0a$ // CS 2 byte's in ASCII
  // 1st byte of CS
  if((UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 4] >=0x41) && (UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 4] <= 0x46)) 
  {
    cs_in = (UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 4] - 55)<<4;
  }
  else if((UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 4] >=0x30) && (UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 4] <= 0x39))
  {
    cs_in = (UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 4] - 48)<<4;
  }
  //
  // 2nd byte of CS
  if((UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 3] >=0x41) && (UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 3] <= 0x46))
  {
    cs_in |= UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 3] - 55;
  }
  else if((UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 3] >=0x30) && (UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 3] <= 0x39))
  {
    cs_in |= UBLOX_message_temp_pack[UBLOX_message_temp_pack_ptr - 3] - 48;
  }
  //
  cs_calc = 0;
  // calc CS
  for(i=0;i<UBLOX_message_temp_pack_ptr - 5;i++)
  {
    cs_calc ^= UBLOX_message_temp_pack[i];
  }
  //Serial.println(cs_calc);
  if(cs_calc == cs_in) return 1;
  
  return 0;
}

//"xxGLL", "xxRMC", "xxVTG", "xxGGA", "xxGSA", "xxGSV" - messages type from UBLOX\
// GPGSV,4,3,13,24,28,069,21,27,39,256,,32,52,213,23,40,40,201,*760x0d0x0a -  message ex



void UBLOX_pack_parse()
{

  byte i,j;
  char *_ublox_pack_ptr = UBLOX_message_temp_pack;
  char _delimiter = ',';
  char _pack_field[20];
  char *_pack_field_ptr;

  byte _pack_type, _field_cnt;
  byte _sumb_in_pack_field;

  digitalWrite(LED, HIGH);
  _pack_field_ptr = &_pack_field[0];
  _field_cnt = _sumb_in_pack_field = 0;
  _pack_type = 10;
  UBLOX_message_temp_pack_ptr -=  5;// cut *(1byte)CS(2byte)0x0d(1byte)0x0a(1byte)
  //1 split by delimiter ","
  while(UBLOX_message_temp_pack_ptr)
  {
    if((*_ublox_pack_ptr == _delimiter) || (UBLOX_message_temp_pack_ptr == 1))
    {// find delimiter, or end of pack write count of sumbols before delemiter in last element
      if(_field_cnt == 0) // if _field_cnt == 0 - pack header (xxGLL)
      {
        //Serial.println("Finding mess type");
        for(j=0;j<UBLOX_MES_COUNT;j++)
        {
          if(strstr(_pack_field,UBLOX_messages_type[j])) 
          {
            _pack_type = j; // finded pack type  = j
            break;
          }
        }
      }

      if(UBLOX_message_temp_pack_ptr == 1) // last sumb in last field of pack
      {
        *_pack_field_ptr++ = *_ublox_pack_ptr++;
        _sumb_in_pack_field++;
      }
      // when I find delimiter, I must analyse this field by _pack_type!
      switch(_pack_type) // enum {GCL, RMC, VTG, GCA, GSA, GSV};
      {
        case GLL:
          //$--GLL,llll.lll,a,yyyyy.yyy,b,hhmmss.sss,A,a       
              //llll.lll  Latitude  Latitude in ddmm.mmmm format. Leading zeros are inserted. 
              //A  N/S Indicator  ‘N’ = North, ‘S’ = South 
              //yyyyy.yyy  Longitude  Longitude in dddmm.mmmm format.Leading zeros are inserted. 
              //B  E/W Indicator  ‘E’ = East, ‘W’ = West 
              //hhmmss.sss  UTC Time  UTC of position in hhmmss.sss format, (000000.000 ~ 235959.999) 
              //A  Status  A= data valid, V= Data not valid              
          switch(_field_cnt)
          {
            case 0:
             //memcpy (UBLOX_pack_type, _pack_field, 5);
             break;
            case 1: // Latitude
             if(!Common_gps_fix_status) break;
             memcpy (UBLOX_latitude,_pack_field,_sumb_in_pack_field);
             //latitude_f = float(atof(UBLOX_latitude));
             latitude_d = strtod(UBLOX_latitude, NULL);
             latitude_d = int(latitude_d/100) +(latitude_d - int(latitude_d/100)*100)/60;
             break;
            case 3: // Longitude
             if(!Common_gps_fix_status) break;
             memcpy (UBLOX_longitude,_pack_field,_sumb_in_pack_field);
             //longitude_f = float(atof(UBLOX_longitude));
             longitude_d = strtod(UBLOX_longitude, NULL);
             longitude_d = int(longitude_d/100) +(longitude_d - int(longitude_d/100)*100)/60;
             break;
            case 5: // UTC time hhmmss.sss
             if(!Common_gps_fix_status) break;
             memcpy (UBLOX_utc_HH, _pack_field, 2);
             memcpy (UBLOX_utc_MM, _pack_field+2, 2);
             memcpy (UBLOX_utc_SS, _pack_field+4, 2);
             memcpy (UBLOX_utc_mm, _pack_field+7, 2);
             hour_b = (UBLOX_utc_HH[0]-0x30)*10 + (UBLOX_utc_HH[1]-0x30) + 5; // HH
             if(hour_b > 24) hour_b -= 24;
             else if(hour_b == 24) hour_b = 0;
             minut_b = (UBLOX_utc_MM[0]-0x30)*10 + (UBLOX_utc_MM[1]-0x30); // MM
             second_b = (UBLOX_utc_SS[0]-0x30)*10 + (UBLOX_utc_SS[1]-0x30); // SS
             msecond_b = (UBLOX_utc_mm[0]-0x30)*10 + (UBLOX_utc_mm[1]-0x30); // mm

             // check packet sequensly
             if(pack_noseq_next1 != msecond_b)
             {
              pack_noseq_cnt1++;
             }
             pack_noseq_next1 = msecond_b + 10;
             if(pack_noseq_next1 == 100) pack_noseq_next1 = 0;
             //    
             
             break;
          }
          //end indicator of "packet's" from UBLOX
          UBLOX_last_message = true;
          break;
        case RMC:
          break;
        case VTG:
          // $--VTG,x.x,T,y.y,M,u.u,N,  ,K,m
            // x.x  Course  Course over ground, degrees True (000.0 ~ 359.9)
            // y.y  Course  Course over ground, degrees Magnetic (000.0 ~ 359.9) 
            // u.u  Speed  Speed over ground in knots (000.0 ~ 999.9)
            // v.v  Speed  Speed over ground in kilometers per hour (0000.0 ~ 1800.0)
            // m  Mode  Mode indicator 
                    //‘N’ = not valid 
                    //‘A’ = Autonomous mode 
                    //‘D’ = Differential mode 
                    //‘E’ = Estimated (dead reckoning) mode 
          switch(_field_cnt)
          {
            case 0:
             //memcpy (UBLOX_pack_type, _pack_field, 5);
             break;
            case 7: // speed
             if(!Common_gps_fix_status) break;
             memcpy (UBLOX_speed_km, _pack_field, _sumb_in_pack_field);
             speed_f = float(atof(UBLOX_speed_km));
             break;
          }
          break;
        case GGA:
          //$--GGA,hhmmss.ss,llll.lll,a,yyyyy.yyy,a,x,uu,v.v,w.w,M,x.x,M,,zzzz
              //hhmmss.ss  UTC Time  UTC of position in hhmmss.sss format, (000000.000 ~ 235959.999) 
              //llll.lll  Latitude  Latitude in ddmm.mmmm format. Leading zeros are inserted. 
              //A  N/S Indicator  ‘N’ = North, ‘S’ = South 
              //yyyyy.yyy  Longitude  Longitude in dddmm.mmmm format.Leading zeros are inserted. 
              //A  E/W Indicator  ‘E’ = East, ‘W’ = West 
              //x  GPS quality indicator  GPS quality indicator 
                 //0: position fix unavailable 
                 //1: valid position fix, SPS mode  
                 //2: valid position fix, differential GPS mode 
              //uu  Satellites Used  Number of satellites in use, (00~ 24) 
              //v.v  HDOP  Horizontal dilution of precision, (00.0 ~ 99.9) 
              //w.w  Altitude  Mean sea level altitude (-9999.9 ~ 17999.9) in meter 
              //x.x  Geoidal Separation  In meter 
              //zzzz  DGPS Station ID  Differential reference stationID, 0000 ~ 1023 
              //NULL when DGPS not used 
          //Serial.println("Finded GCA!");
          switch(_field_cnt)
          {
            case 6: // FIX
             UBLOX_fix_status = _pack_field[0];
             fix_b = byte(UBLOX_fix_status - 0x30);
             if(UBLOX_fix_status != 0x30)
              Common_gps_fix_status = 1;
             else
              Common_gps_fix_status = 0;
             //Serial.println(UBLOX_fix_status);
             System_flags |= UBLOX_RX_OK; 
             break;
            case 7: // satellit
             if(!Common_gps_fix_status) break;
             memcpy (UBLOX_sat_used, _pack_field, _sumb_in_pack_field);
             sat_b = byte((UBLOX_sat_used[0]-0x30)*10 + (UBLOX_sat_used[1]-0x30));
             break;
            case 9: // altitude
             if(!Common_gps_fix_status) break;
             memcpy (UBLOX_altitude, _pack_field, _sumb_in_pack_field);
             altitude_f = float(atof(UBLOX_altitude));
             break;
          }
          break;
        case GSA:
          break;
        case GSV:
          break;
        default:
          //Serial.println("Not found!");
          UBLOX_message_temp_pack_ptr = 0;
          digitalWrite(LED, LOW);
          return;
      }
      
      _field_cnt++; // field cnt of pack
      _ublox_pack_ptr++; // in pack counter
      _pack_field_ptr = &_pack_field[0];// init pointer to array _pack_field
      memset (_pack_field, 0, sizeof(_pack_field));
      _sumb_in_pack_field = 0;
      UBLOX_message_temp_pack_ptr--;
    }
    else
    { // copy sumbols before delimitor (copy field of packet)
      //Serial.println("Accum field");
      *_pack_field_ptr++ = *_ublox_pack_ptr++;
      _sumb_in_pack_field++;
      UBLOX_message_temp_pack_ptr--;
    }
  }
  //Serial.println("Parse end");
}


void UBLOX_init()
{
  /*
  byte i;
  if(!UBLOX_init_flg)
  { 
    
    delay(200);
    // try different speed to INIT
    Serial.write("Init UBLOX UART speed.");
    //for(i=0;i<5;i++)
    for(i=0;i<INITUART_BRATE_LEN;i++)
    {
      Serial.write(".");
      Serial2.flush();
      delay(100);
      Serial2.end();  
      delay(200);
      Serial2.begin(InitUART_brate[i],SERIAL_8N1,RXD2,TXD2);
      delay(100);
      Serial2.print("$PUBX,41,1,0007,0003,115200,0*18\r\n");
      delay(200);
    }
    Serial2.end();
    delay(100);
    Serial2.begin(115200,SERIAL_8N1,RXD2,TXD2);
    Serial.print("\n\rUblox UART speed init complete!\n\r");
  }
  delay(1000);
  
  //set 10 Hz measurement rate
  Serial.write("\n\rUblox measurement rate change to 10 Hz start.");
  //for(i=0;i<14;i++)
  for(i=0;i<UBLOX_INIT_ARR_LEN;i++)
  {
    Serial2.write(ubloxInit[i]);
    //Serial.write(ubloxInit[i]);
    Serial.print(".");
  }
  Serial.print("\n\rUblox measurement rate change to 10 Hz complete!\n\r");
  

  Serial.print("\n\rUblox UART speed init complete!\n\r");
  curcular_uart2_in_ptr, curcular_uart2_out_ptr = 0;
  */
  System_flags |= UBLOX_INIT;
}
