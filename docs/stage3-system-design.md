# Stage 3 – System Design & Architecture

## 1. System Architecture

```text
+-----------------------+
| Arduino / Tinkercad   |
| Sensor Simulation     |
+-----------+-----------+
            |
            | USB Serial
            v
+-----------------------+
| Linux CDC-ACM / TTY   |
| /dev/ttyACM0          |
+-----------+-----------+
            |
            v
+-----------------------+
| C++ Serial Reader     |
+-----------+-----------+
            |
            v
+-----------------------+
| Sensor Processor      |
+-----------+-----------+
            |
      +-----+------+
      |            |
      v            v
+-----------+  +-----------+
| Alert     |  | SQLite    |
| Engine    |  | Database  |
+-----+-----+  +-----+-----+
      |              |
      +------+-------+
             |
             v
      +--------------+
      | HTTP Server   |
      +------+-------+
             |
             v
      +--------------+
      | Web Dashboard |
      +--------------+
```

## 2. Component Responsibilities

### SerialReader
Communicates with the Linux serial device and receives sensor data.

### DriverMonitor
Checks the presence/status of the serial device and reports CDC-ACM/TTY information.

### SensorProcessor
Parses, validates, and processes sensor readings.

### AlertEngine
Evaluates readings against configured thresholds and generates alerts.

### Database
Stores sensor readings and alert information using SQLite.

### HTTP Server
Provides the local dashboard over HTTP.

### Dashboard Generator
Generates the monitoring interface from application data.

## 3. Data Structures
Typical application entities include:
- SensorReading
- Alert
- DeviceStatus
- Configuration/threshold data

## 4. UML Design

The project should include the following diagrams alongside this document:
- `class-diagram.png`
- `sequence-diagram.png`
- `state-machine.png`

## 5. Linux Device-Driver Interaction
The project uses the Linux CDC-ACM kernel driver and TTY subsystem for a USB serial Arduino device. The application interacts with the character device exposed as `/dev/ttyACM0`.

**Important:** The project demonstrates interaction with a Linux device driver; it does not claim to implement a custom kernel driver.

## 6. Development Environment
The implementation environment includes Linux/Kali Linux, GCC/G++, CMake, SQLite3, Git, and a web browser.

## 7. Git Strategy
- `main` contains stable project versions.
- Stage work is committed using descriptive messages.
- Changes should be tested before committing.
- Documentation and source changes are committed together when they form a stage milestone.
