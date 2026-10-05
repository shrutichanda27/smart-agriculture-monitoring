# Stage 4 – Initial Implementation & Prototype

## 1. Prototype Objective
The prototype integrates the sensor simulation, Linux/C++ application, database, alert processing, and HTTP dashboard.

## 2. Implemented Components
- Arduino/Tinkercad sensor simulation
- Linux serial-device handling
- C++17 application
- Sensor processing
- Alert engine
- SQLite database
- HTTP server
- Dashboard
- Demo mode

## 3. Prototype Execution
The application can be run in demo mode using:

```bash
./smart_agriculture --demo
```

The local HTTP server is available at:

```text
http://127.0.0.1:8080
```

## 4. Prototype Behavior
When the serial device is unavailable, the application can continue in a server-only/demo configuration rather than terminating unexpectedly.

The demo mode provides data for validating:
- Sensor processing
- Alert generation
- Database insertion
- Dashboard rendering

## 5. Progress Evidence
Recommended evidence to attach to this stage:
- Tinkercad circuit screenshot
- Arduino Serial Monitor screenshot
- Successful CMake build
- Application startup screenshot
- Dashboard screenshot
- Demo-mode screenshot

## 6. Issues and Solutions
### Issue: `/dev/ttyACM0` unavailable
**Cause:** No physical Arduino was connected to the Linux environment.

**Solution:** Use the application's demo mode for software-side testing and use the physical device when available.

### Issue: SQLite development package missing
**Cause:** SQLite development headers/libraries were not installed.

**Solution:**
```bash
sudo apt install libsqlite3-dev sqlite3
```

## 7. Prototype Result
The prototype successfully demonstrated the end-to-end Linux/C++ software pipeline without requiring a physical Arduino for demo-mode execution.
