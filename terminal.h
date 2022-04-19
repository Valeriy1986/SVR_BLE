
void Terminal_work();

void Terminal_work() {
    if (System_flags & BT_CONNECT)
    {
      return;
    }
    /// temp 
    //fix_b = 2;
    ///
    Serial.print("FIX:");
    Serial.print(fix_b);
    if( fix_b != 0 )
    { 
      Serial.print("|Sat:");
      Serial.print(sat_b);
      
      Serial.print("|Speed:");
      Serial.print(speed_f);

      Serial.print("|Time:");
      Serial.print(hour_b);Serial.print(".");Serial.print(minut_b);Serial.print(".");Serial.print(second_b);Serial.print("-");Serial.print(msecond_b);

      //Serial.print(" Lat:");
      //Serial.print(latitude_d);
      
      //Serial.print(" Lon:");
      //Serial.print(longitude_d);


      //Serial.print(" Accel:");
      //Serial.print(accel_f);

      //Serial.print(" ALt:");
      //Serial.print(altitude_f);

      //Serial.print("|SS:");
      //Serial.print(speed_0_f);

      //Serial.print("|SS60:");
      //Serial.print(speed_60_f);

      Serial.print("|0-60:");
      Serial.print(speed_time_60);

      //Serial.print("|0-100:");
      //Serial.print(speed_time_100);

      //Serial.print(" UBLOX SQ:"); // UBLOX
      //Serial.print(pack_noseq_cnt1);

      //Serial.print(" BT SQ:"); //TO BT
      //Serial.print(pack_noseq_cnt);

      Serial.print(" Accel:");
      Serial.print(accel_f);

      //Serial.print(" Amax:");
      //Serial.print(accel_f_max);

      //Serial.printf(" Temp: %f", (float)accel_temperature/340 + 36.53);
      //Serial.print(accel_temperature);
      Serial.printf(" ADC: %d", ADC_value);

      //Serial.print(" D:"); //TO BT
      //Serial.print(distance);


      Serial.print(" "); 

      //Serial.print(" Dist:");
      //Serial.print(distance);

    }
    Serial.print("\r");
}
