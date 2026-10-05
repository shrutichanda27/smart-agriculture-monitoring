/*
 * Database Tests
 * Tests for SQLite insertion, retrieval, and schema integrity with water_level.
 */

#include "../src/database.h"
#include "../src/sensor_processor.h"
#include "../src/alert_engine.h"
#include <iostream>
#include <cstdio>
#include <cmath>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) \
    std::cout << "  TEST: " << name << " ... ";

#define PASS() \
    std::cout << "PASSED" << std::endl; testsPassed++;

#define FAIL(msg) \
    std::cout << "FAILED: " << msg << std::endl; testsFailed++;

static const char* TEST_DB = "test_agriculture.db";

static bool floatEqual(float a, float b, float epsilon = 0.1f) {
    return std::fabs(a - b) < epsilon;
}

void testOpenDatabase() {
    TEST("Open database and create tables");
    std::remove(TEST_DB);

    Database db(TEST_DB);
    if (!db.open()) { FAIL("failed to open database"); return; }
    db.close();
    PASS();
}

void testInsertAndRetrieve() {
    TEST("Insert reading and retrieve latest (including waterLevel)");
    std::remove(TEST_DB);
    Database db(TEST_DB);
    if (!db.open()) { FAIL("failed to open"); return; }

    SensorReading r;
    r.timestamp = "2025-01-01 12:00:00";
    r.temperature = 25.5f;
    r.humidity = 550.0f;
    r.soilMoisture = 450.0f;
    r.waterLevel = 610.0f;
    r.light = 700.0f;
    r.status = "NORMAL";

    if (!db.insertReading(r)) { FAIL("insert failed"); db.close(); return; }

    SensorReading latest;
    if (!db.getLatestReading(latest)) { FAIL("getLatestReading failed"); db.close(); return; }

    if (!floatEqual(latest.temperature, 25.5f)) { FAIL("temperature mismatch"); db.close(); return; }
    if (!floatEqual(latest.humidity, 550.0f)) { FAIL("humidity mismatch"); db.close(); return; }
    if (!floatEqual(latest.waterLevel, 610.0f)) { FAIL("waterLevel mismatch"); db.close(); return; }
    if (latest.status != "NORMAL") { FAIL("status mismatch"); db.close(); return; }

    db.close();
    PASS();
}

void testHistory() {
    TEST("Retrieve history with correct ordering");
    std::remove(TEST_DB);
    Database db(TEST_DB);
    if (!db.open()) { FAIL("failed to open"); return; }

    for (int i = 0; i < 5; i++) {
        SensorReading r;
        r.timestamp = "2025-01-01 12:0" + std::to_string(i) + ":00";
        r.temperature = 20.0f + i;
        r.humidity = 500.0f;
        r.soilMoisture = 400.0f;
        r.waterLevel = 550.0f;
        r.light = 500.0f;
        r.status = "NORMAL";
        db.insertReading(r);
    }

    auto history = db.getHistory(3);
    if (history.size() != 3) {
        FAIL("expected 3 records, got " + std::to_string(history.size()));
        db.close();
        return;
    }

    // Most recent first (temperature should be 24, 23, 22)
    if (!floatEqual(history[0].temperature, 24.0f)) {
        FAIL("first record should be newest (temp=24)");
        db.close();
        return;
    }

    db.close();
    PASS();
}

void testInsertAndRetrieveAlert() {
    TEST("Insert and retrieve alert");
    std::remove(TEST_DB);
    Database db(TEST_DB);
    if (!db.open()) { FAIL("failed to open"); return; }

    Alert a;
    a.timestamp = "2025-01-01 12:00:00";
    a.parameter = "Water Level";
    a.condition = "TOO_LOW";
    a.value = 350.0f;
    a.threshold = 400.0f;
    a.message = "Water reservoir critically low";

    if (!db.insertAlert(a)) { FAIL("insert alert failed"); db.close(); return; }

    auto alerts = db.getRecentAlerts(5);
    if (alerts.empty()) { FAIL("no alerts retrieved"); db.close(); return; }
    if (alerts[0].parameter != "Water Level") { FAIL("parameter mismatch"); db.close(); return; }

    db.close();
    PASS();
}

void testEmptyDatabase() {
    TEST("Empty database returns no latest reading");
    std::remove(TEST_DB);
    Database db(TEST_DB);
    if (!db.open()) { FAIL("failed to open"); return; }

    SensorReading r;
    if (db.getLatestReading(r)) { FAIL("should return false for empty db"); db.close(); return; }

    db.close();
    PASS();
}

void testMultipleInserts() {
    TEST("Multiple inserts maintain data integrity");
    std::remove(TEST_DB);
    Database db(TEST_DB);
    if (!db.open()) { FAIL("failed to open"); return; }

    for (int i = 0; i < 100; i++) {
        SensorReading r;
        r.timestamp = getCurrentTimestamp();
        r.temperature = 20.0f + (i % 20);
        r.humidity = 500.0f;
        r.soilMoisture = 400.0f;
        r.waterLevel = 600.0f;
        r.light = 500.0f;
        r.status = (i % 10 == 0) ? "ALERT" : "NORMAL";
        db.insertReading(r);
    }

    auto history = db.getHistory(100);
    if (history.size() != 100) {
        FAIL("expected 100 records, got " + std::to_string(history.size()));
        db.close();
        return;
    }

    db.close();
    PASS();
}

int main() {
    std::cout << "=== Database Tests ===" << std::endl;

    testOpenDatabase();
    testInsertAndRetrieve();
    testHistory();
    testInsertAndRetrieveAlert();
    testEmptyDatabase();
    testMultipleInserts();

    // Cleanup
    std::remove(TEST_DB);

    std::cout << "\nResults: " << testsPassed << " passed, "
              << testsFailed << " failed" << std::endl;

    return (testsFailed > 0) ? 1 : 0;
}
