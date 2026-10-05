# Stage 5 – Testing, Integration & Improvement

## 1. Testing Strategy

Testing covers:
- Build verification
- Unit/module tests
- Database integration
- HTTP server behavior
- Sensor-data processing
- Alert processing
- Demo-mode execution
- Error handling

## 2. Automated Test Result

The project was successfully built and the automated test suite produced:

```text
4/4 tests passed
0 tests failed
```

This result should be preserved as Stage 5 evidence.

## 3. Database Verification

The SQLite database contains:
- `sensor_data`
- `alerts`

Example verification:

```sql
.tables
SELECT * FROM sensor_data;
SELECT * FROM alerts;
```

## 4. HTTP Verification

The local server is tested using:

```bash
curl http://127.0.0.1:8080
```

and through a browser at:

```text
http://127.0.0.1:8080
```

## 5. Error Handling

The system was tested with the serial device unavailable. The application reported that `/dev/ttyACM0` was not found and continued by starting the HTTP server rather than crashing.

## 6. Integration Flow

```text
Sensor/Demo Data
      ↓
Serial Reader / Demo Input
      ↓
Sensor Processor
      ↓
Alert Engine
      ↓
SQLite
      ↓
HTTP Server
      ↓
Dashboard
```

## 7. Improvement Areas
- Improve sensor calibration for physical hardware.
- Add more comprehensive stress tests.
- Improve authentication if the dashboard is exposed beyond localhost.
- Add configurable thresholds through a dedicated configuration interface.
- Add a custom Linux kernel module only if the academic requirement explicitly demands custom driver development.

## 8. Final Test Checklist

| Test | Expected Result | Status |
|---|---|---|
| CMake configure | Successful | PASS |
| C++ build | Successful | PASS |
| Automated tests | 4/4 passed | PASS |
| Demo mode | Starts correctly | PASS |
| SQLite | Data stored | PASS |
| HTTP server | Port 8080 available | PASS |
| Dashboard | Loads in browser | PASS |
| Missing serial device | Graceful handling | PASS |
