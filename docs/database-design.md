# Database Design & Schema Specification

## 1. Relational Database Engine

The system uses **SQLite 3**, an embedded, zero-configuration, serverless, transactional SQL database engine. The entire database is stored in a single disk file: `smart_agriculture.db`.

SQLite is linked directly into the C++ executable via the standard C/C++ API (`sqlite3.h`). It requires no separate background server process, administrative users, or network configuration.

---

## 2. Relational Schema Architecture

The database comprises two relational tables: `sensor_data` and `alerts`.

```
+-----------------------------------+        +-----------------------------------+
|            sensor_data            |        |              alerts               |
+-----------------------------------+        +-----------------------------------+
| id            INTEGER (PK AUTO)   |        | id            INTEGER (PK AUTO)   |
| timestamp     TEXT NOT NULL       |        | timestamp     TEXT NOT NULL       |
| temperature   REAL NOT NULL       |        | parameter     TEXT NOT NULL       |
| humidity      REAL NOT NULL       |        | condition     TEXT NOT NULL       |
| soil_moisture REAL NOT NULL       |        | value         REAL NOT NULL       |
| water_level   REAL NOT NULL       |        | threshold     REAL NOT NULL       |
| light         REAL NOT NULL       |        | message       TEXT NOT NULL       |
| status        TEXT NOT NULL       |        +-----------------------------------+
+-----------------------------------+
```

### 2.1 Table: `sensor_data`
Records periodic telemetry packets received from the Arduino microcontroller.

```sql
CREATE TABLE IF NOT EXISTS sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    temperature REAL NOT NULL,
    humidity REAL NOT NULL,
    soil_moisture REAL NOT NULL,
    water_level REAL NOT NULL,
    light REAL NOT NULL,
    status TEXT NOT NULL
);
```

| Column Name | SQL Type | Description |
|:---|:---|:---|
| `id` | `INTEGER` | Synthetic surrogate primary key with auto-increment |
| `timestamp` | `TEXT` | ISO 8601 formatted timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `temperature` | `REAL` | Calibrated ambient temperature in degrees Celsius (°C) |
| `humidity` | `REAL` | Relative air humidity value (0 to 1023 raw / %) |
| `soil_moisture` | `REAL` | Volumetric soil moisture value (0 to 1023 raw / %) |
| `water_level` | `REAL` | Water reservoir level value (0 to 1023 raw / %) |
| `light` | `REAL` | Ambient illuminance in simulated lux / raw ADC |
| `status` | `TEXT` | Overall system operating status (`NORMAL` or `ALERT`) |

### 2.2 Table: `alerts`
Maintains an append-only historical log of threshold breach events.

```sql
CREATE TABLE IF NOT EXISTS alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    parameter TEXT NOT NULL,
    condition TEXT NOT NULL,
    value REAL NOT NULL,
    threshold REAL NOT NULL,
    message TEXT NOT NULL
);
```

| Column Name | SQL Type | Description |
|:---|:---|:---|
| `id` | `INTEGER` | Primary key with auto-increment |
| `timestamp` | `TEXT` | Timestamp when threshold condition was detected |
| `parameter` | `TEXT` | Breached environmental metric (e.g., `Temperature`, `pH`) |
| `condition` | `TEXT` | Threshold violation type (`TOO_HIGH` or `TOO_LOW`) |
| `value` | `REAL` | Exact sensor value that triggered the alert |
| `threshold` | `REAL` | Configured boundary limit value |
| `message` | `TEXT` | Human-readable alert summary string |

---

## 3. C++ SQLite Prepared Statements

To prevent SQL injection and maximize parsing efficiency, all SQL operations are executed using SQLite's compiled statement interface:

1. **`sqlite3_prepare_v2()`**: Compiles the parameterized SQL query string into byte code.
2. **`sqlite3_bind_*()`**: Binds values to placeholders (`?`) safely by value and type.
3. **`sqlite3_step()`**: Steps through statement execution (either inserting a record or returning query rows).
4. **`sqlite3_finalize()`**: Destroys the prepared statement and reclaims associated memory.

### Representative Insert Statement
```cpp
const char* sql =
    "INSERT INTO sensor_data (timestamp, temperature, humidity, soil_moisture, ph, light, status) "
    "VALUES (?, ?, ?, ?, ?, ?, ?);";
```
Parameter bindings explicitly map native C++ data types to SQLite column types, guaranteeing strict type consistency.
