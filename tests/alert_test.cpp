/*
 * Alert Engine Tests
 * Tests for threshold checking and alert generation matching Tinkercad conditions:
 *   - Soil Moisture: < 400 (LED1 D12)
 *   - Humidity: < 400 (LED2 D11)
 *   - Temperature: > 30°C (LED3 D10)
 *   - Water Level: < 400 (LED4 D9)
 */

#include "../src/alert_engine.h"
#include "../src/sensor_processor.h"
#include <iostream>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) \
    std::cout << "  TEST: " << name << " ... ";

#define PASS() \
    std::cout << "PASSED" << std::endl; testsPassed++;

#define FAIL(msg) \
    std::cout << "FAILED: " << msg << std::endl; testsFailed++;

static SensorReading makeReading(float temp, float hum, float soil, float water, float light) {
    SensorReading r;
    r.temperature = temp;
    r.humidity = hum;
    r.soilMoisture = soil;
    r.waterLevel = water;
    r.light = light;
    r.status = "NORMAL";
    r.timestamp = "2025-01-01 12:00:00";
    return r;
}

void testNormalReadingNoAlerts() {
    TEST("Normal reading produces no alerts");
    AlertEngine engine;
    auto r = makeReading(25.0f, 600.0f, 600.0f, 600.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    if (!alerts.empty()) { FAIL("expected 0 alerts, got " + std::to_string(alerts.size())); return; }
    PASS();
}

void testTemperatureTooHigh() {
    TEST("Temperature > 30°C triggers alert (LED3)");
    AlertEngine engine;
    auto r = makeReading(35.0f, 600.0f, 600.0f, 600.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    bool found = false;
    for (const auto& a : alerts) {
        if (a.parameter == "Temperature" && a.condition == "TOO_HIGH") found = true;
    }
    if (!found) { FAIL("expected Temperature TOO_HIGH alert"); return; }
    PASS();
}

void testTemperatureTooLow() {
    TEST("Temperature < 10°C triggers alert");
    AlertEngine engine;
    auto r = makeReading(5.0f, 600.0f, 600.0f, 600.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    bool found = false;
    for (const auto& a : alerts) {
        if (a.parameter == "Temperature" && a.condition == "TOO_LOW") found = true;
    }
    if (!found) { FAIL("expected Temperature TOO_LOW alert"); return; }
    PASS();
}

void testHumidityTooLow() {
    TEST("Humidity < 400 triggers alert (LED2)");
    AlertEngine engine;
    auto r = makeReading(25.0f, 350.0f, 600.0f, 600.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    bool found = false;
    for (const auto& a : alerts) {
        if (a.parameter == "Humidity" && a.condition == "TOO_LOW") found = true;
    }
    if (!found) { FAIL("expected Humidity TOO_LOW alert"); return; }
    PASS();
}

void testSoilMoistureTooLow() {
    TEST("Soil moisture < 400 triggers alert (LED1)");
    AlertEngine engine;
    auto r = makeReading(25.0f, 600.0f, 350.0f, 600.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    bool found = false;
    for (const auto& a : alerts) {
        if (a.parameter == "Soil Moisture" && a.condition == "TOO_LOW") found = true;
    }
    if (!found) { FAIL("expected Soil Moisture TOO_LOW alert"); return; }
    PASS();
}

void testWaterLevelTooLow() {
    TEST("Water level < 400 triggers alert (LED4)");
    AlertEngine engine;
    auto r = makeReading(25.0f, 600.0f, 600.0f, 350.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    bool found = false;
    for (const auto& a : alerts) {
        if (a.parameter == "Water Level" && a.condition == "TOO_LOW") found = true;
    }
    if (!found) { FAIL("expected Water Level TOO_LOW alert"); return; }
    PASS();
}

void testMultipleAlerts() {
    TEST("All 4 parameter breaches trigger 4 alerts");
    AlertEngine engine;
    auto r = makeReading(35.0f, 250.0f, 200.0f, 150.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    if (alerts.size() != 4) {
        FAIL("expected exactly 4 alerts, got " + std::to_string(alerts.size()));
        return;
    }
    PASS();
}

void testBoundaryValues() {
    TEST("Values at normal limits produce no alerts");
    AlertEngine engine;
    auto r = makeReading(30.0f, 400.0f, 400.0f, 400.0f, 500.0f);
    auto alerts = engine.checkThresholds(r);
    if (!alerts.empty()) {
        FAIL("boundary values should not trigger alerts, got " + std::to_string(alerts.size()));
        return;
    }
    PASS();
}

void testFormatAlert() {
    TEST("formatAlert produces readable string");
    Alert a;
    a.timestamp = "2025-01-01 12:00:00";
    a.parameter = "Water Level";
    a.condition = "TOO_LOW";
    a.value = 350.0f;
    a.threshold = 400.0f;
    std::string s = AlertEngine::formatAlert(a);
    if (s.empty()) { FAIL("formatted string is empty"); return; }
    if (s.find("Water Level") == std::string::npos) { FAIL("missing parameter name"); return; }
    PASS();
}

int main() {
    std::cout << "=== Alert Engine Tests ===" << std::endl;

    testNormalReadingNoAlerts();
    testTemperatureTooHigh();
    testTemperatureTooLow();
    testHumidityTooLow();
    testSoilMoistureTooLow();
    testWaterLevelTooLow();
    testMultipleAlerts();
    testBoundaryValues();
    testFormatAlert();

    std::cout << "\nResults: " << testsPassed << " passed, "
              << testsFailed << " failed" << std::endl;

    return (testsFailed > 0) ? 1 : 0;
}
