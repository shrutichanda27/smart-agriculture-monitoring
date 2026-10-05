# Software Design Specification

## 1. Modular C++ Architecture

The host software is structured into focused, single-responsibility modules adhering to modern C++17 design principles:

```
src/
├── main.cpp                - System lifecycle, threading, signal handling
├── serial_reader.h/.cpp    - POSIX serial port access & line assembly
├── sensor_processor.h/.cpp - CSV tokenization, parsing & bounds validation
├── alert_engine.h/.cpp     - Rule-based multi-parameter threshold evaluation
├── database.h/.cpp         - SQLite relational persistence & prepared statements
├── dashboard_generator.h/.cpp - HTML/CSS presentation document synthesis
├── http_server.h/.cpp      - POSIX TCP socket server bound to 127.0.0.1:8080
└── driver_monitor.h/.cpp   - Linux character device & CDC-ACM diagnostic observer
```

---

## 2. Module Responsibilities & Interfaces

### 2.1 `SensorProcessor` (`sensor_processor.h` / `sensor_processor.cpp`)
- **Data Model**: `SensorReading` containing temperature, humidity, soil moisture, waterLevel, light, status string, and timestamp.
- **`parseSensorLine(const std::string& line, SensorReading& reading)`**:
  - Supports Tinkercad pipe-delimited format (`Light: 650 | Temperature: 26.4 C | ...`) and standard CSV.
  - Trims trailing whitespace and carriage returns (`\r`, `\n`).
  - Converts strings to floating-point numbers via `std::stof()` inside structured `try-catch` blocks catching `std::invalid_argument` and `std::out_of_range`.
  - Computes status token (`NORMAL` or `ALERT`) based on threshold rules.
- **`validateReading(const SensorReading& reading)`**:
  - Confirms physical boundaries:
    - Temperature: $[-40.0^\circ\text{C}, 125.0^\circ\text{C}]$
    - Humidity: $[0.0, 1023.0]$ (raw ADC / scaled)
    - Soil Moisture: $[0.0, 1023.0]$ (raw ADC / scaled)
    - Water Level: $[0.0, 1023.0]$ (raw ADC / scaled)
    - Light: $[0.0, 1023.0]$ (raw ADC / scaled)

### 2.2 `SerialReader` (`serial_reader.h` / `serial_reader.cpp`)
- Interfaces with the Linux CDC-ACM character device (default: `/dev/ttyACM0`).
- Configures POSIX `termios` parameters:
  - Baud rate: 9600 (`B9600`) via `cfsetispeed()` and `cfsetospeed()`.
  - Data bits: 8 (`CS8`).
  - Parity: None (`~PARENB`).
  - Stop bits: 1 (`~CSTOPB`).
  - Raw mode: Disables canonical processing (`~ICANON`), echo (`~ECHO`), and signals (`~ISIG`).
- Reads character-by-character to construct full newline-terminated frames safely without buffer overflows.

### 2.3 `AlertEngine` (`alert_engine.h` / `alert_engine.cpp`)
- Evaluates incoming sensor data against strict, deterministic rule boundaries matching Tinkercad hardware LEDs:
  - **Soil Moisture**: Alert if $< 400$ (LED1 / D12).
  - **Humidity**: Alert if $< 400$ (LED2 / D11).
  - **Temperature**: Alert if $> 30.0^\circ\text{C}$ (LED3 / D10) or $< 10.0^\circ\text{C}$.
  - **Water Level**: Alert if $< 400$ (LED4 / D9).
  - **Light**: Monitored without alert trigger.
- Emits structured `Alert` records containing timestamp, parameter name, breach condition (`TOO_HIGH` / `TOO_LOW`), observed value, threshold value, and descriptive message.

### 2.4 `Database` (`database.h` / `database.cpp`)
- Manages an embedded SQLite database (`smart_agriculture.db`).
- Creates tables with `CREATE TABLE IF NOT EXISTS`.
- All writes and reads use compiled prepared statements (`sqlite3_prepare_v2`, `sqlite3_bind_*`, `sqlite3_step`, `sqlite3_finalize`) to guarantee transactional integrity and prevent memory corruption.

### 2.5 `DashboardGenerator` (`dashboard_generator.h` / `dashboard_generator.cpp`)
- Converts raw database records into complete, well-formed HTML5 documents.
- Automatically escapes HTML entities to eliminate cross-site injection risks.
- Employs an agricultural palette featuring glassmorphism card layouts, responsive CSS grids, and condition-based status badges.
- Provides specialized views: main dashboard, component status, 404 Not Found, and 405 Method Not Allowed.

### 2.6 `HttpServer` (`http_server.h` / `http_server.cpp`)
- Native POSIX TCP socket server.
- Uses `socket()`, `setsockopt()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`, and `close()`.
- Explicitly binds to IPv4 loopback (`127.0.0.1`) on port `8080`.
- Limits maximum HTTP request sizes to 8 KB to prevent memory exhaustion attacks.
- Dispatches requests based on path (`/`, `/status`, `/alerts`, `/history`).

### 2.7 `DriverMonitor` (`driver_monitor.h` / `driver_monitor.cpp`)
- Uses Linux `stat()` system call to verify the presence and accessibility of `/dev/ttyACM0`.
- Provides non-fatal diagnostics when Arduino hardware is disconnected, allowing server-only or demo execution.

---

## 3. Concurrency & Threading Model

The application leverages standard C++11/17 threading primitives:

1. **Main Thread**: Initializes system components, executes the database migration, registers POSIX signal handlers (`SIGINT`, `SIGTERM`), and runs the HTTP server's blocking connection loop (`accept()`).
2. **Serial Reader Thread**: When hardware is present, a dedicated background thread continuously reads and parses serial frames, updates the SQLite database, and triggers alerts without impeding web dashboard responsiveness.
3. **Atomic Shutdown Coordination**: An `std::atomic<bool> g_running` flag guarantees thread-safe, coordinated lifecycle termination across all threads upon receiving interruption signals.
