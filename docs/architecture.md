# System Architecture

## 1. Overview

The **Smart Agriculture Monitoring System** is an embedded, Linux-native, end-to-end monitoring solution built with modern C++ (C++17), SQLite, POSIX sockets, and an Arduino UNO hardware simulation.

The architecture emphasizes **local execution**, **client/server decoupling**, **Linux kernel-to-userspace device communication**, and **lightweight networking fundamentals**.

No cloud services, external web frameworks, machine learning models, or interpreted languages (Python, JavaScript, etc.) are utilized.

---

## 2. Complete End-to-End Architectural Pipeline

```
                    +---------------------------+
                    |     TINKERCAD CIRCUITS    |
                    |   (Virtual Breadboard)    |
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    |        Arduino UNO        |
                    |      - TMP36 (Temp)       |
                    |      - Potentiometer (Hum)|
                    |      - Potentiometer (Soil|
                    |      - Potentiometer (pH) |
                    |      - LDR (Light)        |
                    |      - LED Status (D13)   |
                    +---------------------------+
                                  |
                                  | USB Serial (CDC-ACM)
                                  | 9600 Baud, 8N1 CSV Protocol
                                  v
                    +---------------------------+
                    | Linux USB / TTY Subsystem |
                    | Driver: cdc_acm           |
                    | Device: /dev/ttyACM0      |
                    +---------------------------+
                                  |
                                  | POSIX open(), tcgetattr(), read()
                                  v
                    +---------------------------+
                    |     C++ Serial Reader     |
                    |    (src/serial_reader)    |
                    +---------------------------+
                                  | Line buffering ('\n')
                                  v
                    +---------------------------+
                    |   C++ Sensor Processor    |
                    |  (src/sensor_processor)   |
                    +---------------------------+
                                  |
                   +--------------+--------------+
                   |                             |
                   v                             v
       +-----------------------+     +-----------------------+
       |   SQLite Database     |     |   C++ Alert Engine    |
       | (smart_agriculture.db)|     |  (src/alert_engine)   |
       +-----------------------+     +-----------------------+
                   |                             |
                   | Query latest / history      | Store active alerts
                   +--------------+--------------+
                                  |
                                  v
                    +---------------------------+
                    |  C++ Dashboard Generator  |
                    | (src/dashboard_generator) |
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    |     C++ HTTP Server       |
                    |    (src/http_server)      |
                    |  POSIX Sockets (AF_INET)  |
                    |  Binds to 127.0.0.1:8080  |
                    +---------------------------+
                                  ^
                                  | HTTP GET /
                                  | HTTP/1.1 200 OK (HTML/CSS)
                                  v
                    +---------------------------+
                    |     Web Browser           |
                    |   (Google Chrome, etc.)   |
                    |   URL: http://localhost:8080
                    +---------------------------+
```

---

## 3. Subsystem Breakdown

### 3.1 Hardware Emulation (Tinkercad Circuits)
- Simulates an Arduino UNO connected to physical agricultural sensors via analog inputs `A0`–`A4`.
- Converts physical/voltage phenomena into 10-bit analog-to-digital converter (ADC) integers (0–1023).
- Applies mathematical calibration transformations to derive engineering units (°C, %, pH, lux).
- Outputs a formatted ASCII CSV packet every 2000 ms.

### 3.2 Linux Kernel & Device Interface Layer
- USB communication adheres to the **USB Communication Device Class (CDC) Abstract Control Model (ACM)**.
- Handled transparently by the Linux `cdc_acm` driver module.
- Exposed to user space as the character device node `/dev/ttyACM0`.
- The user-space C++ application interacts with this device using standard POSIX system calls (`open()`, `read()`, `close()`) and termios line disciplines (`cfsetispeed()`, `tcsetattr()`).

### 3.3 Data Ingestion & Rule-Based Processing
- `SerialReader`: Assembles stream-oriented bytes into complete record lines terminated by `\n`.
- `SensorProcessor`: Validates field counts, parses floating-point numbers with defensive exception handling, enforces physical bounds, and timestamps records.
- `AlertEngine`: Pure deterministic rule evaluation. Compares observed values against threshold limits (temperature [10, 40], humidity [30, 80], soil moisture >= 25, pH [5.5, 8.0]).
- `Database`: Persists normalized sensor records and generated alert logs into an SQLite transactional relational database (`smart_agriculture.db`) using prepared statements.

### 3.4 Local Networking & Presentation Layer
- `HttpServer`: Written directly with POSIX socket APIs (`socket()`, `setsockopt()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`).
- Binds exclusively to the IPv4 loopback address (`127.0.0.1`) on TCP port `8080`.
- Decoupled from the browser: The web browser issues standard HTTP/1.1 GET requests and renders the returned HTML/CSS. The browser never accesses the database or serial stream directly.

---

## 4. Key Architectural Guarantees

1. **Zero External Network Dependencies**: The system functions completely offline on a standalone Linux machine. No internet connection, cloud host, or LAN broadcast is required.
2. **Deterministic Processing**: No heuristic, probabilistic, or stochastic machine learning algorithms are used. Threshold enforcement is strictly rule-based.
3. **Robust Isolation**: The database and serial hardware are completely abstracted behind the C++ application server.
