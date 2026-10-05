# Stage 1 – Project Introduction

## Project Title
**Smart Agriculture Monitoring System**

## 1. Introduction
The Smart Agriculture Monitoring System is a Linux-based monitoring application that collects and processes environmental data from an Arduino-based sensor simulation. The system is designed to demonstrate embedded-device communication, Linux system programming, C++ programming, database storage, alert processing, and a local web dashboard.

## 2. Problem Statement
Agricultural environments require continuous monitoring of conditions such as temperature, humidity, soil moisture, pH, and light. Manual monitoring can be time-consuming and may delay the detection of abnormal conditions. This project provides a small, modular monitoring system that can collect readings, validate them, store them, detect alerts, and present the information through a local dashboard.

## 3. Objectives
- Read environmental sensor data from an Arduino/serial device.
- Demonstrate Linux device and TTY/CDC-ACM interaction.
- Process sensor data using C++17.
- Detect abnormal environmental conditions.
- Store readings and alerts in SQLite.
- Provide a local HTTP dashboard.
- Demonstrate modular system programming and testing.

## 4. Scope
The project covers the Arduino-side sensor simulation, Linux serial-device interaction, C++ processing, SQLite persistence, alert generation, HTTP serving, and dashboard presentation. It is intended as an academic prototype rather than a production agricultural control system.

## 5. Expected Outcome
The completed system should:
1. Accept sensor readings or demo readings.
2. Process and validate the readings.
3. Store sensor data in SQLite.
4. Generate alerts when configured thresholds are exceeded.
5. Serve the monitoring dashboard locally.
6. Continue safely when the serial device is unavailable.

## 6. Applications
- Academic demonstration of embedded/Linux integration.
- Smart agriculture prototypes.
- Environmental monitoring demonstrations.
- IoT and edge-computing learning projects.

## 7. Technology Stack
- Linux/Kali Linux
- C++17
- CMake
- SQLite
- POSIX serial/socket APIs
- Arduino/Tinkercad
- HTML/CSS dashboard
- Git/GitHub
