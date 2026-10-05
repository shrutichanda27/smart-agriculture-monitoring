#include "database.h"
#include <sqlite3.h>
#include <iostream>

Database::Database(const std::string& dbPath)
    : dbPath_(dbPath), db_(nullptr) {}

Database::~Database() {
    close();
}

bool Database::open() {
    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Failed to open " << dbPath_
                  << ": " << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
        return false;
    }

    std::cout << "[Database] Opened " << dbPath_ << std::endl;
    return createTables();
}

bool Database::createTables() {
    const char* sensorTableSQL =
        "CREATE TABLE IF NOT EXISTS sensor_data ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  timestamp TEXT NOT NULL,"
        "  temperature REAL NOT NULL,"
        "  humidity REAL NOT NULL,"
        "  soil_moisture REAL NOT NULL,"
        "  water_level REAL NOT NULL,"
        "  light REAL NOT NULL,"
        "  status TEXT NOT NULL"
        ");";

    const char* alertTableSQL =
        "CREATE TABLE IF NOT EXISTS alerts ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  timestamp TEXT NOT NULL,"
        "  parameter TEXT NOT NULL,"
        "  condition TEXT NOT NULL,"
        "  value REAL NOT NULL,"
        "  threshold REAL NOT NULL,"
        "  message TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;

    int rc = sqlite3_exec(db_, sensorTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Failed to create sensor_data table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    rc = sqlite3_exec(db_, alertTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Failed to create alerts table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool Database::insertReading(const SensorReading& reading) {
    const char* sql =
        "INSERT INTO sensor_data (timestamp, temperature, humidity, soil_moisture, water_level, light, status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Prepare insert failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, reading.timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, reading.temperature);
    sqlite3_bind_double(stmt, 3, reading.humidity);
    sqlite3_bind_double(stmt, 4, reading.soilMoisture);
    sqlite3_bind_double(stmt, 5, reading.waterLevel);
    sqlite3_bind_double(stmt, 6, reading.light);
    sqlite3_bind_text(stmt, 7, reading.status.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "[Database] Insert reading failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    return true;
}

bool Database::insertAlert(const Alert& alert) {
    const char* sql =
        "INSERT INTO alerts (timestamp, parameter, condition, value, threshold, message) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Prepare alert insert failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, alert.timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, alert.parameter.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, alert.condition.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, alert.value);
    sqlite3_bind_double(stmt, 5, alert.threshold);
    sqlite3_bind_text(stmt, 6, alert.message.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "[Database] Insert alert failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    return true;
}

bool Database::getLatestReading(SensorReading& reading) {
    const char* sql =
        "SELECT timestamp, temperature, humidity, soil_moisture, water_level, light, status "
        "FROM sensor_data ORDER BY id DESC LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Prepare select failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        reading.timestamp    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        reading.temperature  = static_cast<float>(sqlite3_column_double(stmt, 1));
        reading.humidity     = static_cast<float>(sqlite3_column_double(stmt, 2));
        reading.soilMoisture = static_cast<float>(sqlite3_column_double(stmt, 3));
        reading.waterLevel   = static_cast<float>(sqlite3_column_double(stmt, 4));
        reading.light        = static_cast<float>(sqlite3_column_double(stmt, 5));
        reading.status       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

std::vector<SensorReading> Database::getHistory(int count) {
    std::vector<SensorReading> history;
    const char* sql =
        "SELECT timestamp, temperature, humidity, soil_moisture, water_level, light, status "
        "FROM sensor_data ORDER BY id DESC LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Prepare history failed: " << sqlite3_errmsg(db_) << std::endl;
        return history;
    }

    sqlite3_bind_int(stmt, 1, count);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SensorReading r;
        r.timestamp    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        r.temperature  = static_cast<float>(sqlite3_column_double(stmt, 1));
        r.humidity     = static_cast<float>(sqlite3_column_double(stmt, 2));
        r.soilMoisture = static_cast<float>(sqlite3_column_double(stmt, 3));
        r.waterLevel   = static_cast<float>(sqlite3_column_double(stmt, 4));
        r.light        = static_cast<float>(sqlite3_column_double(stmt, 5));
        r.status       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        history.push_back(r);
    }

    sqlite3_finalize(stmt);
    return history;
}

std::vector<Alert> Database::getRecentAlerts(int count) {
    std::vector<Alert> alerts;
    const char* sql =
        "SELECT timestamp, parameter, condition, value, threshold, message "
        "FROM alerts ORDER BY id DESC LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Prepare alerts query failed: " << sqlite3_errmsg(db_) << std::endl;
        return alerts;
    }

    sqlite3_bind_int(stmt, 1, count);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Alert a;
        a.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        a.parameter = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        a.condition = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        a.value     = static_cast<float>(sqlite3_column_double(stmt, 3));
        a.threshold = static_cast<float>(sqlite3_column_double(stmt, 4));
        a.message   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        alerts.push_back(a);
    }

    sqlite3_finalize(stmt);
    return alerts;
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
        std::cout << "[Database] Closed" << std::endl;
    }
}
