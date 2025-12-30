# ESP32-SerialSniffer

A lightweight, real-time WiFi packet monitor for the ESP32 that displays captured 802.11 frames in a clean, tabular format **directly in the Arduino Serial Monitor**.

## Features

- **Human-readable output** with columns:  
  `No. | Time | Source MAC | Destination MAC | Protocol | Length | Info`  
  (Info includes frame type, RSSI, channel, and SSID)
- Captures common frames: Beacons, Probe Requests/Responses, Data, ACKs, Block Acks, Deauth, etc.
- **Channel hopping** across channels 1–11 (configurable) to see activity on all 2.4 GHz channels
- Displays **SSID** (shows `<hidden>` for hidden networks, prints visible characters safely)
- Shows **signal strength (RSSI)** and current channel for every packet
- Fully self-contained — no external libraries needed
- Works on any ESP32 with 2.4 GHz WiFi (WROOM-32, WROOM-32U, S2, S3, C3, etc.)

## Example Output

|No.|Time      |Source   	        |   Destination  	|  Protocol | Length    |   Info	                        |
|---|---	   |---	                |---	            |---	    |---	    |---	                            |
|  1|0.000000  |AA:BB:CC:11:22:33	|FF:FF:FF:FF:FF:FF  |802.11   	|280   	    |Beacon frame, RSSI=-54 dBm, Ch=6, SSID=MyHomeNetwork|
|  2|0.102400  |DD:EE:FF:44:55:66 	|FF:FF:FF:FF:FF:FF  |802.11   	|273  	    |Beacon frame, RSSI=-72 dBm, Ch=1, SSID=<hidden>   	|
|  3|0.215300  |11:22:33:AA:BB:CC   |AA:BB:CC:11:22:33  |802.11   	|30   	    |ACK frame, RSSI=-45 dBm, Ch=6   	|

## Hardware Requirements

- Any ESP32 development board with 2.4 GHz WiFi (e.g., ESP32 DevKitC, WROOM-32, WROOM-32U)
- USB cable for programming and serial output

## Software Requirements

- Arduino IDE (or PlatformIO)
- ESP32 board package installed (https://github.com/espressif/arduino-esp32)

## Installation & Usage

1. Open the Arduino IDE.
2. Copy the code from `ESP32-SerialSniffer.ino` into a new sketch.
3. Select your board (e.g., **ESP32 Dev Module**).
4. Set the Serial Monitor baud rate to **115200**.
5. Upload the sketch.

## Customization

Edit these defines at the top of the sketch:

```cpp
#define BAUD_RATE       115200     // Serial speed
#define CHANNEL_HOPPING true       // false = stay on one channel
#define MAX_CHANNEL     11         // 11 (US), up to 13/14 in some regions
#define HOP_INTERVAL    214        // milliseconds per channel (lower = faster hopping)
```
## License

MIT License — feel free to use, modify, and share!