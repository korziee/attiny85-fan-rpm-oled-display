#include <Wire.h>
#include <avr/io.h>
#include "avr/interrupt.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

const int INTERRUPT_PIN = PCINT3;

volatile uint32_t rising_edge_counter = 0;
volatile uint32_t last_rec_period_time = 0;
volatile uint32_t period_start_time = millis();

float previous_rpm = 0.0;

SSD1306AsciiWire oled;

void setup()
{
  Wire.begin();
  Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);

  oled.clear();
  oled.set2X();
  oled.println("RPM Reader");
  oled.println("RPM:");
  oled.set1X();
  oled.println();
  oled.println();
  oled.println();
  oled.print("By Kory Porter");
  oled.set2X();

  cli();                         // clear any interrupts
  PCMSK |= (1 << INTERRUPT_PIN); // enable interrupts only on interrupt pin
  MCUCR |= (1 << 3);             // configure only to trigger interrupts on rising edge
  GIMSK |= (1 << PCIE);          // enable pin change interrupts
  pinMode(INTERRUPT_PIN, INPUT); // has a physical pull up on breadboard
  sei();                         // enable global interrupts
}

void loop()
{
  // getting fontWidth here as 1 character does not equal 1 pixel
  // so fontWidth * amount of columns to move along
  oled.setCursor(oled.fontWidth() * 5, 2);

  // if it's been over a second since our last update, fan is stalled
  // if last_rec is 0, we haven't startedvv
  if ((millis() - period_start_time) > 1000 || last_rec_period_time == 0)
  {
    oled.clearToEOL();
    oled.print(0);
  }
  else
  {
    float rpm = 60.0 / ((2 * last_rec_period_time) / 1000.0);

    // only print if rpm has changed
    if (rpm != previous_rpm)
    {
      previous_rpm = rpm;
      oled.clearToEOL();
      oled.print(int(rpm));
    }
  }

  delay(500);
}

// "ISR" is a magic interrupt function for ATtiny85 (maybe all the x5 but not sure)
// Any interrupts on registered pins will trigger this ISR, I'm sure you
// could check registers to see which pin triggered the interrupt but
// I've only registered PB3
ISR(PCINT0_vect)
{
  // if we've already seen a rising edge, we've done a full period
  // so take note of timing, and reset
  if (rising_edge_counter == 1)
  {
    uint32_t now = millis();
    last_rec_period_time = now - period_start_time;
    period_start_time = now;
    rising_edge_counter = 0;
  }
  else
  {
    rising_edge_counter += 1;
  }
}