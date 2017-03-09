#include <SPI.h>                  // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>          // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 3
#define NEOPIXEL_PIXELS 310
#define UDP_TX_PACKET_MAX_SIZE NEOPIXEL_PIXELS*3
#define UDP_PORT 8888

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {   0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 100);

// buffers for receiving and sending data
char ReplyBuffer[] = "OK";                  // a string to send back

EthernetUDP udp; // An EthernetUDP instance to let us send and receive packets over UDP

void setup() {

  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  
  Serial.begin(250000);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.print("Strip initialized with ");
  Serial.print(NEOPIXEL_PIXELS);
  Serial.println(" pixels.");
    
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  } else {
    // print your local IP address:
    printIPAddress();
  }
    
  udp.begin(UDP_PORT);
  
}

int freeRam ()
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void printIPAddress()
{
  Serial.print(F("My IP address: "));
  
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    if (thisByte) Serial.print(".");
    Serial.print(Ethernet.localIP()[thisByte], DEC);
  }

  Serial.println();
}

void loop() {
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
    if (packetSize > UDP_TX_PACKET_MAX_SIZE){
      packetSize = UDP_TX_PACKET_MAX_SIZE;
    }
    
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
