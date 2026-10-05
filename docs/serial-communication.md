# Serial Communication Protocol & Specification

## 1. Physical & Link Layer Parameters

Communication between the Arduino UNO microcontroller and the Linux host system operates over a virtual USB serial channel configured as follows:

| Parameter | Specification | Rationale |
|:---|:---|:---|
| **Baud Rate** | 9600 bps | Standard, robust transmission rate for 8-bit AVR microcontrollers |
| **Data Bits** | 8 | Standard byte width |
| **Parity** | None | Low overhead, reliable transmission over short virtual link |
| **Stop Bits** | 1 | Standard framing |
| **Flow Control**| None | Software/hardware flow control is disabled for simplicity |
| **Transmission Interval** | 2000 ms | Balances sensor thermal response and Linux CPU consumption |

---

## 2. Packet Framing & Payload Syntax

The system natively supports two formats:
1. **Tinkercad Stream Protocol (Primary)**: Human-readable, pipe-delimited key-value frame emitted directly by the Arduino simulation firmware.
2. **Standard CSV Protocol (Alternative)**: Compact comma-separated fields.

### 2.1 Tinkercad Stream Format (Primary)
```text
Light: <light> | Temperature: <temp> C | Soil Moisture: <soil> | Humidity: <hum> | Water Level: <water>\n
```

- **Nominal Reading**:
  ```text
  Light: 650 | Temperature: 26.40 C | Soil Moisture: 520 | Humidity: 580 | Water Level: 610
  ```

- **Alert Reading (Low soil, dry humidity, high temp)**:
  ```text
  Light: 300 | Temperature: 32.50 C | Soil Moisture: 250 | Humidity: 350 | Water Level: 380
  ```

### 2.2 Standard CSV Format (Alternative)
```text
<temperature>,<humidity>,<soilMoisture>,<waterLevel>,<light>,<status>\n
```

| Field Index | Field Name | Data Type | Units / Range | Example |
|:---:|:---|:---:|:---:|:---:|
| 0 | `temperature` | Float (1 dec.) | °C (-40.0 to 125.0) | `28.5` |
| 1 | `humidity` | Float | 0 to 1023 (raw) / % | `580.0` |
| 2 | `soilMoisture`| Float | 0 to 1023 (raw) / % | `520.0` |
| 3 | `waterLevel` | Float | 0 to 1023 (raw) / % | `610.0` |
| 4 | `light` | Float | 0 to 1023 (raw) / lux | `650.0` |
| 5 | `status` | String token | `NORMAL` or `ALERT` | `NORMAL` |

---

## 3. Protocol Design Choices

1. **Why Plain ASCII CSV Instead of JSON?**
   - **Minimal Memory Footprint**: The ATmega328P has only 2 KB of SRAM. JSON serialization libraries consume significant RAM, risking stack/heap collisions.
   - **Fast Tokenization**: Tokenizing 6 comma-delimited tokens in C++ takes microseconds with minimal memory allocation.
   - **Debuggability**: Raw frames are directly readable in any serial monitor (e.g., Tinkercad Serial Monitor, `picocom`, `minicom`, `cat /dev/ttyACM0`).

2. **Why Not Binary Packets?**
   - ASCII formatting avoids cross-architecture endianness discrepancies between AVR (little-endian 8-bit) and x86-64/ARM64 Linux hosts.
   - Eliminates complex binary framing (e.g., HDLC, COBS) while maintaining zero data loss at 9600 baud.
