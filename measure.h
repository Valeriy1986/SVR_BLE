#define E_RASDIUS 6371  // Radius of the earth in km

void getDistanceFromLatLonInM();
double deg2rad(double deg);
void getSpeed_60_100();
void measurement();
void getAccel();

void measurement()
{
  //getAccel();
  getSpeed_60_100();
  getDistanceFromLatLonInM();
}

void getAccel()
{
  long int t_p_20ms = millis();
  
  if(t_next_p_20ms<t_p_20ms) // all 1000 ms print
  {
    t_next_p_20ms = t_p_20ms + T_20MS;
  
    MPU6050_work();
    accel_f -= 1;
    if(accel_f_max < accel_f) 
    {
      accel_f_max = accel_f;
    }
  }
}

void getDistanceFromLatLonInM() {

  if(speed_0)
  {
    lat1 = latitude_d;
    lon1 = longitude_d;
    return;
  }

  double lat2 = latitude_d;
  double lon2 = longitude_d;
  

  double dLat = deg2rad(lat2 - lat1);  // deg2rad below
  double dLon = deg2rad(lon2 - lon1);
  double a = sin(dLat / 2) * sin(dLat / 2)
      + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sin(dLon / 2)
          * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double d = (E_RASDIUS * c) * 1000; // Distance in metr

  lat1 = lat2;
  lon1 = lon2;
  distance += d;
}

double deg2rad(double deg) {
  return deg * (M_PI / 180);
}


void getSpeed_60_100() {
  long _now_time_milli;
  //  convert time to millisec
  _now_time_milli = (hour_b*3600 + minut_b*60 + second_b)*1000 + msecond_b*10;

  speed_0  = false;
  if(speed_f < 1.0)
  {
    speed_time_milli_0 = _now_time_milli;
    speed_0_f = speed_f;
    speed_check_60_flg = true;
    speed_check_100_flg = true;
    speed_0 = true;
  }
  
  if((speed_f > 60.0) && (speed_check_60_flg))
  {
    speed_time_60 = (float)(_now_time_milli - speed_time_milli_0)/1000;
    speed_60_f = speed_f;
    speed_check_60_flg = false;
  }
  
  if((speed_f > 100.0) && (speed_check_100_flg))
  {
    speed_time_100 = (float)(_now_time_milli - speed_time_milli_0)/1000;
    speed_check_100_flg = false;
  }
}
