# aros-micropython

[Micropython](https://github.com/micropython/micropython) port to AROS with support for Rasberry Pi hardware peripherals (GPIO, UART, I2C, SPI, PWM).
This is based on the standalone port to Rasberry Pi here : [micropython-raspberrypi](https://github.com/boochow/micropython-raspberrypi/)

Warning : This is a work in progress and only partly tested (GPIO, UART confirmed working).

## How to build
```
wget https://github.com/micropython/micropython/archive/refs/tags/v1.29.0.tar.gz
tar -xaf v1.29.0.tar.gz
git clone https://github.com/tenko/aros-micropython.git
cd aros-micropython
make
```
The build was performed with help of [aros-compiler-docker](https://github.com/BlitterStudio/aros-compiler-docker)
and tested on Pi400 and Rasberry Pi4.
