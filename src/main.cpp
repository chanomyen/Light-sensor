#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "WiFiUdp.h"

struct setting
{
  uint16_t interval   = 5;
  char * ssid         = "LPR";
  char * password     = "0024261362";
  // char * ssid         = "Greenhouse";
  // char * password     = "0940815410";
  uint16_t port        = 5555;
  IPAddress local_ip  = {192,168,1,144};
  IPAddress target_ip = {192,168,1,37};   
  IPAddress gateway   = {192,168,1,1};    
  IPAddress subnet    = {255,255,255,0};   
};
setting config;

WiFiUDP udp;
Adafruit_ADS1115 ads; 
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool dataReady = false;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  dataReady = true;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void serialConfig();
void applyConfig(String ssid, String password, String local_ip, String gateway, String subnet, String target_ip, uint8_t interval, uint16_t port);
void applyConfig();
 
void setup() {
 
  Serial.begin(115200);

  ads.setGain(GAIN_ONE);
  ads.begin();
  // Setup 3V comparator on channel 0
  ads.startComparator_SingleEnded(0, 1000);

  // applyConfig();

  // Network setup
  WiFi.begin(config.ssid, config.password);
  Serial.println("Connecting AccessPoint " + String(config.ssid));
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  WiFi.config(config.local_ip, config.gateway, config.subnet);

  // UDP setup
  udp.begin(config.port);

  // Set interval
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, config.interval*1000000, true);
  timerAlarmEnable(timer);
  // timerAlarmWrite(timer, config.interval*1000000, true);
  // timerAlarmEnable(timer);
 
}
 
void loop() {
  // if (WiFi.status() != WL_CONNECTED){
  //   WiFi.reconnect();
  // }

  if (dataReady) {
    uint16_t adc;
    // Comparator will only de-assert after a read
    adc = ads.getLastConversionResults();
    Serial.print("ADC0: ");
    char packet[6];
    sprintf(packet, "%d", adc);
    Serial.println(String(packet));
    udp.beginPacket(config.target_ip, config.port);
    udp.write((uint8_t *)packet, 6);
    udp.endPacket();
    portENTER_CRITICAL(&timerMux);
    dataReady = false;
    portEXIT_CRITICAL(&timerMux);
  }
  
  if (Serial.available()) {
    String command = Serial.readString();
    if ((command == "config") || (command == "config\n") || (command == "config\r\n")) {
      serialConfig();
    } else if ((command == "show config") || (command == "show config\n") || (command == "show config\r\n")) {
      // readConfig();
    }
  }
  // delay(config.interval*1000);
}

void serialConfig() {
  timerAlarmDisable(timer);
  Serial.println("# Configuratin mode...");
  Serial.println("# Input 0 to use the default value.");

  Serial.print("$ Interval [default:" + String(config.interval) + "] : \t\t");
  while (!Serial.available());
  int interval = Serial.parseInt();
  if (interval == 0) {
    interval = config.interval;
  } else {
    config.interval = interval;
  }
  Serial.println(interval);
    
  Serial.print("$ Subnet [default:" + config.subnet.toString() + "] : \t");
  while (!Serial.available());
  String subnet = Serial.readString();
  subnet.replace('\n', 0);
  if ((subnet == "0") || (subnet == "")) {
    subnet = config.subnet.toString();
  } else {
    config.subnet.fromString(subnet);
  }
  Serial.println(subnet);

  Serial.print("$ WiFi SSID [default:" + String(config.ssid) + "] : \t\t");
  while (!Serial.available());
  String ssid = Serial.readString();
  ssid.replace('\n', 0);
  if ((ssid == "0") || (ssid == "")) {
    ssid = config.ssid;
  } else {
    strcpy(config.ssid, ssid.c_str());
  }
  Serial.println(ssid);

  Serial.print("$ WiFi Password [default:" + String(config.password) + "] : \t");
  while (!Serial.available());
  String password = Serial.readString();
  password.replace('\n', 0);
  if ((password == "0") || (password == "")) {
    password = config.password;
  } else {
    strcpy(config.password, password.c_str());
  }
  Serial.println(password);

  Serial.print("$ Local IP [default:" + config.local_ip.toString() + "] : \t");
  while (!Serial.available());
  String local_ip = Serial.readString();
  local_ip.replace('\n', 0);
  if ((local_ip == "0") || (local_ip == "")) {
    local_ip = config.local_ip.toString();
  } else {
    config.local_ip.fromString(local_ip);
  }
  Serial.println(local_ip);

  Serial.print("$ Gateway [default:" + config.gateway.toString() + "] : \t");
  while (!Serial.available());
  String gateway = Serial.readString();
  gateway.replace('\n', 0);
  if ((gateway == "0") || (gateway == "")) {
    gateway = config.gateway.toString();
  } else {
    config.gateway.fromString(gateway);
  }
  Serial.println(gateway);


  Serial.print("$ Target IP [default:" + config.target_ip.toString() + "] : \t");
  while (!Serial.available());
  String target_ip = Serial.readString();
  target_ip.replace('\n', 0);
  if ((target_ip == "0") || (target_ip == "")) {
    target_ip = config.target_ip.toString();
  } else {
    config.target_ip.fromString(target_ip);
  }
  Serial.println(target_ip);

  Serial.print("$ port [default:" + String(config.port) + "] : \t\t");
  while (!Serial.available());
  uint16_t port = Serial.parseInt();
  if (port == 0) {
    port = config.port;
  } else {
    config.port = port;
  }
  Serial.println(port);

  // Serial.println("WiFi SSID: " + ssid);
  // Serial.println("WiFi Password: " + password);
  // Serial.println("Local IP: " + local_ip);
  // Serial.println("Gateway: " + gateway);
  // Serial.println("Subnet: " + subnet);
  // Serial.println("Target IP: " + target_ip);

  // applyConfig(ssid, password, local_ip, gateway, subnet, target_ip, interval, port);
  applyConfig();
  
}

void applyConfig(String ssid, String password, String local_ip, String gateway, String subnet, String target_ip, uint8_t interval,uint16_t port) {
  // Set interval
  config.interval = interval;
  // timerAlarmDisable(timer);
  timerAlarmWrite(timer, config.interval*1000000, true);

  WiFi.disconnect(true);
  WiFi.begin(config.ssid, config.password);
  Serial.println("Connecting AccessPoint " + String(config.ssid));
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  
  IPAddress ip;
  IPAddress server_ip;
  IPAddress gw;
  IPAddress netmask;

  ip.fromString(local_ip);
  server_ip.fromString(target_ip);
  gw.fromString(gateway);
  netmask.fromString(subnet);

  WiFi.config(ip, gw, netmask);

  // UDP setup
  udp.stop();
  udp.begin(port);

  timerAlarmEnable(timer);
}

void applyConfig() {
  // Network setup
  WiFi.disconnect(true);
  WiFi.begin(config.ssid, config.password);
  Serial.println("Connecting AccessPoint " + String(config.ssid));
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  WiFi.config(config.local_ip, config.gateway, config.subnet);

  // UDP setup
  udp.begin(config.port);

  // Set interval
  timerAlarmWrite(timer, config.interval*1000000, true);
  timerAlarmEnable(timer);
}