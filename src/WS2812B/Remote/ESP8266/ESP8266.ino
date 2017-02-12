#include <Arduino_Base64.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>

const char* ssid = "Dom";
const char* password = "internet24";

#define LED_PIN       13

#define PIXEL_PIN     14       // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT   12

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer server(80);

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Display a single color on the whole string
void showColor(uint32_t c) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  
  strip.show();
}

void show(byte* buffer, byte length) {
  Serial.print("Received bytes: ");
  Serial.println(length);
  
  if (length == 3) {
    uint32_t c = color(buffer[0], buffer[1], buffer[2]);
  
    showColor(c);
  }
  else {
    uint16_t pixelLength = length / 3;
    for(uint16_t i = 0; i < strip.numPixels() && i < pixelLength; i++) {
      strip.setPixelColor(i, color(buffer[3 * i], buffer[3 * i + 1], buffer[3 * i + 2]));
    }
  
    strip.show();
  }
}

void handleRoot() {
  digitalWrite(LED_PIN, 1);
  server.send(200, "text/plain", "Hello from esp8266!");
  digitalWrite(LED_PIN, 0);
}

void handleLedStrip() {
//  Serial.print("Args: ");
//  Serial.println(server.args());
//
//  Serial.print("Content-Length: ");
//  Serial.println(server.header("Content-Length"));
//
//  Serial.println(server.argName(0));
  
  String colorsBase64String = server.arg(0);

  int decodedLength = base64_dec_len((char*)colorsBase64String.c_str(), colorsBase64String.length());
  byte decoded[decodedLength];
  
  base64_decode((char*)decoded, (char*)colorsBase64String.c_str(), colorsBase64String.length());
  
//  Serial.print("Received string length: ");
//  Serial.println(decodedLength);
  
  show(decoded, decodedLength);

  server.send(204);
}

void setup(void){
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/led-strip", handleLedStrip);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
