# Stage 2 – Project Requirements & Development Plan

## 1. Project Requirements Document (PRD)

### 1.1 Functional Requirements
- **FR1:** The system shall receive sensor data from the Arduino serial interface.
- **FR2:** The system shall support demo-mode data for testing without physical hardware.
- **FR3:** The system shall validate incoming sensor data.
- **FR4:** The system shall process temperature, humidity, soil moisture, pH, and light values.
- **FR5:** The system shall detect configured abnormal conditions.
- **FR6:** The system shall store sensor readings in SQLite.
- **FR7:** The system shall store generated alerts in SQLite.
- **FR8:** The system shall provide a local HTTP dashboard.
- **FR9:** The system shall report the Linux serial-device/driver status.
- **FR10:** The system shall shut down gracefully when requested.

### 1.2 Non-Functional Requirements
- **NFR1:** The application shall run on Linux.
- **NFR2:** The core application shall use C++17.
- **NFR3:** The application shall be modular and maintainable.
- **NFR4:** The application shall handle a missing serial device without crashing.
- **NFR5:** Database operations shall use SQLite.
- **NFR6:** Automated tests shall be available for core functionality.
- **NFR7:** The system shall work locally without requiring cloud connectivity.
- **NFR8:** Source code and documentation shall be maintained using Git.

## 2. Major Modules
1. Serial Reader
2. Driver Monitor
3. Sensor Processor
4. Alert Engine
5. SQLite Database
6. HTTP Server
7. Dashboard Generator
8. Test Suite

## 3. Deliverables
- C++ source code
- Arduino/Tinkercad implementation
- CMake build configuration
- SQLite database integration
- Automated tests
- Dashboard
- Architecture/UML diagrams
- Six-stage documentation
- Git repository
- Final presentation/report

## 4. Development Plan

| Stage | Main Work | Evidence |
|---|---|---|
| Stage 1 | Project introduction and scope | Introduction document |
| Stage 2 | PRD and development plan | Requirements document |
| Stage 3 | Architecture, UML, environment setup | Diagrams and Git |
| Stage 4 | Core implementation and prototype | Running application/screenshots |
| Stage 5 | Testing, debugging and integration | Test results |
| Stage 6 | Final implementation and presentation | Final report/demo |

## 5. Development Environment
- Kali Linux/Linux environment
- GCC/G++
- CMake
- SQLite3
- Git
- Arduino/Tinkercad
- Web browser
