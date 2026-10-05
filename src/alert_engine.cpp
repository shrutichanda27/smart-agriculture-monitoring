#include "alert_engine.h"
#include <sstream>

AlertEngine::AlertEngine()
    : tempMin_(10.0f), tempMax_(30.0f),
      humidityMin_(400.0f),
      soilMin_(400.0f),
      waterMin_(400.0f) {}

std::vector<Alert> AlertEngine::checkThresholds(const SensorReading& reading) const {
    std::vector<Alert> alerts;

    // 1. Temperature checks (Matches Tinkercad LED3 D10: temp > 30°C)
    if (reading.temperature > tempMax_) {
        Alert a;
        a.timestamp = reading.timestamp;
        a.parameter = "Temperature";
        a.condition = "TOO_HIGH";
        a.value = reading.temperature;
        a.threshold = tempMax_;
        a.message = "Temperature too high: " + std::to_string(reading.temperature) + " C (threshold: > " + std::to_string(tempMax_) + " C)";
        alerts.push_back(a);
    } else if (reading.temperature < tempMin_) {
        Alert a;
        a.timestamp = reading.timestamp;
        a.parameter = "Temperature";
        a.condition = "TOO_LOW";
        a.value = reading.temperature;
        a.threshold = tempMin_;
        a.message = "Temperature too low: " + std::to_string(reading.temperature) + " C (threshold: < " + std::to_string(tempMin_) + " C)";
        alerts.push_back(a);
    }

    // 2. Soil Moisture check (Matches Tinkercad LED1 D12: soil < 400)
    float effectiveSoilThreshold = (reading.soilMoisture <= 100.0f) ? 40.0f : soilMin_;
    if (reading.soilMoisture < effectiveSoilThreshold) {
        Alert a;
        a.timestamp = reading.timestamp;
        a.parameter = "Soil Moisture";
        a.condition = "TOO_LOW";
        a.value = reading.soilMoisture;
        a.threshold = effectiveSoilThreshold;
        a.message = "Soil moisture critically low: " + std::to_string(reading.soilMoisture) + " (min: " + std::to_string(effectiveSoilThreshold) + ")";
        alerts.push_back(a);
    }

    // 3. Humidity check (Matches Tinkercad LED2 D11: humidity < 400)
    float effectiveHumidityThreshold = (reading.humidity <= 100.0f) ? 40.0f : humidityMin_;
    if (reading.humidity < effectiveHumidityThreshold) {
        Alert a;
        a.timestamp = reading.timestamp;
        a.parameter = "Humidity";
        a.condition = "TOO_LOW";
        a.value = reading.humidity;
        a.threshold = effectiveHumidityThreshold;
        a.message = "Humidity too low: " + std::to_string(reading.humidity) + " (min: " + std::to_string(effectiveHumidityThreshold) + ")";
        alerts.push_back(a);
    }

    // 4. Water Level check (Matches Tinkercad LED4 D9: water < 400)
    float effectiveWaterThreshold = (reading.waterLevel <= 100.0f) ? 40.0f : waterMin_;
    if (reading.waterLevel < effectiveWaterThreshold) {
        Alert a;
        a.timestamp = reading.timestamp;
        a.parameter = "Water Level";
        a.condition = "TOO_LOW";
        a.value = reading.waterLevel;
        a.threshold = effectiveWaterThreshold;
        a.message = "Water reservoir critically low: " + std::to_string(reading.waterLevel) + " (min: " + std::to_string(effectiveWaterThreshold) + ")";
        alerts.push_back(a);
    }

    return alerts;
}

std::string AlertEngine::formatAlert(const Alert& alert) {
    std::ostringstream oss;
    oss << "[" << alert.timestamp << "] "
        << alert.parameter << " " << alert.condition
        << ": " << alert.value
        << " (threshold: " << alert.threshold << ")";
    return oss.str();
}
