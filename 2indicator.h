/**
    Indicator routines

    @author Boris Volkovyski
    @version 1.0 8/03/2021 
*/

#include <Arduino.h>

// https://github.com/jasonacox/TM1637TinyDisplay
#include <TM1637TinyDisplay.h>

// Initialize TM1637TinyDisplay
TM1637TinyDisplay display(CLK_PIN, DIO_PIN);

// Animations created with Tool https://jasonacox.github.io/TM1637TinyDisplay/examples/7-segment-animator.html
const uint8_t ANIMATION[13][4] PROGMEM = {
{ 0x00, 0x00, 0x00, 0x00 },  // Frame 0
{ 0x00, 0x40, 0x40, 0x00 },  // Frame 1
{ 0x40, 0x40, 0x40, 0x40 },  // Frame 2
{ 0x09, 0x09, 0x09, 0x09 },  // Frame 3
{ 0x09, 0x0f, 0x39, 0x09 },  // Frame 4
{ 0x09, 0x39, 0x0f, 0x09 },  // Frame 5
{ 0x0f, 0x09, 0x09, 0x39 },  // Frame 6
{ 0x39, 0x09, 0x09, 0x0f },  // Frame 7
{ 0x39, 0x00, 0x00, 0x0f },  // Frame 8
{ 0x30, 0x00, 0x00, 0x06 },  // Frame 9
{ 0x40, 0x00, 0x00, 0x40 },  // Frame 10
{ 0x00, 0x40, 0x40, 0x00 },  // Frame 11
{ 0x00, 0x00, 0x00, 0x00 }  // Frame 12
};

/**
 * Displays welcome animation
 * 
 * @return void
 */
void show_welcome_animation() {
  display.setBrightness(BRIGHT_7);
  
  display.clear();
  
  display.showAnimation_P(ANIMATION, FRAMES(ANIMATION), TIME_MS(100));
  
  display.showString("HELLO");
  
  delay(500);
  
  display.clear();
}

/**
 * Displays current mode and program
 * 
 * @param mode
 * @param glitter
 * 
 * @return void
 */
void show_status(uint8_t mode, bool glitter) {
  display.showNumber(mode);
  
#if LOG_ON == 1
  Serial.println(F("--- indicator ---"));
  Serial.print(F("Mode: ")); Serial.println(mode);
  Serial.print(F("Glitter: ")); Serial.println(glitter);
  Serial.println(F("--- indicator ---"));
#endif
}
