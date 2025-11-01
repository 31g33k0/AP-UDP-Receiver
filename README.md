# AP-UDP-Receiver

![AP-UDP-Receiver Diagram](AP-UDP-Receiver.png)

A simple UDP server that runs on an ESP32/ESP8266, creating its own WiFi Access Point to receive and log UDP packets.

## Features

- Creates a WiFi Access Point (AP) for direct device connection
- Listens for UDP packets on a configurable port (default: 12345)
- Logs received packets with timestamp and sender information
- Handles non-printable characters in received data
- Automatic restart on critical failures
- Configurable network settings

## Hardware Requirements

- ESP32 or ESP8266 board
- USB cable for programming and power
- H bridge or motor driver
- two motors
- power supply for the motors
- breadboard and jumper wires or equivalent
- optional: oscilloscope, logic analyser, voltmeter, ...


## Pinout

The following GPIO pins are used in this project for motor control:

|Hbridge Pin| esp32 Pin | Function      | Direction|Description    
|----|-----|---------------|-----------|------------------------------------|
|TODO| 25  | ctrlPinX      | Output    | X-axis control signal (PWM or DAC) |
|TODO| 26  | ctrlPinY      | Output    | Y-axis control signal (PWM or DAC) |
|TODO| 32  | inPinY0       | Output    | Y-axis direction                   |
|TODO| 33  | inPinX0       | Output    | X-axis direction                   |
|TODO| 34  | inPinY1       | Output    | Y-axis direction                   |
|TODO| 35  | inPinX1       | Output    | X-axis direction                   |
|TODO| VCC | 5V Power      | -         | Power for ESP32 and Hbridge logic  |
|TODO| GND | Ground        | -         | Common ground for all components   |


### Notes:
- Pins 25 and 26 can be used for PWM/DAC output for speed control
- X-axis direction is controlled by inPinX0 and inPinX1
- Y-axis direction is controlled by inPinY0 and inPinY1
- Ensure proper H-bridge or motor driver is used to prevent short circuits


## Installation

1. Clone this repository
2. Open the project in Arduino IDE or PlatformIO
3. Install required libraries (if needed)
4. Upload the sketch to your board
5. Open the Serial Monitor (115200 baud) to view logs

## Configuration

Edit the following constants in `AP-UDP-Receiver.ino` to customize the behavior:

```cpp
const char* SSID = "AP-UDP-Receiver";    // Access Point SSID
const char* PASSWORD = "12345678";       // Password (min 8 chars)
const uint16_t LOCAL_UDP_PORT = 12345;   // UDP port to listen on
const size_t BUFFER_SIZE = 512;          // Size of receive buffer
const uint8_t MAX_CONNECTIONS = 1;       // Maximum number of connected clients
```

## Usage

1. After uploading the sketch, the device will create a WiFi network with the configured SSID
2. Connect a device (for example the UDP-Joystick https://github.com/31g33k0/UDP-Joystick) to this network
3. Send UDP packets to the AP's IP (default: 192.168.4.1) on the configured port
4. View received packets in the Serial Monitor
5. The received packets will be used to control the motors

## Example

To send a test message from a Linux/macOS terminal:

```bash
echo "314159" | nc -u 192.168.4.1 12345
```

```bash
echo "Hello, ESP32!" | nc -u 192.168.4.1 12345
```

## License

This project has no license.

## Remarks

I'll use an H bridge to control the X and Y axis.
The candidates are self made H bridge or L298N or L293D. Suggestions are welcome.

TODO add a timer to avoid short circuiting the H bridge if necessary