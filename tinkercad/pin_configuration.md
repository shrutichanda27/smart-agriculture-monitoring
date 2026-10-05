# Arduino Pin Configuration & Sensor Specifications

## 1. Master Pin Allocation Matrix

The Arduino UNO R3 hardware configuration is defined as follows:

| Pin | Identifier | Mode | Device Connected | Function / Description |
|:---:|:---|:---:|:---|:---|
| **A0** | `LDR_PIN` | Input (Analog) | Photoresistor (LDR) + 10 kΩ | Ambient light intensity detection |
| **A1** | `TEMP_PIN` | Input (Analog) | TMP36 Temperature Sensor | Ambient temperature measurement |
| **A2** | `SOIL_PIN` | Input (Analog) | Potentiometer 1 (B10K) | Soil moisture level simulation |
| **A3** | `HUM_PIN` | Input (Analog) | Potentiometer 2 (B10K) | Relative humidity simulation |
| **A4** | `WATER_PIN`| Input (Analog) | Potentiometer 3 (B10K) | Water reservoir level simulation |
| **D9** | `LED4` | Output (Digital)| Red LED + 220 Ω resistor | Water level alert indicator |
| **D10**| `LED3` | Output (Digital)| Red LED + 220 Ω resistor | High temperature alert indicator |
| **D11**| `LED2` | Output (Digital)| Red LED + 220 Ω resistor | Low humidity alert indicator |
| **D12**| `LED1` | Output (Digital)| Red LED + 220 Ω resistor | Low soil moisture alert indicator |
| **5V** | `VCC` | Power Output | Breadboard Power Rail (+) | +5.0V DC regulated sensor power supply |
| **GND**| `GND` | Power Output | Breadboard Power Rail (-) | Common circuit ground reference |

---

## 2. Analog-to-Digital Converter (ADC) Conversion Formulas

The ATmega328P uses a 10-bit successive-approximation ADC with a 5.0V reference voltage ($V_{\text{ref}}$), yielding values from $0$ to $1023$.

### 2.1 Temperature (A1 — TMP36)
The TMP36 outputs $500\text{ mV}$ ($0.5\text{V}$) at $0^\circ\text{C}$ and scales at $10\text{ mV}/^\circ\text{C}$:
$$\text{Voltage (V)} = \text{tempRaw} \times \left(\frac{5.0}{1023.0}\right)$$
$$\text{Temperature } (^\circ\text{C}) = (\text{Voltage} - 0.5) \times 100.0$$

### 2.2 Light Intensity (A0 — LDR)
- Raw ADC range: $0$ to $1023$.
- In darkness, LDR resistance is high $\rightarrow$ ADC reading approaches $0$.
- In bright illumination, LDR resistance drops $\rightarrow$ ADC reading increases towards $1023$.

### 2.3 Simulated Soil Moisture, Humidity & Water Level (A2, A3, A4)
- **Soil Moisture (`SOIL_PIN` — A2)**: Raw integer $0$ to $1023$.
- **Humidity (`HUM_PIN` — A3)**: Raw integer $0$ to $1023$.
- **Water Level (`WATER_PIN` — A4)**: Raw integer $0$ to $1023$.
- *Percent Equivalence*: $\text{Value (\%)} = \frac{\text{raw}}{1023.0} \times 100.0$.

---

## 3. Threshold Rules & Actuator Triggers

The firmware implements individual condition checks for each metric, driving dedicated digital pins:

| Metric | Monitored Pin | Alert Condition | Active LED Output | Action Required |
|:---|:---:|:---:|:---:|:---|
| **Soil Moisture** | A2 | $\text{soilValue} < 400$ | **LED1 (D12)** = HIGH | Soil critically dry; initiate irrigation |
| **Humidity** | A3 | $\text{humidityValue} < 400$ | **LED2 (D11)** = HIGH | Air humidity low; misting required |
| **Temperature** | A1 | $\text{temperatureC} > 30.0^\circ\text{C}$ | **LED3 (D10)** = HIGH | High temperature warning; ventilation required |
| **Water Level** | A4 | $\text{waterValue} < 400$ | **LED4 (D9)** = HIGH | Water reservoir critically low; refill tank |

When values return to nominal ranges, the firmware automatically clears the respective digital outputs (`LOW`).

---

## 4. Serial Communication Specifications

- **Baud Rate**: `9600 bps` (8 Data bits, No Parity, 1 Stop bit — `8N1`).
- **Update Frequency**: Every 1000 ms (1 Hz).
- **Header Line (at boot)**:
  ```text
  Smart Agriculture Monitoring System
  ------------------------------------
  ```
- **Packet Structure**:
  ```text
  Light: <light> | Temperature: <temp> C | Soil Moisture: <soil> | Humidity: <hum> | Water Level: <water>\r\n
  ```
- **Example Serial Frame**:
  ```text
  Light: 650 | Temperature: 26.40 C | Soil Moisture: 520 | Humidity: 580 | Water Level: 610
  ```
- **Example Alert Frame (High Temp & Low Soil)**:
  ```text
  Light: 780 | Temperature: 34.50 C | Soil Moisture: 280 | Humidity: 420 | Water Level: 510
  ```
