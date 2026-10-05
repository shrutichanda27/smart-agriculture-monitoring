/*
 * Sensor Parser Tests
 * Tests for CSV and Tinkercad pipe-delimited parsing, field validation, and numeric conversion.
 */

#include "../src/sensor_processor.h"
#include <iostream>
#include <cassert>
#include <cmath>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) \
    std::cout << "  TEST: " << name << " ... "; \

#define PASS() \
    std::cout << "PASSED" << std::endl; testsPassed++;

#define FAIL(msg) \
    std::cout << "FAILED: " << msg << std::endl; testsFailed++;

static bool floatEqual(float a, float b, float epsilon = 0.1f) {
    return std::fabs(a - b) < epsilon;
}

void testValidLine() {
    TEST("Parse valid sensor line");
    SensorReading r;
    bool ok = parseSensorLine("28.5,62.0,55.0,600.0,720.0,NORMAL", r);
    if (!ok) { FAIL("parse returned false"); return; }
    if (!floatEqual(r.temperature, 28.5f)) { FAIL("temperature mismatch"); return; }
    if (!floatEqual(r.humidity, 62.0f)) { FAIL("humidity mismatch"); return; }
    if (!floatEqual(r.soilMoisture, 55.0f)) { FAIL("soilMoisture mismatch"); return; }
    if (!floatEqual(r.waterLevel, 600.0f)) { FAIL("waterLevel mismatch"); return; }
    if (!floatEqual(r.light, 720.0f)) { FAIL("light mismatch"); return; }
    if (r.status != "NORMAL") { FAIL("status mismatch"); return; }
    PASS();
}

void testAlertLine() {
    TEST("Parse ALERT status line");
    SensorReading r;
    bool ok = parseSensorLine("31.2,70.0,18.0,300.0,790.0,ALERT", r);
    if (!ok) { FAIL("parse returned false"); return; }
    if (r.status != "ALERT") { FAIL("expected ALERT status"); return; }
    if (!floatEqual(r.soilMoisture, 18.0f)) { FAIL("soilMoisture mismatch"); return; }
    if (!floatEqual(r.waterLevel, 300.0f)) { FAIL("waterLevel mismatch"); return; }
    PASS();
}

void testCarriageReturn() {
    TEST("Parse line with \\r\\n");
    SensorReading r;
    bool ok = parseSensorLine("25.0,50.0,40.0,500.0,600.0,NORMAL\r\n", r);
    if (!ok) { FAIL("parse returned false"); return; }
    if (!floatEqual(r.temperature, 25.0f)) { FAIL("temperature mismatch"); return; }
    PASS();
}

void testEmptyLine() {
    TEST("Reject empty line");
    SensorReading r;
    bool ok = parseSensorLine("", r);
    if (ok) { FAIL("should reject empty line"); return; }
    PASS();
}

void testMissingFields() {
    TEST("Reject line with missing fields");
    SensorReading r;
    bool ok = parseSensorLine("28.5,62.0,55.0,600.0", r);
    if (ok) { FAIL("should reject 4 fields"); return; }
    PASS();
}

void testExtraFields() {
    TEST("Reject line with extra fields");
    SensorReading r;
    bool ok = parseSensorLine("28.5,62.0,55.0,600.0,720.0,NORMAL,extra", r);
    if (ok) { FAIL("should reject 7 fields"); return; }
    PASS();
}

void testNonNumericValue() {
    TEST("Reject non-numeric temperature");
    SensorReading r;
    bool ok = parseSensorLine("abc,62.0,55.0,600.0,720.0,NORMAL", r);
    if (ok) { FAIL("should reject non-numeric value"); return; }
    PASS();
}

void testInvalidStatus() {
    TEST("Reject invalid status");
    SensorReading r;
    bool ok = parseSensorLine("28.5,62.0,55.0,600.0,720.0,UNKNOWN", r);
    if (ok) { FAIL("should reject invalid status"); return; }
    PASS();
}

void testValidateNormalReading() {
    TEST("Validate normal reading passes");
    SensorReading r;
    r.temperature = 25.0f; r.humidity = 500.0f; r.soilMoisture = 600.0f;
    r.waterLevel = 650.0f; r.light = 500.0f;
    if (!validateReading(r)) { FAIL("should pass validation"); return; }
    PASS();
}

void testValidateOutOfRange() {
    TEST("Validate out-of-range temperature fails");
    SensorReading r;
    r.temperature = 200.0f; r.humidity = 500.0f; r.soilMoisture = 400.0f;
    r.waterLevel = 500.0f; r.light = 500.0f;
    if (validateReading(r)) { FAIL("should fail validation for temp 200"); return; }
    PASS();
}

void testValidateNegativeHumidity() {
    TEST("Validate negative humidity fails");
    SensorReading r;
    r.temperature = 25.0f; r.humidity = -5.0f; r.soilMoisture = 400.0f;
    r.waterLevel = 500.0f; r.light = 500.0f;
    if (validateReading(r)) { FAIL("should fail validation for humidity -5"); return; }
    PASS();
}

void testTimestamp() {
    TEST("getCurrentTimestamp returns non-empty string");
    std::string ts = getCurrentTimestamp();
    if (ts.empty()) { FAIL("timestamp is empty"); return; }
    if (ts.size() < 19) { FAIL("timestamp too short"); return; }
    PASS();
}

void testTinkercadPipeFormat() {
    TEST("Parse Tinkercad pipe-delimited sensor stream");
    SensorReading r;
    bool ok = parseSensorLine("Light: 650 | Temperature: 26.40 C | Soil Moisture: 520 | Humidity: 580 | Water Level: 610", r);
    if (!ok) { FAIL("parse returned false"); return; }
    if (!floatEqual(r.light, 650.0f)) { FAIL("light mismatch"); return; }
    if (!floatEqual(r.temperature, 26.4f)) { FAIL("temperature mismatch"); return; }
    if (!floatEqual(r.soilMoisture, 520.0f)) { FAIL("soilMoisture mismatch"); return; }
    if (!floatEqual(r.humidity, 580.0f)) { FAIL("humidity mismatch"); return; }
    if (!floatEqual(r.waterLevel, 610.0f)) { FAIL("water level mismatch"); return; }
    if (r.status != "NORMAL") { FAIL("status should be NORMAL for nominal values"); return; }
    PASS();
}

void testTinkercadPipeAlertFormat() {
    TEST("Parse Tinkercad pipe-delimited alert stream (low soil & water)");
    SensorReading r;
    bool ok = parseSensorLine("Light: 300 | Temperature: 32.50 C | Soil Moisture: 250 | Humidity: 450 | Water Level: 350", r);
    if (!ok) { FAIL("parse returned false"); return; }
    if (!floatEqual(r.temperature, 32.5f)) { FAIL("temperature mismatch"); return; }
    if (r.status != "ALERT") { FAIL("status should be ALERT for temp>30 / soil<400 / water<400"); return; }
    PASS();
}

int main() {
    std::cout << "=== Sensor Parser Tests ===" << std::endl;

    testValidLine();
    testAlertLine();
    testTinkercadPipeFormat();
    testTinkercadPipeAlertFormat();
    testCarriageReturn();
    testEmptyLine();
    testMissingFields();
    testExtraFields();
    testNonNumericValue();
    testInvalidStatus();
    testValidateNormalReading();
    testValidateOutOfRange();
    testValidateNegativeHumidity();
    testTimestamp();

    std::cout << "\nResults: " << testsPassed << " passed, "
              << testsFailed << " failed" << std::endl;

    return (testsFailed > 0) ? 1 : 0;
}
