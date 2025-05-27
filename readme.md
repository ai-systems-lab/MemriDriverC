# MVM Driver Project

This project provides a driver for MVM (Memristor Vector-Matrix) operations on Raspberry Pi.

## Project Structure

- `src/` - Source code directory
  - `r595hc.c` - Shift register control implementation
  - `r595hc.h` - Shift register control header
  - `rpi_modes.c` - Main MVM operation modes implementation
  - `MVM_SPI.c` - SPI communication implementation
  - `MVM_SPI.h` - SPI communication header
- `test/` - Test programs
  - `test.c` - Example test program
- `lib/` - Compiled library output
- `bin/` - Compiled test programs output

## Dependencies

- wiringPi library
- Linux SPIdev interface

## Building

To build the project:

```bash
make all