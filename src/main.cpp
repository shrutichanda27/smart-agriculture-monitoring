/*
 * Smart Agriculture Monitoring System
 * Main Application Entry Point
 *
 * Coordinates all components:
 *   - Driver Monitor: checks Arduino connectivity
 *   - Serial Reader: reads data from /dev/ttyACM0
 *   - Sensor Processor: parses and validates sensor CSV lines
 *   - Alert Engine: checks thresholds and generates alerts
 *   - Database: stores readings and alerts in SQLite
 *   - HTTP Server: serves dashboard on http://127.0.0.1:8080
 *
 * Usage:
 *   ./smart_agriculture              # Normal mode (requires Arduino)
 *   ./smart_agriculture --demo       # Demo mode (inserts sample data, starts server)
 *   ./smart_agriculture --server-only # Start HTTP server with existing database
 */

#include "serial_reader.h"
#include "sensor_processor.h"
#include "alert_engine.h"
#include "database.h"
#include "http_server.h"
#include "driver_monitor.h"

#include <iostream>
#include <string>
#include <thread>
#include <csignal>
#include <atomic>
#include <chrono>
#include <cstring>

static std::atomic<bool> g_running(true);
static HttpServer* g_server = nullptr;

static void signalHandler(int signum) {
    std::cout << "\n[Main] Received signal " << signum << ", shutting down..." << std::endl;
    g_running = false;
    if (g_server) {
        g_server->stop();
    }
}

// Insert demonstration data for testing without an Arduino
static void insertDemoData(Database& db, AlertEngine& alertEngine) {
    std::cout << "[Main] Inserting demo sensor data..." << std::endl;

    struct DemoEntry {
        float temp, hum, soil, water, light;
        const char* status;
    };

    DemoEntry demos[] = {
        {25.3f, 550.0f, 600.0f, 650.0f, 720.0f, "NORMAL"},
        {28.1f, 620.0f, 450.0f, 580.0f, 680.0f, "NORMAL"},
        {27.5f, 700.0f, 480.0f, 620.0f, 750.0f, "NORMAL"},
        {22.0f, 480.0f, 500.0f, 540.0f, 500.0f, "NORMAL"},
        {29.8f, 520.0f, 420.0f, 500.0f, 800.0f, "NORMAL"},
        {35.8f, 350.0f, 250.0f, 320.0f, 900.0f, "ALERT"},   // Multiple alerts: Temp>30, Hum<400, Soil<400, Water<400
        {8.5f,  300.0f, 200.0f, 250.0f, 100.0f, "ALERT"},   // Multiple alerts
        {27.0f, 500.0f, 550.0f, 600.0f, 650.0f, "NORMAL"},
        {29.5f, 580.0f, 420.0f, 510.0f, 710.0f, "NORMAL"},
        {26.8f, 520.0f, 480.0f, 490.0f, 690.0f, "NORMAL"},
    };

    for (const auto& d : demos) {
        SensorReading reading;
        reading.temperature  = d.temp;
        reading.humidity     = d.hum;
        reading.soilMoisture = d.soil;
        reading.waterLevel   = d.water;
        reading.light        = d.light;
        reading.status       = d.status;
        reading.timestamp    = getCurrentTimestamp();

        db.insertReading(reading);

        auto alerts = alertEngine.checkThresholds(reading);
        for (const auto& alert : alerts) {
            db.insertAlert(alert);
        }

        // Small delay so timestamps differ
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "[Main] Inserted " << (sizeof(demos) / sizeof(demos[0])) << " demo readings" << std::endl;
}

// Serial reading loop — runs in a separate thread
static void serialLoop(SerialReader& serial, Database& db, AlertEngine& alertEngine, HttpServer& server) {
    if (!serial.open()) {
        std::cerr << "[Main] Failed to open serial port. Serial reading disabled." << std::endl;
        server.setSerialConnected(false);
        return;
    }

    server.setSerialConnected(true);
    std::string line;

    while (g_running.load()) {
        if (!serial.readLine(line)) {
            continue;
        }

        SensorReading reading;
        if (!parseSensorLine(line, reading)) {
            std::cerr << "[Main] Failed to parse: " << line << std::endl;
            continue;
        }

        if (!validateReading(reading)) {
            std::cerr << "[Main] Invalid reading rejected" << std::endl;
            continue;
        }

        // Store in database
        db.insertReading(reading);

        // Check alerts
        auto alerts = alertEngine.checkThresholds(reading);
        for (const auto& alert : alerts) {
            db.insertAlert(alert);
            std::cout << "[Alert] " << AlertEngine::formatAlert(alert) << std::endl;
        }

        if (alerts.empty()) {
            std::cout << "[Main] Reading OK: T=" << reading.temperature
                      << " H=" << reading.humidity
                      << " S=" << reading.soilMoisture
                      << " W=" << reading.waterLevel
                      << " L=" << reading.light << std::endl;
        }
    }

    serial.close();
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << " Smart Agriculture Monitoring System" << std::endl;
    std::cout << " C++ / Linux / SQLite / TCP/HTTP" << std::endl;
    std::cout << "========================================" << std::endl;

    // Parse command-line arguments
    bool demoMode = false;
    bool serverOnly = false;
    std::string devicePath = "/dev/ttyACM0";

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--demo") == 0) {
            demoMode = true;
        } else if (std::strcmp(argv[i], "--server-only") == 0) {
            serverOnly = true;
        } else if (std::strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            devicePath = argv[++i];
        } else if (std::strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [OPTIONS]\n"
                      << "  --demo          Insert demo data and start server\n"
                      << "  --server-only   Start HTTP server without serial reader\n"
                      << "  --device PATH   Specify serial device (default: /dev/ttyACM0)\n"
                      << "  --help          Show this help\n";
            return 0;
        }
    }

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Check driver status
    DriverMonitor monitor(devicePath);
    monitor.printStatus();

    // Open database
    Database db("smart_agriculture.db");
    if (!db.open()) {
        std::cerr << "[Main] Failed to open database. Exiting." << std::endl;
        return 1;
    }

    AlertEngine alertEngine;

    // Demo mode: insert sample data
    if (demoMode) {
        insertDemoData(db, alertEngine);
    }

    // Create HTTP server
    HttpServer server(db, 8080);
    g_server = &server;

    if (serverOnly || demoMode) {
        // No serial reader — just run the HTTP server
        std::cout << "[Main] Starting HTTP server (no serial reader)..." << std::endl;
        server.setSerialConnected(false);
        server.start();  // Blocks until stop() is called
    } else {
        // Full mode: serial reader in a thread, HTTP server in main thread
        if (!monitor.isDevicePresent()) {
            std::cerr << "[Main] WARNING: " << devicePath << " not found." << std::endl;
            std::cerr << "[Main] Connect Arduino or use --demo mode." << std::endl;
            std::cerr << "[Main] Starting HTTP server without serial reader..." << std::endl;
            server.setSerialConnected(false);
            server.start();
        } else {
            SerialReader serial(devicePath);
            std::thread serialThread(serialLoop, std::ref(serial), std::ref(db),
                                     std::ref(alertEngine), std::ref(server));

            server.start();  // Blocks until stop()

            if (serialThread.joinable()) {
                serialThread.join();
            }
        }
    }

    db.close();
    g_server = nullptr;

    std::cout << "[Main] Shutdown complete." << std::endl;
    return 0;
}
