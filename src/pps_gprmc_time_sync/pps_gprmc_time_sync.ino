#include <SoftwareSerial.h>

const byte pps_pin = 8;
const byte msg_pin = 9;
const byte rx_pin = 10;
const byte tx_pin = 11;
const bool inverse_logic = false;
SoftwareSerial gprmc_conn(rx_pin, tx_pin, inverse_logic);

const unsigned int trigger_freq = 1;
const unsigned long dt = 1000000 / trigger_freq; // => dt = 1 sec
const unsigned long dt_pps_pull_low = dt + 100000; // => dt + 100ms
const unsigned long dt_sent_gprmc = dt_pps_pull_low + 250000; // => dt_pps_pull_low + 250ms
unsigned long timestamp;
unsigned long trigger_start_time;
unsigned long ts_pps_high;
unsigned long ts_pps_low;
unsigned long ts_msg_high;
unsigned long ts_msg_low;
unsigned long i;

// Start timestamp: HH:MM:SS
int hh = 00;
int mm = 01;
int ss = 02;
// GPS coordinates from Taipei
char pos_latitude[] = "25.04776";
char pos_longitude[] = "121.53185";

void setup() {
  pinMode(rx_pin, INPUT);
  pinMode(tx_pin, OUTPUT);
  pinMode(pps_pin, OUTPUT); // PPS
  pinMode(msg_pin, OUTPUT); // Indicator of msg_sent
  Serial.begin(9600);
  gprmc_conn.begin(9600);
  // gprmc_conn.println("Hello World!");
  trigger_start_time = micros();
}

void loop() {
  char buffer[128];
  byte CRC = 0;
  
  timestamp = micros() - trigger_start_time;
  if (timestamp >= dt*i + dt_sent_gprmc)
  {
    // digitalWrite(msg_pin, HIGH);
    // ts_msg_high = micros();
    
    i += 1;
    if (ss == 59)
    {
      ss = 0;
      if (mm == 59)
      {
        mm = 0;
        hh += 1;
      }
      else mm += 1;
    }
    else ss += 1;


    // Prepare GPRMC msg
    sprintf(buffer, "GPRMC,%02d%02d%02d,A,%s,N,%s,E,022.4,084.4,070423,,A", hh, mm, ss, pos_latitude, pos_longitude);
    for (byte x = 0; x < strlen(buffer); x++) {
      // XOR every character in between '$' and '*'
      CRC = CRC ^ buffer[x];
    }
    
    // Send GPRMC msg
    gprmc_conn.print("$");
    gprmc_conn.print(buffer);
    gprmc_conn.print("*");
    gprmc_conn.print(CRC, HEX);
    gprmc_conn.println();
  
    // digitalWrite(msg_pin, LOW);
    // ts_msg_low = micros();

#if 0
    // Print Debug Messages
    Serial.print("PPS at: ");
    Serial.print(ts_pps_high);
    Serial.print("  PPS low after: ");
    Serial.print(ts_pps_low-ts_pps_high);
    Serial.print("  MSG high after: ");
    Serial.print(ts_msg_high-ts_pps_high);
    Serial.print("  MSG low after: ");
    Serial.print(ts_msg_low-ts_pps_high);
    Serial.print("  $");
    Serial.print(buffer);
    Serial.print("*");
    Serial.print(CRC, HEX);
    Serial.println();
#endif

  }
  else if (timestamp >= dt*i + dt_pps_pull_low)
  {
    // Pull pps to low
    digitalWrite(pps_pin, LOW);
    ts_pps_low = micros();
  }
  else if (timestamp >= dt*i)
  {
    // Pull pps to high
    digitalWrite(pps_pin, HIGH);  
    ts_pps_high = micros();
  }

}
