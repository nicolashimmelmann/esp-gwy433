# ESP_GWY433

Brematic GWY433 compatible implementation for the ESP8266

Brennenstuhl sells a gateway operating at 433MHz, which is used to switch on and off "Smart Power Sockets" and other stuff operating at that frequency. The point is, the device costs around 60$ for the simple task of converting the protocol string into a 433MHz signal. Thus, I decided to implement my own version of the device on an ESP8266, which effectively identifies as such a Gateway in the network.

## How to run

1. Enter your WiFi credentials in the `include/WifiCredentials.h` file.
1. Build and flash the project using PlatformIO
1. Connect your 433MHz sender module to the ESP as follows (Wemos D1 mini board in my case):
   | Wemos D1 mini pin | 433 MHz sender pin |
   |-------------------|--------------------|
   | 5V                | VCC                |
   | GND               | GND                |
   | D1                | DATA               |

## Basic operation

- The Gateway listens on UDP port 49880 for incoming messages.
- If a message is received which contains the payload `SEARCH HCGW`, the gateway answers with the following information: ``` HCGW:VC:Brennenstuhl;MC:0290217;FW:V016;IP:192.168.xxx.yyy;;````
- If a message is received which contains a protocol sequence, such as `TXP:0,0,6,10000,314,66,18,18,2,2,1,1,2,2,1,1,2,1,2,1,2,2,1,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,1,2,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,1,1,2,2,1,2,1,1,2,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1,2,2,1,2,1,2,1,2,1,2,1,1,2,1,2,2,1,1,2,2,1,2;`, it simply sends out the signal on 433MHz.

## The protocol

The format of a protocol message follows this specification:

| Value    | Explanation                                                 |
| -------- | ----------------------------------------------------------- |
| TXP:0,0, |                                                             |
| 6        | How often to send the same packet (repeat count)            |
| 10000    | How long to pause between the repetitions (in microseconds) |
| 314      | The duration of the shortest pulse (pulse duration)         |
| 66       | How many signal changes are being sent, divided by 2        |

All numbers following are multipliers for the shortest pulse/pause duration (starting with a pulse, then alternating between pulse and pause). A sequence of `1,2,1` thus results in pulse 314, pause 628, pulse 314 and so on.

This Amplitude Shift Keying protocol (Pulse Distance Width) is commonly used by many different vendors for their 433MHz devices (power sockets, temperature sensors, roller blinds, movie screens, ...).
