# Tinkercad Circuit Design & Simulation

## 1. Overview & Live Model Link

This document details the circuit layout, electronic wiring, and sensor simulation configuration for the **Smart Agriculture Monitoring System**.

- **Tinkercad Project Link**: [Stunning Maimu - Smart Agriculture Monitoring System](https://www.tinkercad.com/things/czTrZN2pkM6-stunning-maimu?sharecode=Dy9K2MuJ0RvyA5Fo0lJsNRhPeJI-zsMpfhhkVO4wJyQ)
- **Microcontroller**: Arduino UNO R3 (ATmega328P)
- **Prototyping Platform**: Solderless Breadboard (Full-size)

---

## 2. Circuit Component List (Bill of Materials)

| # | Component | Quantity | Value / Spec | Role / Purpose |
|:---:|:---|:---:|:---|:---|
| 1 | Arduino UNO R3 | 1 | ATmega328P | Main controller & ADC acquisition |
| 2 | Solderless Breadboard | 1 | Full-size | Prototyping circuit interconnection |
| 3 | Photoresistor (LDR) | 1 | Cadmium-Sulfide (CdS) | Ambient light detection |
| 4 | Temperature Sensor | 1 | TMP36 | Direct temperature measurement (-40°C to +125°C) |
| 5 | Potentiometer 1 | 1 | 10 kΩ Linear (B10K) | Soil moisture simulation (0–1023 ADC) |
| 6 | Potentiometer 2 | 1 | 10 kΩ Linear (B10K) | Relative humidity simulation (0–1023 ADC) |
| 7 | Potentiometer 3 | 1 | 10 kΩ Linear (B10K) | Water level simulation (0–1023 ADC) |
| 8 | Red 5mm LEDs | 4 | 2.0V Forward, 20mA | Dedicated per-parameter alert indicators |
| 9 | Current Limiting Resistors | 4 | 220 Ω / 330 Ω 1/4W | Protection for LEDs (D9, D10, D11, D12) |
| 10 | Pull-Down Resistor | 1 | 10 kΩ 1/4W | Voltage divider resistor for LDR |
| 11 | Jumper Wires | ~25 | 22 AWG M-M | Power rails and signal routing |

> **Important Simulation Note**:
> Potentiometers are used as adjustable analog voltage dividers to emulate environmental conditions (soil moisture, air humidity, and irrigation water level). This allows repeatable testing of drought, high humidity, or low water reserves without physical soil or fluid probes.

---

## 3. Power Distribution Rails

1. **Common 5V Rail**:
   - Arduino `5V` pin $\rightarrow$ Breadboard Bottom Red Rail (`+`).
   - Breadboard Bottom Red Rail (`+`) bridged to Top Red Rail (`+`) via red jumper wire.
2. **Common Ground (GND) Rail**:
   - Arduino `GND` pin $\rightarrow$ Breadboard Bottom Black/Blue Rail (`-`).
   - Breadboard Bottom Black/Blue Rail (`-`) bridged to Top Black/Blue Rail (`-`) via black jumper wire.

---

## 4. Detailed Wiring & Pin Connections

### 4.1 Sensors (Analog Inputs A0–A4)

#### Light Sensor (LDR) $\rightarrow$ A0
- **LDR Leg 1**: Connected to Top `5V` rail (red wire).
- **LDR Leg 2**: Connected to Breadboard Column 37.
- **Pull-down Resistor (10 kΩ)**: From Column 37 to Bottom `GND` rail.
- **Signal Wire**: From Column 37 to Arduino **A0** (Yellow wire).
- *Circuit*: Forms a voltage divider. Higher ambient light decreases LDR resistance, raising voltage at A0.

#### Temperature Sensor (TMP36) $\rightarrow$ A1
- **Pin 1 (Left / +Vs)**: Connected to Bottom `5V` rail (red wire).
- **Pin 2 (Middle / Vout)**: Connected to Arduino **A1** (Yellow wire).
- **Pin 3 (Right / GND)**: Connected to Bottom `GND` rail (black wire).
- *Transfer Function*: $V_{\text{out}} = 0.5\text{V} + (10\text{mV}/^\circ\text{C} \times T)$.

#### Soil Moisture Simulator (POT1) $\rightarrow$ A2
- **Terminal 1 (Left)**: Connected to Top `5V` rail.
- **Center Wiper**: Connected to Arduino **A2** (Green wire).
- **Terminal 2 (Right)**: Connected to Top `GND` rail.
- *Function*: Turning dial varies ADC input from 0 to 1023.

#### Humidity Simulator (POT2) $\rightarrow$ A3
- **Terminal 1 (Left)**: Connected to Top `5V` rail.
- **Center Wiper**: Connected to Arduino **A3** (Blue wire).
- **Terminal 2 (Right)**: Connected to Top `GND` rail.
- *Function*: Turning dial varies ADC input from 0 to 1023.

#### Water Level Simulator (POT3) $\rightarrow$ A4
- **Terminal 1 (Left)**: Connected to Top `5V` rail.
- **Center Wiper**: Connected to Arduino **A4** (Orange wire).
- **Terminal 2 (Right)**: Connected to Top `GND` rail.
- *Function*: Turning dial varies ADC input from 0 to 1023.

---

### 4.2 Alert Indicators (Digital Outputs D9–D12)

Each alert parameter has an independent dedicated red LED indicator with an inline current-limiting resistor to protect the ATmega328P output driver:

| LED | Monitored Parameter | Arduino Pin | Wire Color | Threshold Trigger | Active State |
|:---|:---|:---:|:---:|:---|:---:|
| **LED1** | Soil Moisture | **D12** | Red / Orange | Soil Raw $< 400$ | HIGH (ON) |
| **LED2** | Humidity | **D11** | Red / Orange | Humidity Raw $< 400$ | HIGH (ON) |
| **LED3** | Temperature | **D10** | Red / Orange | Temperature $> 30.0^\circ\text{C}$ | HIGH (ON) |
| **LED4** | Water Level | **D9** | Red / Orange | Water Raw $< 400$ | HIGH (ON) |

- **Anode (Long leg)** of each LED connects to the respective Arduino digital pin.
- **Cathode (Short leg / flat rim)** connects through a 220 Ω / 330 Ω resistor to the `GND` rail.

---

## 5. ASCII Schematic Diagram

```
                             ARDUINO UNO R3
                         +--------------------+
                         |                    |
                         |                 D9 |----[220R]--->| (LED4: Water Alert)
                         |                D10 |----[220R]--->| (LED3: Temp Alert)
                         |                D11 |----[220R]--->| (LED2: Humidity Alert)
                         |                D12 |----[220R]--->| (LED1: Soil Alert)
                         |                    |
     LDR Divider --------| A0                 |
     TMP36 Vout  --------| A1                 |
     POT1 Wiper  --------| A2              5V |===================> 5V Rail
     POT2 Wiper  --------| A3             GND |===================> GND Rail
     POT3 Wiper  --------| A4                 |
                         +--------------------+
```

---

## 6. How to Recreate & Test in Tinkercad

1. Open the [Tinkercad Circuits Link](https://www.tinkercad.com/things/czTrZN2pkM6-stunning-maimu?sharecode=Dy9K2MuJ0RvyA5Fo0lJsNRhPeJI-zsMpfhhkVO4wJyQ) or create a new circuit.
2. Place an **Arduino UNO R3** and a full-size breadboard.
3. Wire the common 5V and GND buses to the breadboard rails.
4. Insert the 4 Red LEDs into columns 24, 26, 28, and 30, with 220 Ω resistors to GND and anodes to pins D12, D11, D10, and D9.
5. Wire the LDR with 10 kΩ pull-down resistor to pin A0.
6. Connect the TMP36 (+Vs to 5V, Vout to A1, GND to GND).
7. Connect the 3 potentiometers across 5V and GND with wipers connected to A2, A3, and A4.
8. Paste the code from `tinkercad/arduino_code.ino` into the Code window.
9. Click **Start Simulation**.
10. Open the **Serial Monitor** (set to 9600 baud) to view real-time multi-sensor output.
11. Turn the potentiometers or click the TMP36 to change temperature: observe the corresponding LEDs light up when threshold values are violated!
