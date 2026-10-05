# Verification & Testing Strategy

## 1. Overview

The test harness comprises four independent test suites targeting every layer of the system:
1. `parser_test`: Verifies serial stream tokenization and boundary validation.
2. `alert_test`: Verifies rule-based alert triggering across valid, invalid, and boundary cases.
3. `database_test`: Validates SQLite CRUD operations, prepared statements, and transactional persistence.
4. `http_server_test`: Validates POSIX TCP socket creation, HTTP request parsing, routing, and response headers.

---

## 2. Test Suites & Coverage Matrix

### 2.1 Sensor Parser Tests (`tests/parser_test.cpp`)
| Test Case | Description | Expected Outcome |
|:---|:---|:---|
| `testValidLine` | Parses well-formed CSV line: `28.5,62.0,55.0,6.8,720.0,NORMAL` | Returns true, fields parsed correctly |
| `testAlertLine` | Parses CSV line with `ALERT` status: `31.2,70.0,18.0,7.0,790.0,ALERT` | Returns true, status marked `ALERT` |
| `testCarriageReturn` | Parses CSV line terminated with CRLF (`\r\n`) | Returns true, ignores carriage return |
| `testEmptyLine` | Parses empty string `""` | Returns false |
| `testMissingFields` | Parses truncated CSV line with only 4 fields | Returns false (requires 6) |
| `testExtraFields` | Parses CSV line with 7 fields | Returns false |
| `testNonNumericValue` | Parses line with alphanumeric token `abc` in place of temperature | Returns false, catches exception |
| `testInvalidStatus` | Parses line with unrecognized status token `UNKNOWN` | Returns false |
| `testValidateNormalReading`| Validates reading within physical ranges | Returns true |
| `testValidateOutOfRange` | Tests reading with temperature at 200°C | Returns false |
| `testValidateNegativeHumidity` | Tests reading with relative humidity at -5% | Returns false |
| `testTimestamp` | Validates ISO timestamp generation format | Produces non-empty 19-char string |

### 2.2 Alert Engine Tests (`tests/alert_test.cpp`)
| Test Case | Condition Under Test | Expected Alert |
|:---|:---|:---|
| `testNormalReadingNoAlerts` | All parameters within acceptable boundaries | 0 alerts |
| `testTemperatureTooHigh` | Temperature = 45.0°C ($> 40.0^\circ\text{C}$) | `Temperature TOO_HIGH` |
| `testTemperatureTooLow` | Temperature = 5.0°C ($< 10.0^\circ\text{C}$) | `Temperature TOO_LOW` |
| `testHumidityTooHigh` | Humidity = 85.0% ($> 80.0\%$) | `Humidity TOO_HIGH` |
| `testHumidityTooLow` | Humidity = 20.0% ($< 30.0\%$) | `Humidity TOO_LOW` |
| `testSoilMoistureTooLow` | Soil Moisture = 15.0% ($< 25.0\%$) | `Soil Moisture TOO_LOW` |
| `testPhTooLow` | pH = 4.0 ($< 5.5$) | `pH TOO_LOW` |
| `testPhTooHigh` | pH = 9.0 ($> 8.0$) | `pH TOO_HIGH` |
| `testMultipleAlerts` | Simultaneous out-of-range parameters | 4 separate structured alerts |
| `testBoundaryValues` | Parameters set exactly on threshold boundaries | 0 alerts (strictly inclusive) |
| `testFormatAlert` | String serialization test | Correctly formatted human-readable alert |

### 2.3 Database Tests (`tests/database_test.cpp`)
| Test Case | Description | Expected Outcome |
|:---|:---|:---|
| `testOpenDatabase` | Creates temporary test database and table schema | Returns true |
| `testInsertAndRetrieve` | Inserts reading, retrieves newest record | Values match inserted floats |
| `testHistory` | Inserts 5 records, retrieves 3 newest | Returns 3 records in reverse chronological order |
| `testInsertAndRetrieveAlert` | Inserts alert record, reads back | Fields and parameters match |
| `testEmptyDatabase` | Queries latest reading on empty database | Returns false gracefully |
| `testMultipleInserts` | Performs 100 consecutive inserts | All 100 records preserved |

### 2.4 HTTP Server Integration Tests (`tests/http_server_test.cpp`)
| Test Case | Request / Scenario | Expected Response |
|:---|:---|:---|
| `testServerStartup` | Connects TCP socket to `127.0.0.1:8081` | Connection accepted |
| `testDashboardResponse` | `GET / HTTP/1.1` | `HTTP/1.1 200 OK`, `Content-Type: text/html` |
| `testStatusRoute` | `GET /status HTTP/1.1` | `HTTP/1.1 200 OK` |
| `testAlertsRoute` | `GET /alerts HTTP/1.1` | `HTTP/1.1 200 OK` |
| `testHistoryRoute` | `GET /history HTTP/1.1` | `HTTP/1.1 200 OK` |
| `test404Response` | `GET /invalid HTTP/1.1` | `HTTP/1.1 404 Not Found` |
| `test405Response` | `POST / HTTP/1.1` | `HTTP/1.1 405 Method Not Allowed` |
| `testContentLength` | Checks HTTP response headers | `Content-Length:` header present |
| `testConnectionClose` | Checks HTTP connection policy | `Connection: close` header present |
| `testDashboardWithData`| Inserts reading, requests dashboard | Reading value rendered in HTML body |

---

## 3. Running the Test Suite on Linux

```bash
# 1. Create build directory
mkdir -p build && cd build

# 2. Configure with CMake
cmake ..

# 3. Compile all targets
make -j$(nproc)

# 4. Run tests with CTest
ctest --output-on-failure

# Or execute individual test binaries directly:
./parser_test
./alert_test
./database_test
./http_server_test
```
