# Hardware Design & Simulation Specification

## 1. Environment & Simulation Platform

The physical sensing and microcontroller layer is simulated using **Autodesk Tinkercad Circuits**. Tinkercad provides SPICE-accurate electrical simulation of basic analog and digital components combined with an AVR ATmega328P instruction simulator.

- **Live Tinkercad Model**: [Stunning Maimu - Smart Agriculture Monitoring System](https://www.tinkercad.com/things/czTrZN2pkM6-stunning-maimu?sharecode=Dy9K2MuJ0RvyA5Fo0lJsNRhPeJI-zsMpfhhkVO4wJyQ)

> **CRITICAL ARCHITECTURAL NOTE**:
> In this simulated environment, potentiometers are used as variable voltage dividers to emulate environmental transducers (relative humidity, volumetric soil moisture, and water reservoir level). They are **simulation inputs**, not laboratory-grade agricultural probes. The TMP36 is a genuine temperature sensor model, and the photoresistor (LDR) responds to virtual ambient light.

---

## 2. Complete Bill of Materials (BOM)

| Item | Component | Specification | Quantity | Role |
|:---:|:---|:---|:---:|:---|
| 1 | Microcontroller | Arduino UNO R3 (ATmega328P) | 1 | Sensor acquisition, ADC conversion, UART framing |
| 2 | Breadboard | Full-size solderless breadboard | 1 | Circuit prototyping |
| 3 | Light Sensor | Photoresistor (LDR) | 1 | Ambient light intensity detection |
| 4 | Temperature Sensor | TMP36 Precision Sensor | 1 | Ambient temperature (-40°C to +125°C) |
| 5 | Potentiometer 1 | 10 kΩ Linear Potentiometer | 1 | Soil Moisture simulation (0–1023 ADC) |
| 6 | Potentiometer 2 | 10 kΩ Linear Potentiometer | 1 | Humidity simulation (0–1023 ADC) |
| 7 | Potentiometer 3 | 10 kΩ Linear Potentiometer | 1 | Water Level simulation (0–1023 ADC) |
| 8 | Fixed Resistor | 10 kΩ 1/4W Metal Film Resistor | 1 | Voltage divider load resistor for LDR |
| 9 | Status Alert LEDs | 5mm Red LEDs | 4 | Dedicated per-parameter alert indicators |
| 10 | Current Limiters | 220 Ω / 330 Ω 1/4W Resistors | 4 | Current limiting for LEDs (D9–D12) |
| 11 | Power Rail Jumpers| Solid Core 22 AWG Wires | ~25 | Signal routing and power distribution |

---

## 3. Hardware Pin Allocation Matrix

```
================================================================================
ARDUINO PIN   DEVICE / TRANSDUCER               SIGNAL TYPE     ROLE / THRESHOLD
================================================================================
A0            Photoresistor (LDR) Divider       Analog Input    Ambient Light Detection
A1            TMP36 (Pin 2 / Vout)              Analog Input    Temperature (°C)
A2            Potentiometer 1 (Center Wiper)    Analog Input    Soil Moisture Simulation
A3            Potentiometer 2 (Center Wiper)    Analog Input    Humidity Simulation
A4            Potentiometer 3 (Center Wiper)    Analog Input    Water Level Simulation
D9            LED4 + 220Ω (Red)                 Digital Output  Water Level Alert (< 400)
D10           LED3 + 220Ω (Red)                 Digital Output  Temperature Alert (> 30°C)
D11           LED2 + 220Ω (Red)                 Digital Output  Humidity Alert (< 400)
D12           LED1 + 220Ω (Red)                 Digital Output  Soil Moisture Alert (< 400)
5V            Common VCC Power Bus              Power Rail      +5.0V DC (+/- 5%)
GND           Common System Ground              Power Rail      0.0V Ground Reference
================================================================================
```

---

## 4. Electrical Schematics & Hookup Details

### 4.1 Light Sensor LDR (A0)
Forms a voltage divider with a 10 kΩ pull-down resistor:
- Top lead of LDR connects to `5V` rail.
- Bottom lead connects to Column 37, Arduino `A0`, and 10 kΩ resistor to `GND`.

### 4.2 TMP36 Temperature Sensor (A1)
- **Pin 1 (Left / +Vs)**: Connected to Arduino `5V` rail.
- **Pin 2 (Middle / Vout)**: Connected to Arduino Analog Pin `A1`.
- **Pin 3 (Right / GND)**: Connected to Arduino `GND` rail.
- **Characteristics**: $V_{\text{out}} = 500\,\text{mV}$ at $0^\circ\text{C}$; scale factor $= 10\,\text{mV}/^\circ\text{C}$.

### 4.3 Potentiometers (A2, A3, A4)
For each 10 kΩ potentiometer:
- **Terminal 1**: Connected to `5V` rail.
- **Terminal 2**: Connected to `GND` rail.
- **Wiper (Center pin)**:
  - Potentiometer 1 $\rightarrow$ `A2` (Soil Moisture emulation)
  - Potentiometer 2 $\rightarrow$ `A3` (Humidity emulation)
  - Potentiometer 3 $\rightarrow$ `A4` (Water Level emulation)

### 4.4 Dedicated Alert LEDs (D9–D12)
Four independent red LEDs provide visual feedback for specific threshold breaches:
- **D12 $\rightarrow$ LED1 (Soil Alert)**: Lights when soilValue $< 400$.
- **D11 $\rightarrow$ LED2 (Humidity Alert)**: Lights when humidityValue $< 400$.
- **D10 $\rightarrow$ LED3 (Temperature Alert)**: Lights when temperatureC $> 30.0^\circ\text{C}$.
- **D9  $\rightarrow$ LED4 (Water Alert)**: Lights when waterValue $< 400$.
Each LED cathode is grounded through a 220 Ω current-limiting resistor.
