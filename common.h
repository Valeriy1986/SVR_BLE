
//#define TRANSPARENT  0
#define SW              02
byte System_flags = 0;
#define BT_INIT       0x01
#define UBLOX_INIT    0x02
#define UBLOX_MPU6050 0x04
#define UBLOX_RX_OK   0x08
#define BT_CONNECT    0x10
//7 seconds WDT
#define WDT_TIMEOUT 7
#define T_100MS  100
#define T_500MS  500
#define T_1000MS 1000
#define T_5000MS 5000
#define T_200MS  200
#define T_20MS   50
#define T_ACCEL  3000

#define UBLOX_WRONG_CS 100
#define RXD2 16
#define TXD2 17
#define CURCULAR_BUFF_UART2_SIZE 255
#define I2C_SDA 21
#define I2C_SCL 22
#define T_OUT 20
#define ACEL_CALIB_ALPHA 0.1
// Соединим потенциометр к пину GPIO 34 (Analog ADC1_CH6)
#define ADC_1_PIN  34
#define MAX_BAT    3.8
#define MIN_BAT    3.2
// RGB LED'S
#define LED_OFF   0
#define LED       2
#define RED_LED   15
#define GREEN_LED 4
#define BLUE_LED  5

// BLINK LEDS
#define LED_BLINK_INTERVAL  5 // 500 ms
byte Leds_blink_interval = 0;
byte Leds_ON_OFF_state = 0;

unsigned long int t_next_accel, t_next_p_20ms, t_next_100ms, t_next_200ms, t_next_500ms, t_next_1000ms, t_next_5000ms, wd_timer;
// uart
int Curcular_UART2_buffer[CURCULAR_BUFF_UART2_SIZE+1];
int curcular_uart2_in_ptr, curcular_uart2_out_ptr;
int diff_UART2;
// bt and packager
byte BT_connected_flag = 0;
byte Common_gps_fix_status = 0;
#define BT_PACK_SIZE 37
byte BT_packet[BT_PACK_SIZE];
byte bt_rx_packet_out_ptr;
byte bt_pack_cs;
// ublox
byte UBLOX_wrong_CS_cnt;
byte fix_b, sat_b, hour_b, minut_b, second_b, msecond_b;
float latitude_f;
float longitude_f;
float altitude_f;
float distance_f;
float speed_f;
char UBLOX_longitude[11]; // yyyyy.yyy 
char UBLOX_latitude[11]; // llll.lll
double latitude_d;
double longitude_d;
bool UBLOX_last_message;
// GPS data in ASCII
char UBLOX_pack_type[5]; 
char UBLOX_altitude[7]; // -9999.9 ~ 17999.9 meter
//char UBLOX_longitude[9]; // yyyyy.yyy 
//char UBLOX_latitude[8]; // llll.lll
char UBLOX_utc_HH[2]; // HH
char UBLOX_utc_MM[2]; // MM
char UBLOX_utc_SS[2]; // SS
char UBLOX_utc_mm[2]; // mm
char UBLOX_speed_km[8]; // (0000.000) km/h
char UBLOX_sat_used[2]; // max 24
char UBLOX_fix_status; 


// test
int pack_noseq_cnt;
byte pack_noseq_next;
int pack_noseq_cnt1;
byte pack_noseq_next1;

// measure
long speed_time_milli_0;
float speed_time_60;
float speed_time_100;
bool speed_check_60_flg;
bool speed_check_100_flg;
double lat1;
double lon1;
double distance;
bool speed_0;
bool start_402_calc_now;
bool start_402_calc_prew;
float speed_0_f;
float speed_60_f;

//accel
float ax_max_p,ax_max_n, ay_max_p, ay_max_n, az_max_p, az_max_n;
float accel_f;
float accel_f_0;
float accel_f_max;
float pre_speed_f;
bool accel_start = true;
int accel_temperature;
byte huynya;

// ADC
byte ADC_value = 0;
float ADC_value_avg = 0;
int ADC_cnt = 0;
#define ADC_AVG_ATTEMPT  120

long int t_p;
