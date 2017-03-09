#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include "OTAUpdate.h"

extern "C" {
#include <user_interface.h>
}

/* Fallback configuration if connection fails */
const char ssid[] = "WRITE_SSID_HERE";
const char password[] = "WRITE_PASSWORD_HERE";

/* Name and version */
const char VERSION[] = "0.1-dev (20170305)";

#define LED_PIN           13
#define NEOPIXEL_PIN      2
#define NEOPIXEL_PIXELS   310
#define UDP_PORT          8888
#define HTTP_PORT         80
#define LOG_PORT          Serial  /* Serial port for console logging */
#define CONNECT_TIMEOUT   15000   /* 15 seconds */

char ReplyBuffer[] = "OK";        // a string to send back

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
WiFiUDP           udp;
AsyncWebServer    web(HTTP_PORT); /* Web Server */

RF_PRE_INIT() {
  //wifi_set_phy_mode(PHY_MODE_11G);    // Force 802.11g mode
  system_phy_set_powerup_option(31);  // Do full RF calibration on power-up
  system_phy_set_max_tpw(82);         // Set max TX power
}



/* Forward Declarations */
int initWifi();
void initWeb();

void setup() {
  /* Configure SDK params */
  wifi_set_sleep_type(NONE_SLEEP_T);

  /* Generate and set hostname */
  char chipId[7] = { 0 };
  snprintf(chipId, sizeof(chipId), "%06x", ESP.getChipId());
  String hostname = "esps_" + String(chipId);
  WiFi.hostname(hostname);
  ArduinoOTA.setHostname(hostname.c_str());
  
  /* Initial pin states */
  pinMode(NEOPIXEL_PIN, OUTPUT);
  digitalWrite(NEOPIXEL_PIN, LOW);

  /* Setup serial log port */
  LOG_PORT.begin(115200);
  delay(10);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  LOG_PORT.println("");
  LOG_PORT.print(F("Luma.SmartHub.LedStrip ESP8266 v"));
  for (uint8_t i = 0; i < strlen_P(VERSION); i++)
    LOG_PORT.print((char)(pgm_read_byte(VERSION + i)));
  LOG_PORT.println("");

  Serial.print("Strip initialized with ");
  Serial.print(NEOPIXEL_PIXELS);
  Serial.println(" pixels.");
  
  /* Fallback to default SSID and passphrase if we fail to connect */
  int status = initWifi();
  if (status != WL_CONNECTED) {
    LOG_PORT.println(F("*** Timeout - Reverting to default SSID ***"));
    status = initWifi();
  }

  /* If we fail again, go SoftAP or reboot */
  if (status != WL_CONNECTED) {
    LOG_PORT.println(F("**** FAILED TO ASSOCIATE WITH AP, GOING SOFTAP ****"));
    WiFi.mode(WIFI_AP);
    String ssid = "ESP8266 " + String(chipId);
    status = WiFi.softAP(ssid.c_str());
  }

  if (status != WL_CONNECTED){
    LOG_PORT.println(F("**** FAILED TO ASSOCIATE WITH AP, REBOOTING ****"));
    //ESP.restart();
  }

  /* Configure and start the web server */
  initWeb();

  /* Setup mDNS / DNS-SD */
  MDNS.setInstanceName(String(chipId));
  if (MDNS.begin(hostname.c_str())) {
    MDNS.addService("http", "tcp", HTTP_PORT);
    MDNS.addService("e131", "udp", UDP_PORT);
    MDNS.addServiceTxt("e131", "udp", "CID", String(chipId));
  } else {
    LOG_PORT.println(F("*** Error setting up mDNS responder ***"));
  }
}

int initWifi() {
  /* Switch to station mode and disconnect just in case */
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(secureRandom(100, 500));

  LOG_PORT.println("");
  LOG_PORT.print(F("Connecting to "));
  LOG_PORT.println(ssid);

  WiFi.begin(ssid, password);
//  if (config.dhcp) {
//    LOG_PORT.print(F("Connecting with DHCP"));
//  } else {
//    /* We don't use DNS, so just set it to our gateway */
//    WiFi.config(IPAddress(config.ip[0], config.ip[1], config.ip[2], config.ip[3]),
//                IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3]),
//                IPAddress(config.netmask[0], config.netmask[1], config.netmask[2], config.netmask[3]),
//                IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3])
//               );
//    LOG_PORT.print(F("Connecting with Static IP"));
//  }

  uint32_t timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LOG_PORT.print(".");
    if (millis() - timeout > CONNECT_TIMEOUT) {
      LOG_PORT.println("");
      LOG_PORT.println(F("*** Failed to connect ***"));
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    LOG_PORT.println("");
    LOG_PORT.print(F("Connected with IP: "));
    LOG_PORT.println(WiFi.localIP());
  }

  return WiFi.status();
}

/* Configure and start the web server */
void initWeb() {

  web.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(LED_PIN, 1);
    request->send(200, "text/plain", "Hello from esp8266!");
    digitalWrite(LED_PIN, 0);
  });
  
  web.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
  
  web.on("/mac", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", getMacAddress());
  });

  /* Firmware upload handler */
  web.on("/update-firmware", HTTP_POST, [](AsyncWebServerRequest *request) {
      
  }, handleFirmwareUpdate);
    
  web.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Page not found");
  });

  web.begin();

  LOG_PORT.print(F("- Web Server started on port "));
  LOG_PORT.println(HTTP_PORT);

  udp.begin(UDP_PORT);
  
  // Start OTA server.
  ArduinoOTA.begin();
}

void loop(void) {
  // Handle OTA server.
  ArduinoOTA.handle();
  
  // if there's data available, read a packet  
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print(F("Packet size: "));
    Serial.println(packetSize);
    
    /*Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());*/
    
    // read the packet into packetBufffer
    udp.read(strip.getPixels(), packetSize);
  
    show(strip.getPixels(), packetSize);
  
    // send a reply to the IP address and port that sent us the packet we received
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(ReplyBuffer);
    udp.endPacket();
  }
}

void show(byte* buffer, byte length) {
  if (length == 3) {
    for(uint16_t i = 1; i < strip.numPixels(); i++) {
      buffer[3*i + 0] = buffer[0];
      buffer[3*i + 1] = buffer[1];
      buffer[3*i + 2] = buffer[2];
    }
  }
  
  strip.show(); 
}

OTAUpdate otaupdate;

void handleFirmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
        WiFiUDP::stopAll();
        LOG_PORT.print(F("* Upload Started: "));
        LOG_PORT.println(filename.c_str());
        otaupdate.begin();
    }

    if (!otaupdate.process(data, len)) {
        LOG_PORT.print(F("*** UPDATE ERROR: "));
        LOG_PORT.println(String(otaupdate.getError()));
    }

    if (otaupdate.hasError())
        request->send(200, "text/plain", "Update Error: " + String(otaupdate.getError()));

    if (final) {
        LOG_PORT.println(F("* Upload Finished."));
        otaupdate.end();
        //reboot = true;
    }
}

/* Plain text friendly MAC */
String getMacAddress() {
    uint8_t mac[6];
    char macStr[18] = {0};
    WiFi.macAddress(mac);
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return  String(macStr);
}
