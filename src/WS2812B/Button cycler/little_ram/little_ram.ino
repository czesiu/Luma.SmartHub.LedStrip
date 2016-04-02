// ############################################# START LIBRARY ###################################################################
/*
 This is an example of how simple driving a Neopixel can be
 This code is optimized for understandability and changability rather than raw speed
 More info at http://wp.josh.com/2014/05/11/ws2812-neopixels-made-easy/
*/

// Change this to be at least as long as your pixel string (too long will work fine, just be a little slower)

// These values depend on which pin your string is connected to and what board you are using 
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation

// These values are for digital pin 8 on an Arduino Yun or digital pin 12 on a DueMilinove/UNO
// Note that you could also include the DigitalWriteFast header file to not need to to this lookup.

#define PIXEL_PORT  PORTB  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRB   // Port of the pin the pixels are connected to
#define PIXEL_BIT   0      // Bit of the pin the pixels are connected to

// These are the timing constraints taken mostly from the WS2812 datasheets 
// These are chosen to be conservative and avoid problems rather than for maximum throughput 

#define T1H  650    // Width of a 1 bit in ns
#define T1L  300    // Width of a 1 bit in ns

#define T0H  250    // Width of a 0 bit in ns
#define T0L  700    // Width of a 0 bit in ns

#define RES 6000    // Width of the low gap between bits to cause a frame to latch

// Here are some convience defines for using nanoseconds specs to generate actual CPU delays

#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives

#define CYCLES_PER_SEC (F_CPU)

#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

// Actually send a bit to the string. We must to drop to asm to enusre that the complier does
// not reorder things and make it so the delay happens in the wrong place.

void sendBit( bool bitVal ) {
  
    if (  bitVal ) {        // 0 bit
      
    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"                                // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),    // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      [offCycles]   "I" (NS_TO_CYCLES(T1L) - 2)     // Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

    );
                                  
    } else {          // 1 bit

    // **************************************************************************
    // This line is really the only tight goldilocks timing in the whole program!
    // **************************************************************************


    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"        // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
      "nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T0H) - 2),
      [offCycles] "I" (NS_TO_CYCLES(T0L) - 2)

    );
      
    }
    
    // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)
    // Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
    // This has thenice side effect of avoid glitches on very long strings becuase 

    
}  

  
void sendByte( unsigned char byte ) {
    
    for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {
      
      sendBit( bitRead( byte , 7 ) );                // Neopixel wants bit in highest-to-lowest order
                                                     // so send highest bit (bit #7 in an 8-bit byte since they start at 0)
      byte <<= 1;                                    // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
      
    }           
} 

/*

  The following three functions are the public API:
  
  ledSetup() - set up the pin that is connected to the string. Call once at the begining of the program.  
  sendPixel( r g , b ) - send a single pixel to the string. Call this once for each pixel in a frame.
  show() - show the recently sent pixel on the LEDs . Call once per frame. 
  
*/


// Set the specified pin up as digital out

void ledsetup() {
  bitSet( PIXEL_DDR , PIXEL_BIT );
}

void sendPixel( uint32_t c )  {
  uint8_t
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
              
  sendByte(g);            // Neopixel wants colors in green then red then blue order
  sendByte(r);
  sendByte(b);
}

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint32_t color(uint32_t c, uint8_t brightness) {
  uint8_t
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
      
  return color((r * brightness) / 256, (g * brightness) / 256 , (b * brightness) / 256);
}

// Just wait long enough without sending any bots to cause the pixels to latch and display the last sent frame

void show() {
  _delay_us( (RES / 1000UL) + 1);       // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}


/*

  That is the whole API. What follows are some demo functions rewriten from the AdaFruit strandtest code...
  
  https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino
  
  Note that we always turn off interrupts while we are sending pixels becuase an interupt
  could happen just when we were in the middle of somehting time sensitive.
  
  If we wanted to minimize the time interrupts were off, we could instead 
  could get away with only turning off interrupts just for the very brief moment 
  when we are actually sending a 0 bit (~1us), as long as we were sure that the total time 
  taken by any interrupts + the time in our pixel generation code never exceeded the reset time (5us).
  
*/

// ############################################# END LIBRARY ###################################################################

#define PIXELS 159        // Number of pixels in the string

#define BUTTON_PIN   1    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.
                          
bool oldState = HIGH;
long oldMillis = 0;
int showType = 8;
uint16_t j = 0;

void setup() {
  ledsetup();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN+1, OUTPUT);
  digitalWrite(BUTTON_PIN+1, LOW);
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
    case  0: colorWipe(color(0, 0, 0), 50);           // Black/off
             break;
    case  1: colorWipe(color(255, 0, 0), 50);         // Red
             break;
    case  2: colorWipe(color(0, 255, 0), 50);         // Green
             break;
    case  3: colorWipe(color(0, 0, 255), 50);         // Blue
             break;
    case  4: theaterChase(color(127, 127, 127), 50);  // White
             break;
    case  5: theaterChase(color(127,   0,   0), 50);  // Red
             break;
    case  6: theaterChase(color(  0,   0, 127), 50);  // Blue
             break;
    case  7: rainbow(20);
             break;
    case  8: rainbowCycleNew(20);
             break;
    case  9: theaterChaseRainbow(50);
             break;
    case 10: detonate(color(255, 255, 255));
             break;
    default: showType = 0;
             break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  cli();
  unsigned int p = 0;
  
  while (p++ <= j) {
      sendPixel(c);
  } 
   
  while (p++ <= PIXELS) {
      sendPixel(0);  
  }
  
  sei();
  
  show();
  delay(wait);
  
  j = (j + 1) % PIXELS;
}

void rainbow(uint8_t wait) {
  cli();
  for(uint16_t i = 0; i < PIXELS; i++) {
    sendPixel(wheel(i + j));
  }
  sei();
    
  show();
  delay(wait);
  
  j = (j + 1) % 256;
}


// Slightly different, this makes the rainbow equally distributed throughout
// It is not working, because division takes a lot of processor time ((i * 256 / strip.numPixels()) + j).
void rainbowCycleOld(uint8_t wait) {
  cli();
  for(uint16_t i = 0; i < PIXELS; i++) {
    sendPixel(wheel((i * 256 / PIXELS) + j));
  }
  sei();
  
  show();
  delay(wait);

  j = (j + 1) % (256 * 5);
}

// Slightly different, this makes the rainbow equally distributed throughout
// I rewrite this one from scrtach to use high resolution for the color wheel to look nicer on a *much* bigger string
void rainbowCycleNew(uint8_t wait) {
  
  // Hue is a number between 0 and 3*256 than defines a mix of r->g->b where
  // hue of 0 = Full red
  // hue of 128 = 1/2 red and 1/2 green
  // hue of 256 = Full Green
  // hue of 384 = 1/2 green and 1/2 blue
  // ...
  
  uint16_t currentPixelHue = j;
     
  cli();
  for(uint16_t i = 0; i < PIXELS; i++) {
    
    if (currentPixelHue >= (3 * 256)) {                  // Normalize back down incase we incremented and overflowed
      currentPixelHue = 0;
    }
          
    uint8_t phase = currentPixelHue >> 8;
    uint8_t step = currentPixelHue & 0xff;
               
    switch (phase) {
      case 0: 
        sendPixel(color(~step, step, 0));
        break;
        
      case 1: 
        sendPixel(color(0, ~step, step));
        break;

      case 2: 
        sendPixel(color(step, 0, ~step));
        break;
    }
    
    currentPixelHue += 5;      
  } 
  sei();
  
  show();
  delay(wait);

  j = (j + 20) % (3 * 256);
}

#define THEATER_SPACING (PIXELS / 20)

// Theatre-style crawling lights.
// Changes spacing to be dynmaic based on string size
void theaterChase(uint32_t c, uint8_t wait) {
  for (uint16_t q = 0; q < THEATER_SPACING; q++) {
    uint16_t step = 0;
    
    cli();
    for (uint16_t i = 0; i < PIXELS; i++) {
      if (step == q) {
        sendPixel(c);
      } else {
        sendPixel(0);
      }
      
      step++;
      
      if (step == THEATER_SPACING) {
        step = 0;
      }
    }
    
    sei();
    
    show();
    delay(wait);
  }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (uint16_t q = 0; q < THEATER_SPACING; q++) {
    uint16_t step = 0;
    
    cli();
    for (uint16_t i = 0; i < PIXELS; i++) {
      if (step == q) {
        sendPixel(wheel(i + j));
      } else {
        sendPixel(0);
      }
      
      step++;
      
      if (step == THEATER_SPACING) {
        step = 0;
      }
    }
    sei();
    
    show();
    delay(wait);
  }

  j = (j + 1) % 256;
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
  cli();
  for(uint16_t i = 0; i < PIXELS; i++) {
    sendPixel(c);
  }
  sei();
  show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
inline uint32_t wheel(uint8_t position) {
  uint16_t currentPixelHue = position << 2;    // Miltiply by 4, because by simple by 3 takes a lot of processor time
  
  if (currentPixelHue >= (3 * 256)) {                  // Normalize back down incase we incremented and overflowed
    currentPixelHue = 0;
  }
  
  uint8_t phase = currentPixelHue >> 8;
  uint8_t step = currentPixelHue & 0xff;
  
  switch (phase) {
    case 0: 
      return color(~step, step, 0);
      break;
      
    case 1: 
      return color(0, ~step, step);
      break;
  
    case 2: 
      return color(step, 0, ~step);
      break;
  }
      
  return 0;
}
