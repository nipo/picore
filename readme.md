# Pico-RE: Pico Runtime Environment

This is an addon to The Raspberry Pi Pico SDK which provides some
runtime environment.

It provides a cooperative scheduling model and some communication
libaries.

# Main utilities

## Tasks

Everything is centered around a task model. Tasks are scheduled in a
queue where they are services in a round-robin manner.  Tasks may be
scheduled ASAP or after a timeout.

## FIFO

A fifo is a ring buffer with producer and consumer ends.  Pico-RE
fifos can schedule tasks every time some data gets available in the
fifo, or every time some room is available in the fifo.  This allows
producers and consumers to be waken up selectively.

## Queue

A queue is a ring buffer of pointers where producer and consumer may
exchange arbitrart structures.

## I2C abstraction

I2C abstraction gives a context for bus and slave. On top of this are
modeled EEPROM access methods with abstract interface.

## Hardware integration

Library gives some integration with Pico-SDK APIs, including:

- TinyUSB service call wrapped as a task,

- UART driver that converts to FIFOs,

- TinyUSB CDC interface that converts to FIFOs,

- stdio binding from/to FIFOs.

# Example code

See [examples](test/) for example code you can build.

# Quick-start your own project

You can add Pico-RE to your project similarly to the SDK (copying
[external/picore_import.cmake](external/picore_import.cmake) into your
project) having set the `PICORE_PATH` variable in your environment or
via cmake variable.

```cmake
cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO RE
include(picore_import.cmake)

project(pico_playground C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
``` 
