#ifndef DATABASE_H
#define DATABASE_H

#include "sensor_processor.h"
#include "alert_engine.h"
#include <string>
#include <vector>

// Forward declaration — sqlite3 is an opaque type from the C API
struct sqlite3;

// Wraps SQLite operations for storing and retrieving sensor data and alerts.
class Database {
public:
    explicit Database(const std::string& dbPath = "smart_agriculture.db");
    ~Database();

    // Open the database and create tables if they don't exist.
    bool open();

    // Insert a sensor reading.
    bool insertReading(const SensorReading& reading);

    // Insert an alert record.
    bool insertAlert(const Alert& alert);

    // Retrieve the most recent sensor reading. Returns false if no data.
    bool getLatestReading(SensorReading& reading);

    // Retrieve the last `count` sensor readings (newest first).
    std::vector<SensorReading> getHistory(int count = 20);

    // Retrieve the last `count` alerts (newest first).
    std::vector<Alert> getRecentAlerts(int count = 10);

    // Close the database connection.
    void close();

private:
    std::string dbPath_;
    sqlite3* db_;

    bool createTables();
};

#endif // DATABASE_H
