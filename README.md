# USB-PD on the CH32V003

I'm trying to get USB-PD working on the CH32V003. It doesn't have a dedicated USB-PD module, but I only want to use the chip itself and passive components. I'm using the internal comparator for receiving and a simple voltage divider for transmitting. The timing sensitive part of the firmware is written in assembler, while the rest is written in C. 

The project is still in an early stage and WIP, but I was able to transmit and receive messages. 

## Setup

The development board can be found in the board directory. The board is a breakout board with the required passive components, a USB-C connector and a high voltage capable voltage regulator. 

The firmware can be built with the provided makefile. It is built upon [ch32v003fun](https://github.com/cnlohr/ch32v003fun) and the toolchain described there.

Build and run via `make flash`.