# Smart Agriculture Monitoring System Using Arduino, Linux and C++

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.kernel.org/)
[![Database](https://img.shields.io/badge/Database-SQLite3-lightgrey.svg)](https://www.sqlite.org/)
[![Hardware Simulation](https://img.shields.io/badge/Simulation-Tinkercad-green.svg)](https://www.tinkercad.com/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A complete, self-contained capstone project demonstrating end-to-end embedded systems engineering, Linux device driver interaction, C++ data processing, relational database management, and local TCP/IP socket networking.

---

## 1. System Overview & Architecture

```
                    TINKERCAD CIRCUITS
                            |
                            v
                       Arduino UNO
                            |
                      Analog Readings
                    (A0, A1, A2, A3, A4)
                            |
                            v
                   USB Serial Interface
                       (9600 Baud)
                            |
                            v
                Linux CDC-ACM / TTY Layer
                     (kernel module)
                            |
                            v
                      /dev/ttyACM0
                     (Character Device)
                            |
                            v
                    C++ Serial Reader
                   (POSIX termios raw)
                            |
                            v
                   C++ Data Processor
                  (Defensive CSV Parser)
                            |
                 +----------+----------+
                 |                     |
                 v                     v
          SQLite Database         Alert Engine
      (smart_agriculture.db)    (Rule-based bounds)
                 |
                 v
           C++ Local HTTP Server
       (POSIX Sockets: AF_INET / TCP)
                 |
                 | HTTP GET /
              TCP/IP
             localhost
            (127.0.0.1:8080)
                 |
                 v
            Web Browser
       (Local HTML/CSS Dashboard)
```

### Key Architectural Characteristics
- **Strictly Local**: Binds exclusively to `127.0.0.1:8080`. No cloud, no external network, no remote APIs.
- **Pure C/C++ Stack**: Core logic written in C++17 using POSIX system APIs and SQLite3 C API.
- **Zero Forbidden Technologies**: No Python, Java, JavaScript, Node.js, React, Docker, MQTT, or Firebase.
- **Rule-Based Determinism**: No opaque machine learning models; thresholds are transparent and deterministic.

---

## 2. Hardware Simulation & Pin Mapping

Sensors are simulated in **Autodesk Tinkercad Circuits** using an Arduino UNO R3:

| Pin | Transducer / Component | Purpose | Unit / Range |
|:---:|:---|:---|:---|
| **A0** | Photoresistor (LDR) + 10 kΩ | Ambient light intensity | 0 to 1023 raw ADC |
| **A1** | TMP36 Temperature Sensor | Ambient temperature | -40.0°C to +125.0°C |
| **A2** | Potentiometer 1 | Emulated soil moisture | 0 to 1023 (Alert < 400) |
| **A3** | Potentiometer 2 | Emulated air humidity | 0 to 1023 (Alert < 400) |
| **A4** | Potentiometer 3 | Emulated water level | 0 to 1023 (Alert < 400) |
| **D9** | Red LED 4 + 220 Ω resistor | Water level alert indicator | ON when water < 400 |
| **D10**| Red LED 3 + 220 Ω resistor | Temperature alert indicator | ON when temp > 30.0°C |
| **D11**| Red LED 2 + 220 Ω resistor | Humidity alert indicator | ON when humidity < 400 |
| **D12**| Red LED 1 + 220 Ω resistor | Soil moisture alert indicator | ON when soil < 400 |

> *Note: Potentiometers are used as controllable voltage dividers for test repeatability. They are simulation inputs, not physical agricultural probes. Simulation model: [Stunning Maimu](https://www.tinkercad.com/things/czTrZN2pkM6-stunning-maimu?sharecode=Dy9K2MuJ0RvyA5Fo0lJsNRhPeJI-zsMpfhhkVO4wJyQ).*

---

## 3. Directory Layout

```
smart-agriculture-monitoring/
│
├── README.md                   # Project overview, setup, and evaluation
├── CMakeLists.txt              # CMake build configuration
├── LICENSE                     # MIT open-source license
│
├── tinkercad/                  # Hardware simulation assets
│   ├── arduino_code.ino        # Arduino firmware (C/C++)
│   ├── circuit.md              # Tinkercad circuit construction guide
│   └── pin_configuration.md    # Pin allocation & ADC conversion formulas
│
├── src/                        # Modular C++17 source code
│   ├── main.cpp                # Application entry point & thread lifecycle
│   ├── serial_reader.h/.cpp    # POSIX termios serial reader (/dev/ttyACM0)
│   ├── sensor_processor.h/.cpp # CSV parsing & bounds validation
│   ├── alert_engine.h/.cpp     # Rule-based threshold evaluation
│   ├── database.h/.cpp         # SQLite prepared statements & persistence
│   ├── http_server.h/.cpp      # POSIX TCP socket server (127.0.0.1:8080)
│   ├── dashboard_generator.h/.cpp # HTML/CSS dashboard generator
│   └── driver_monitor.h/.cpp   # Linux CDC-ACM device presence monitor
│
├── database/                   # Database schemas
│   └── schema.sql              # SQLite table definitions
│
├── dashboard/                  # Dashboard presentation assets
│   ├── index.html              # Static design reference template
│   └── style.css               # Agricultural glassmorphism stylesheet
│
├── tests/                      # Automated unit & integration test suites
│   ├── parser_test.cpp         # CSV parser & validation tests
│   ├── alert_test.cpp          # Rule engine threshold tests
│   ├── database_test.cpp       # SQLite CRUD & transactional tests
│   └── http_server_test.cpp    # Socket lifecycle & HTTP response tests
│
├── docs/                       # Comprehensive documentation
│   ├── architecture.md         # End-to-end architectural specification
│   ├── hardware-design.md      # Hardware BOM, wiring, and schematics
│   ├── software-design.md      # C++ modular design & threading model
│   ├── linux-driver.md         # CDC-ACM, TTY subsystem & character devices
│   ├── serial-communication.md # Framing protocol, baud rate, and CSV specs
│   ├── networking.md           # 18 networking concepts & socket lifecycle
│   ├── database-design.md      # Relational schema & prepared statements
│   ├── testing.md              # Test matrix, assertions, and execution
│   └── evaluation-guide.md     # 20-step viva demonstration & Q&A defense
│
├── scripts/                    # Automation & validation tooling
│   └── validate.sh             # Strict pre-flight validation script
│
└── screenshots/                # Demonstration screenshots
    └── .gitkeep
```

---

## 4. Build & Execution Instructions (Linux)

### 4.1 Prerequisites
On Ubuntu / Debian systems, install the required packages:
```bash
sudo apt update
sudo apt install -y build-essential cmake libsqlite3-dev
```

### 4.2 Building the Project
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 4.3 Running Automated Tests
```bash
ctest --output-on-failure
# Or run individual test suites:
./parser_test
./alert_test
./database_test
./http_server_test
```

### 4.4 Running the Application

#### Option A: Full Mode (Connected to Arduino UNO via USB)
```bash
# Add current user to dialout group to access /dev/ttyACM0:
sudo usermod -a -G dialout $USER

./smart_agriculture
```

#### Option B: Standalone Demonstration Mode (No Arduino required)
Populates the SQLite database with representative telemetry and starts the local web server:
```bash
./smart_agriculture --demo
```

#### Option C: Server-Only Mode (Using existing database records)
```bash
./smart_agriculture --server-only
```

Once running, open any web browser and navigate to:
```
http://localhost:8080
```
or
```
http://127.0.0.1:8080
```

---

## 5. Automated Validation Pipeline

Run the comprehensive validation script to verify project structure, check against forbidden technologies, validate socket and driver code, and compile targets:
```bash
./scripts/validate.sh
```

---

## 6. Supported HTTP Endpoints

| Endpoint | Method | Response | Description |
|:---|:---:|:---:|:---|
| `/` or `/index.html` | `GET` | `200 OK` | Main dashboard displaying real-time metrics, status, alerts, and history |
| `/status` | `GET` | `200 OK` | System component status (HTTP Server, Database, Serial Driver) |
| `/alerts` | `GET` | `200 OK` | Filtered view showing historical alert logs |
| `/history` | `GET` | `200 OK` | Detailed historical readings log |
| `/*` (Other paths) | `GET` | `404 Not Found` | Clean, styled 404 error page |
| Any path | Non-`GET` | `405 Method Not Allowed` | Rejection response for non-GET methods |

---

## 7. Threshold Rules & Actuator Responses

| Parameter | Normal Range | Alert Trigger | Dedicated Actuator Indicator |
|:---|:---:|:---:|:---:|
| **Soil Moisture** | $\ge 400$ | $< 400$ | **LED1 (D12)** = HIGH |
| **Humidity** | $\ge 400$ | $< 400$ | **LED2 (D11)** = HIGH |
| **Temperature** | $\le 30.0^\circ\text{C}$ | $> 30.0^\circ\text{C}$ (or $< 10^\circ\text{C}$) | **LED3 (D10)** = HIGH |
| **Water Level** | $\ge 400$ | $< 400$ | **LED4 (D9)** = HIGH |
| **Light** | Monitored | No alert rule | None |

---

## 8. License

This project is licensed under the [MIT License](LICENSE).

