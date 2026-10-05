#include "sensor_processor.h"
#include <sstream>
#include <vector>
#include <ctime>
#include <stdexcept>
#include <iostream>

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

bool parseSensorLine(const std::string& line, SensorReading& reading) {
    // Trim trailing whitespace / carriage return
    std::string trimmed = line;
    while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n' || trimmed.back() == ' ')) {
        trimmed.pop_back();
    }

    if (trimmed.empty()) {
        return false;
    }

    // 1. Check if line is the pipe-delimited format from the Tinkercad simulation:
    // "Light: 512 | Temperature: 24.50 C | Soil Moisture: 450 | Humidity: 600 | Water Level: 550"
    if (trimmed.find('|') != std::string::npos) {
        std::vector<std::string> segments;
        std::istringstream segStream(trimmed);
        std::string seg;
        while (std::getline(segStream, seg, '|')) {
            segments.push_back(seg);
        }

        if (segments.size() == 5) {
            float light = 0.0f, temp = 0.0f, soil = 0.0f, hum = 0.0f, water = 0.0f;
            bool parsedAll = true;

            for (const auto& s : segments) {
                size_t colon = s.find(':');
                if (colon == std::string::npos) {
                    parsedAll = false;
                    break;
                }
                std::string key = s.substr(0, colon);
                std::string valStr = s.substr(colon + 1);

                // Trim leading/trailing whitespace
                while (!key.empty() && key.front() == ' ') key.erase(key.begin());
                while (!key.empty() && key.back() == ' ') key.pop_back();
                while (!valStr.empty() && valStr.front() == ' ') valStr.erase(valStr.begin());
                while (!valStr.empty() && valStr.back() == ' ') valStr.pop_back();

                // Strip trailing 'C' or 'c' from temperature string
                if (!valStr.empty() && (valStr.back() == 'C' || valStr.back() == 'c')) {
                    valStr.pop_back();
                    while (!valStr.empty() && valStr.back() == ' ') valStr.pop_back();
                }

                try {
                    float val = std::stof(valStr);
                    if (key == "Light") light = val;
                    else if (key == "Temperature") temp = val;
                    else if (key == "Soil Moisture") soil = val;
                    else if (key == "Humidity") hum = val;
                    else if (key == "Water Level") water = val;
                    else parsedAll = false;
                } catch (...) {
                    parsedAll = false;
                    break;
                }
            }

            if (parsedAll) {
                reading.temperature  = temp;
                reading.humidity     = hum;
                reading.soilMoisture = soil;
                reading.waterLevel   = water;
                reading.light        = light;

                // Thresholds directly from the Tinkercad Arduino firmware:
                // Soil Moisture < 400 (LED1 D12)
                // Humidity < 400 (LED2 D11)
                // Temperature > 30°C (LED3 D10)
                // Water Level < 400 (LED4 D9)
                bool alertActive = (soil < 400.0f || hum < 400.0f || temp > 30.0f || water < 400.0f);
                reading.status = alertActive ? "ALERT" : "NORMAL";
                reading.timestamp = getCurrentTimestamp();
                return true;
            }
        }
    }

    // 2. Parse standard CSV format:
    // temperature,humidity,soilMoisture,waterLevel,light,status
    std::vector<std::string> fields;
    std::istringstream stream(trimmed);
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }

    // Expect exactly 6 fields
    if (fields.size() != 6) {
        std::cerr << "[SensorProcessor] Invalid field count: " << fields.size()
                  << " (expected 6)" << std::endl;
        return false;
    }

    // Parse numeric fields
    try {
        reading.temperature  = std::stof(fields[0]);
        reading.humidity     = std::stof(fields[1]);
        reading.soilMoisture = std::stof(fields[2]);
        reading.waterLevel   = std::stof(fields[3]);
        reading.light        = std::stof(fields[4]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "[SensorProcessor] Non-numeric value in: " << trimmed << std::endl;
        return false;
    } catch (const std::out_of_range& e) {
        std::cerr << "[SensorProcessor] Value out of range in: " << trimmed << std::endl;
        return false;
    }

    // Status field
    reading.status = fields[5];
    if (reading.status != "NORMAL" && reading.status != "ALERT") {
        std::cerr << "[SensorProcessor] Invalid status: " << reading.status << std::endl;
        return false;
    }

    // Timestamp
    reading.timestamp = getCurrentTimestamp();

    return true;
}

bool validateReading(const SensorReading& reading) {
    // Temperature: TMP36 range -40 to +125 °C
    if (reading.temperature < -40.0f || reading.temperature > 125.0f) {
        return false;
    }
    // Humidity: 0–1023 (accepts both percentage 0-100% and raw ADC 0-1023)
    if (reading.humidity < 0.0f || reading.humidity > 1023.0f) {
        return false;
    }
    // Soil moisture: 0–1023
    if (reading.soilMoisture < 0.0f || reading.soilMoisture > 1023.0f) {
        return false;
    }
    // Water level: 0–1023
    if (reading.waterLevel < 0.0f || reading.waterLevel > 1023.0f) {
        return false;
    }
    // Light: 0–1023
    if (reading.light < 0.0f || reading.light > 1023.0f) {
        return false;
    }
    return true;
}
