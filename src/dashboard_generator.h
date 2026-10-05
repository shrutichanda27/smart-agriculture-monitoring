#ifndef DASHBOARD_GENERATOR_H
#define DASHBOARD_GENERATOR_H

#include "sensor_processor.h"
#include "alert_engine.h"
#include <string>
#include <vector>

// Generates HTML dashboard pages from sensor data and alerts.
// The generated HTML uses inline CSS referencing the project's design system.
class DashboardGenerator {
public:
    // Generate the main dashboard HTML page showing current readings and alerts.
    static std::string generateDashboard(
        const SensorReading& latest,
        const std::vector<SensorReading>& history,
        const std::vector<Alert>& alerts,
        bool hasData);

    // Generate a simple status page.
    static std::string generateStatusPage(bool serverRunning, bool dbConnected, bool serialConnected);

    // Generate a 404 error page.
    static std::string generate404Page(const std::string& path);

    // Generate a 405 Method Not Allowed page.
    static std::string generate405Page(const std::string& method);

private:
    static std::string getCSS();
    static std::string escapeHTML(const std::string& text);
};

#endif // DASHBOARD_GENERATOR_H
