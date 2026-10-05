#ifndef SENSOR_PROCESSOR_H
#define SENSOR_PROCESSOR_H

#include <string>

// Represents one complete sensor reading matching the Tinkercad simulation
struct SensorReading {
    float temperature;   // TMP36 reading in °C
    float humidity;      // Humidity sensor value (raw ADC / scaled)
    float soilMoisture;  // Soil moisture sensor value (raw ADC / scaled)
    float waterLevel;    // Water reservoir level value (raw ADC / scaled)
    float light;         // Light intensity value (raw ADC / scaled)
    std::string status;  // System status: "NORMAL" or "ALERT"
    std::string timestamp; // Formatted ISO timestamp (YYYY-MM-DD HH:MM:SS)
};

// Parse a sensor line from Arduino serial output.
// Supports both the Tinkercad simulation pipe-delimited format:
//   "Light: <val> | Temperature: <val> C | Soil Moisture: <val> | Humidity: <val> | Water Level: <val>"
// and standard comma-separated format:
//   "temperature,humidity,soilMoisture,waterLevel,light,status"
// Returns true on success and populates `reading`. Returns false on malformed input.
bool parseSensorLine(const std::string& line, SensorReading& reading);

// Validate that numeric fields are within physically plausible ADC / sensor ranges.
// Returns true if all values are within acceptable bounds.
bool validateReading(const SensorReading& reading);

// Get current timestamp as ISO 8601 string (YYYY-MM-DD HH:MM:SS).
std::string getCurrentTimestamp();

#endif // SENSOR_PROCESSOR_H
