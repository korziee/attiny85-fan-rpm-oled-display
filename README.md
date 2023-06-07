# Displaying fan RPM on an OLED using an ATtiny85

![IMG_7049](https://github.com/korziee/attiny85-fan-rpm-oled-display/assets/22607911/ff2b9151-4103-4ea4-8d0f-3252b90be812)

## Demo Video

https://github.com/korziee/attiny85-fan-rpm-oled-display/assets/22607911/2d447892-6852-46a2-81ec-11496557a0af

## Schematic

![schematic-v0.1](./assets/schematic-v0.1.svg)

## Requirements

1. ATtiny85 development board (the one with USB + 5V regulator), [like this](https://www.ebay.com.au/itm/191572154213).
2. Arduino 2 IDE (think this comes pre-installed with the boards manager, you could use the original IDE but you'll need to add ATTinyCore to the boards manager, [like this](https://github.com/SpenceKonde/ATTinyCore/blob/v2.0.0-devThis-is-the-head-submit-PRs-against-this/Installation.md))
3. 3 pin DC Fan
   1. I've used an old [PL80S12M](https://www.elecok.com/power-logic-pl80s12m-12v-0-14a-3wires-cooling-fan.html). It activates down to 5v so I could test in circuit without an external supply.
4. 128x64 i2c OLED
   1. I've used [this one](https://core-electronics.com.au/dual-colour-oled-display-128x64-yellow-and-blue.html)

## Note on the Code

Most of the code is self-documenting, however interrupts on the ATtiny85 are not like other AVR boards and you cannot use `attachInterrupt` directly like you would with an Arduino Uno - you instead need to setup the registers explicitly. The three registers we care about are: `PCMSK`, `GIMSK` and `MCUCR`. Once you've set the registers, you need to enable interrupts globally by calling the `sei` function.

### PCMSK - Pin Change Mask Register

The PCMSK register configures **which** pins (PB-0 through to PB-5) will trigger an interrupt.

### GIMSK - General Interrupt Mask Register

We only care about the 5th bit in this register, which is the PCIE (Pin Change Interrupt Enable) pin. In the PCMSK register we configured **which** pins triggered an interrupt, and by setting PCIE to be `1`, we enabled that behaviour.

### MCUCR - MCU Control Register

Bits 7:2 control assorted properties of the MCU, bits 1 and 0 however serve as the interrupt sense control for the ATtiny85. This controls the condition you want the interrupt to be triggered (rising, falling, low, any) on.

| Bit 1 (ISC01) | Bit 0 (ISC00) | Description                                                |
| ------------- | ------------- | ---------------------------------------------------------- |
| 0             | 0             | The low level of INT0 generates an interrupt request.      |
| 0             | 1             | Any logical change on INT0 generates an interrupt request. |
| 1             | 0             | The falling edge of INT0 generates an interrupt request.   |
| 1             | 1             | The rising edge of INT0 generates an interrupt request.    |

## Note on Interrupt Pins

The ATtiny85 development boards have a handful of components attached to some of the GPIO pins to enable sub programming and to provide a regulated 5V supply, this means that not all pins are suitable to be used an interrupt. I could not get pin 4 to operate in a stable manner as it is part of the USB circuitry. Pin 5 is not connected to anything, but I was unable to get an interrupt to trigger on this pin, unsure why - think it has something to do with that pin being used as the reset. Pins 0 and 2 are ideal candidates for interrupt triggers, but these are used for i2c (SDA and SCL respectively), so I could not use them. I had moderate success with pin 3 even though it's connected to a USB data line. I think this is because unlike pin 4, pin 3 in the development board is tied to 5V. So it is mostly stable, but occasionally some data on the USB line causes it to freak out a bit and the RPM measurement is incorrect - maybe if this was connected to a USB power supply (no data) then it would not be an issue. Because pin 3 is used in the usb circuitry, **IT NEEDS TO BE UNPLUGGED BEFORE** programming, otherwise it'll just hang.

## References

- http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf
- https://www.openimpulse.com/blog/wp-content/uploads/wpsc/downloadables/DIGISPARK-PRODUCTION-REV3-Schematic.pdf
- [ATTinyCore to add TinyAVR support to Arduino](https://github.com/SpenceKonde/ATTinyCore)
- [OLED ASCII Library](https://github.com/greiman/SSD1306Ascii)
