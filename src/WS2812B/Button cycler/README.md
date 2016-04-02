# Button cycler

Sketch with predefined list of animations.
Press button to change animation.
Hold down the button (for 1 second) to turn off animation.

### Neopixel

Example usage of "Adafruit NeoPixel" library. This is modified "buttoncycler" example with added possibility to turning off the animation when you hold the button.

Adafruit NeoPixel uses 3 bytes of RAM for each pixel (one led diode) - this techique is named "buffering". Because of that, NeoPixel library can't drive a lot of pixels with Attiny chips (They have at most 512 bytes of RAM - it's about 110 pixels in my tests with Attiny 85, which also have 512 bytes of RAM). But this is only disadvantage of buffering. It has a few advantages: animation is more predictable (without buffering sometimes it can have some defects like unnecessarily blinking or something similiar), it can work on internal oscillators (from 8 Mhz) and animation is still flawless - microcontroller can do other work and this does not affect the animation.

### Little ram

This is modified example of [this code](https://github.com/bigjosh/SimpleNeoPixelDemo). It does not use buffering. Animation is sent directly to the led strip. 

It needs 16 Mhz oscillator or faster. Code was tested on Attiny 85 with external 16 Mhz oscillator.