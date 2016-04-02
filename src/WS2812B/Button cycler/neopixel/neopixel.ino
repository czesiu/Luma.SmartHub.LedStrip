// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN   1    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    0    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 60

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = HIGH;
long oldMillis = 0;
int showType = 8;
uint16_t j = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN+1, OUTPUT);
  digitalWrite(BUTTON_PIN+1, LOW);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Get current button state.
  bool newState = digitalRead(BUTTON_PIN);
  
  // Check if state changed from high to low (button press).
  if (newState == LOW && oldState == HIGH) {
    // Check if button is still low after debounce.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {
      oldMillis = millis();
    }
  } else if (newState == LOW && oldState == LOW) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {
      long ms = millis() - oldMillis;
  
      if (ms > 1000 && showType != 0) {
        showType=0;
        oldMillis = 0;
      }
    }
  } else if (newState == HIGH && oldState == LOW) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == HIGH && oldMillis != 0) {
      j = 0;
      showType++;
    }
  } else {
    oldMillis = 0;
  }

  // Set the last button state to the old state.
  oldState = newState;

  startShow(showType);
}

void startShow(int i) {
  switch(i){
    case  0: colorWipe(strip.Color(0, 0, 0), 50);           // Black/off
             break;
    case  1: colorWipe(strip.Color(255, 0, 0), 50);         // Red
             break;
    case  2: colorWipe(strip.Color(0, 255, 0), 50);         // Green
             break;
    case  3: colorWipe(strip.Color(0, 0, 255), 50);         // Blue
             break;
    case  4: theaterChase(strip.Color(127, 127, 127), 50);  // White
             break;
    case  5: theaterChase(strip.Color(127,   0,   0), 50);  // Red
             break;
    case  6: theaterChase(strip.Color(  0,   0, 127), 50);  // Blue
             break;
    case  7: rainbow(20);
             break;
    case  8: rainbowCycle(20);
             break;
    case  9: theaterChaseRainbow(50);
             break;
    case 10: detonate(strip.Color(255 , 255 , 255));
             break;
    default: showType = 0;
             break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i< strip.numPixels(); i++) {
    if (i <= j) {
      strip.setPixelColor(i, c);
    }
    else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }
  strip.show();
  delay(wait);

  j = (j + 1) % strip.numPixels();
}

void rainbow(uint8_t wait) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, wheel((i + j) & 255));
  }
  strip.show();
  delay(wait);
  j = (j + 1) % 256;
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  for(uint16_t i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, wheel(((i * 256 / strip.numPixels()) + j) & 255));
  }
  strip.show();
  delay(wait);
  
  j = (j + 1) % (256*5);
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int q=0; q < 3; q++) {
    for (int i=0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, c);    //turn every third pixel on
    }
    strip.show();

    delay(wait);

    for (int i= 0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, 0);        //turn every third pixel off
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int q=0; q < 3; q++) {
    for (int i=0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, wheel( (i+j) % 255));    //turn every third pixel on
    }
    strip.show();

    delay(wait);

    for (int i=0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, 0);        //turn every third pixel off
    }
  }

  j = (j+1) % 256;
}


// I added this one just to demonstrate how quickly you can flash the string.
// Flashes get faster and faster until *boom* and fade to black.
void detonate(uint32_t c) {
  showColor(0);
    
  if (j == 0) {
    j = 1000;
  }
  else if (j > 1) {
    delay(j);
    
    showColor(c); // Flash the color 
    showColor(0);
    
    j =  (j * 4) / 5; // delay between flashes is halved each time until zero
  }
  else {
    // Then we fade to black....
    for (uint8_t fade = 255; fade > 0; fade--) {
      showColor(color(c, fade));
    }
    showColor(0);
      
    j = 1000;
  }
}

// Display a single color on the whole string
void showColor(uint32_t c) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  
  strip.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(uint8_t position) {
  position = 255 - position;
  
  if (position < 85) {
    return strip.Color(255 - position * 3, 0, position * 3);
  }
  
  if (position < 170) {
    position -= 85;
    return strip.Color(0, position * 3, 255 - position * 3);
  }
  
  position -= 170;
  return strip.Color(position * 3, 255 - position * 3, 0);
}


uint32_t color(uint32_t c, uint8_t brightness) {
  uint8_t
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
      
  return strip.Color((r * brightness) / 256, (g * brightness) / 256 , (b * brightness) / 256);
}
