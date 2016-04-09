#include <SoftwareSerial.h>
#include <CRC32.h>
#include <Adafruit_NeoPixel.h>

#define SSerialTxControl 5    // RS485 Direction control
#define SSerialRX        6    // Serial Receive pin
#define SSerialTX        7    // Serial Transmit pin

#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define PIXEL_PIN    10       // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 12

SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);
  
  Serial.begin(74880);
  RS485Serial.begin(74880);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  readSerial();
}

void readSerial()
{
  if (RS485Serial.available() > 0)
  {
    //String r = RS485Serial.readStringUntil('\n');
    //send(r);
    //return;
    byte length = RS485Serial.read();
    Serial.println(length);
    
    uint8_t buffer[length];
    byte read = RS485Serial.readBytes(buffer, length);
    Serial.println(read);
    Serial.println();
    
    //for(int i = 0; i < length; i++) 
    //{
    //  Serial.println(buffer[i]);
    //}
    
    byte crc1 = RS485Serial.read();
    byte crc2 = RS485Serial.read();
    byte crc3 = RS485Serial.read();
    byte crc4 = RS485Serial.read();
    uint32_t sendCrc =  ((uint32_t)crc4 << 24) |
                        ((uint32_t)crc3 << 16) |
                        ((uint32_t)crc2 <<  8) |
                        crc1;
                        
    uint32_t calculatedCrc = CRC32::checksum(buffer, read);
    
    Serial.println(sendCrc);
    Serial.println(calculatedCrc);
    Serial.println();
    
    bool ok = sendCrc == calculatedCrc;

    if (ok) {
      show(buffer, read);
    }
    
    send(ok ? "OK" : "FAIL");
  }
}

void send(String value)
{
  digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit
   
  RS485Serial.println(value);
  RS485Serial.flush();
  
  digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit
}

void show(byte* buffer, byte length) {
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

// Display a single color on the whole string
void showColor(uint32_t c) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  
  strip.show();
}

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}
