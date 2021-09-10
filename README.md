# Smart LED

AVR board to control home lighting autonomously.
It provides automatic control of LED lighting (12vdc) according to:

* Time schedule: active during daily work time except holidays.
* People movement: turning on lighting automatically when sensing movement using a PIR extension.
* Electric supply status: provides auxiliary lighting when needed (in case of power outage).
* Illumination level sensing: no need to turn on the lighting when there is enough lightning.

Also providing manual control:

* Directly using on-board buttons.
* Remotely using infrared remote control (RC5 based).

![Cover](https://raw.githubusercontent.com/syk-yaman/smart-led/master/PCB/cover.jpg)

# Specifications

**Inputs:**

- 12v DC power supply.
- 220v AC (power detection).

**Output:**

- 12v DC power supply for LEDs.

**Schematic**:

![Schematic layout](https://raw.githubusercontent.com/syk-yaman/smart-led/master/PCB/Schematic.png)

**Main parts:**

- ATmega 8A
- DS1307 RTC
- PIR module ([link](https://www.adafruit.com/product/189))
- 5v DC Relay
- Infrared receiver

See [BOM](https://raw.githubusercontent.com/syk-yaman/smart-led/master/PCB/BOM.txt) for further details.

# Development

Required tools:

* Atmel Studio
* Autodesk Eagle
* A brain

**BE AWARE OF ELECTRIC SHOCK, 220V POWER SUPPLY SHOULD BE TAKEN SERIOUSLY. PLEASE TAKE SAFETY MEASURES FIRST**



## License

###### This project is under MIT License

```
MIT License

Copyright (c) [year] [fullname]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```