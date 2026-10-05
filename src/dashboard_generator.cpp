#include "dashboard_generator.h"
#include <sstream>
#include <iomanip>
#include <cmath>

std::string DashboardGenerator::escapeHTML(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            default:   result += c;
        }
    }
    return result;
}

std::string DashboardGenerator::getCSS() {
    return R"CSS(
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: linear-gradient(135deg, #0f4c2e 0%, #1a6b3c 30%, #2d8f5e 60%, #1b5e3a 100%);
    min-height: 100vh;
    color: #e8f5e9;
}
.header {
    background: rgba(0,0,0,0.3);
    backdrop-filter: blur(10px);
    padding: 20px 40px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-bottom: 1px solid rgba(255,255,255,0.1);
}
.header h1 {
    font-size: 1.6em;
    font-weight: 600;
    letter-spacing: 0.5px;
}
.header h1 span { color: #66bb6a; }
.header .status-badge {
    padding: 6px 18px;
    border-radius: 20px;
    font-size: 0.85em;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 1px;
}
.status-normal { background: #2e7d32; color: #c8e6c9; }
.status-alert { background: #c62828; color: #ffcdd2; animation: pulse 1.5s infinite; }
@keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.6; }
}
.container { max-width: 1200px; margin: 0 auto; padding: 30px 20px; }
.cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-bottom: 30px; }
.card {
    background: rgba(255,255,255,0.08);
    backdrop-filter: blur(8px);
    border: 1px solid rgba(255,255,255,0.12);
    border-radius: 16px;
    padding: 24px;
    text-align: center;
    transition: transform 0.2s, box-shadow 0.2s;
}
.card:hover {
    transform: translateY(-4px);
    box-shadow: 0 8px 32px rgba(0,0,0,0.3);
}
.card .icon { font-size: 2em; margin-bottom: 8px; }
.card .label {
    font-size: 0.8em;
    text-transform: uppercase;
    letter-spacing: 1.5px;
    color: rgba(255,255,255,0.6);
    margin-bottom: 8px;
}
.card .value {
    font-size: 2.2em;
    font-weight: 700;
    color: #a5d6a7;
}
.card .unit { font-size: 0.5em; color: rgba(255,255,255,0.5); }
.card.alert-card .value { color: #ef9a9a; }
.section {
    background: rgba(255,255,255,0.06);
    backdrop-filter: blur(8px);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 16px;
    padding: 24px;
    margin-bottom: 24px;
}
.section h2 {
    font-size: 1.1em;
    margin-bottom: 16px;
    color: #a5d6a7;
    border-bottom: 1px solid rgba(255,255,255,0.1);
    padding-bottom: 10px;
}
table { width: 100%; border-collapse: collapse; }
th, td { padding: 10px 14px; text-align: left; }
th {
    font-size: 0.75em;
    text-transform: uppercase;
    letter-spacing: 1px;
    color: rgba(255,255,255,0.5);
    border-bottom: 1px solid rgba(255,255,255,0.15);
}
td { border-bottom: 1px solid rgba(255,255,255,0.05); font-size: 0.9em; }
.alert-row { color: #ef9a9a; }
.normal-row { color: #c8e6c9; }
.footer {
    text-align: center;
    padding: 20px;
    font-size: 0.8em;
    color: rgba(255,255,255,0.3);
}
.no-data { text-align: center; padding: 40px; color: rgba(255,255,255,0.4); font-style: italic; }
.meta-info { display: flex; gap: 20px; justify-content: center; margin-bottom: 20px; font-size: 0.85em; color: rgba(255,255,255,0.5); }
)CSS";
}

std::string DashboardGenerator::generateDashboard(
        const SensorReading& latest,
        const std::vector<SensorReading>& history,
        const std::vector<Alert>& alerts,
        bool hasData) {

    std::ostringstream html;
    html << std::fixed << std::setprecision(1);

    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << "<title>Smart Agriculture Monitoring System</title>\n"
         << "<meta name=\"description\" content=\"Real-time agricultural sensor monitoring dashboard showing temperature, humidity, soil moisture, water level, and light readings.\">\n"
         << "<style>" << getCSS() << "</style>\n"
         << "</head>\n<body>\n";

    // Header
    bool isAlert = hasData && latest.status == "ALERT";
    html << "<div class=\"header\">\n"
         << "  <h1>&#127793; Smart <span>Agriculture</span> Monitor</h1>\n"
         << "  <div class=\"status-badge " << (isAlert ? "status-alert" : "status-normal") << "\">"
         << (hasData ? escapeHTML(latest.status) : "NO DATA") << "</div>\n"
         << "</div>\n";

    html << "<div class=\"container\">\n";

    if (!hasData) {
        html << "<div class=\"no-data\">\n"
             << "  <p style=\"font-size:2em;\">&#128268;</p>\n"
             << "  <p>Waiting for sensor data...</p>\n"
             << "  <p>Connect Arduino to /dev/ttyACM0 and start the serial reader.</p>\n"
             << "</div>\n";
    } else {
        // Meta info
        html << "<div class=\"meta-info\">\n"
             << "  <span>Last Updated: " << escapeHTML(latest.timestamp) << "</span>\n"
             << "  <span>Refresh page to update</span>\n"
             << "</div>\n";

        // Sensor cards
        html << "<div class=\"cards\">\n";

        // Temperature (Alert if > 30°C or < 10°C)
        bool tempAlert = latest.temperature > 30.0f || latest.temperature < 10.0f;
        html << "  <div class=\"card" << (tempAlert ? " alert-card" : "") << "\" id=\"card-temperature\">\n"
             << "    <div class=\"icon\">&#127777;&#65039;</div>\n"
             << "    <div class=\"label\">Temperature</div>\n"
             << "    <div class=\"value\">" << latest.temperature << "<span class=\"unit\"> &deg;C</span></div>\n"
             << "  </div>\n";

        // Humidity (Alert if < 400 or < 40%)
        float humThresh = (latest.humidity <= 100.0f) ? 40.0f : 400.0f;
        bool humAlert = latest.humidity < humThresh;
        html << "  <div class=\"card" << (humAlert ? " alert-card" : "") << "\" id=\"card-humidity\">\n"
             << "    <div class=\"icon\">&#128167;</div>\n"
             << "    <div class=\"label\">Humidity</div>\n"
             << "    <div class=\"value\">" << latest.humidity << "</div>\n"
             << "  </div>\n";

        // Soil Moisture (Alert if < 400 or < 40%)
        float soilThresh = (latest.soilMoisture <= 100.0f) ? 40.0f : 400.0f;
        bool soilAlert = latest.soilMoisture < soilThresh;
        html << "  <div class=\"card" << (soilAlert ? " alert-card" : "") << "\" id=\"card-soil\">\n"
             << "    <div class=\"icon\">&#127807;</div>\n"
             << "    <div class=\"label\">Soil Moisture</div>\n"
             << "    <div class=\"value\">" << latest.soilMoisture << "</div>\n"
             << "  </div>\n";

        // Water Level (Alert if < 400 or < 40%)
        float waterThresh = (latest.waterLevel <= 100.0f) ? 40.0f : 400.0f;
        bool waterAlert = latest.waterLevel < waterThresh;
        html << "  <div class=\"card" << (waterAlert ? " alert-card" : "") << "\" id=\"card-water\">\n"
             << "    <div class=\"icon\">&#128166;</div>\n"
             << "    <div class=\"label\">Water Level</div>\n"
             << "    <div class=\"value\">" << latest.waterLevel << "</div>\n"
             << "  </div>\n";

        // Light
        html << "  <div class=\"card\" id=\"card-light\">\n"
             << "    <div class=\"icon\">&#9728;&#65039;</div>\n"
             << "    <div class=\"label\">Light Intensity</div>\n"
             << "    <div class=\"value\">" << latest.light << "</div>\n"
             << "  </div>\n";

        html << "</div>\n"; // .cards

        // Alerts section
        html << "<div class=\"section\" id=\"alerts-section\">\n"
             << "  <h2>&#128680; Recent Alerts</h2>\n";
        if (alerts.empty()) {
            html << "  <p class=\"normal-row\">No active alerts. All parameters within normal range.</p>\n";
        } else {
            html << "  <table>\n"
                 << "    <tr><th>Time</th><th>Parameter</th><th>Condition</th><th>Value</th><th>Threshold</th></tr>\n";
            for (const auto& a : alerts) {
                html << "    <tr class=\"alert-row\">"
                     << "<td>" << escapeHTML(a.timestamp) << "</td>"
                     << "<td>" << escapeHTML(a.parameter) << "</td>"
                     << "<td>" << escapeHTML(a.condition) << "</td>"
                     << "<td>" << a.value << "</td>"
                     << "<td>" << a.threshold << "</td>"
                     << "</tr>\n";
            }
            html << "  </table>\n";
        }
        html << "</div>\n";

        // History section
        if (!history.empty()) {
            html << "<div class=\"section\" id=\"history-section\">\n"
                 << "  <h2>&#128202; Sensor History</h2>\n"
                 << "  <table>\n"
                 << "    <tr><th>Time</th><th>Temp (&deg;C)</th><th>Humidity</th>"
                 << "<th>Soil Moisture</th><th>Water Level</th><th>Light</th><th>Status</th></tr>\n";
            for (const auto& r : history) {
                bool rowAlert = (r.status == "ALERT");
                html << "    <tr class=\"" << (rowAlert ? "alert-row" : "normal-row") << "\">"
                     << "<td>" << escapeHTML(r.timestamp) << "</td>"
                     << "<td>" << r.temperature << "</td>"
                     << "<td>" << r.humidity << "</td>"
                     << "<td>" << r.soilMoisture << "</td>"
                     << "<td>" << r.waterLevel << "</td>"
                     << "<td>" << r.light << "</td>"
                     << "<td>" << escapeHTML(r.status) << "</td>"
                     << "</tr>\n";
            }
            html << "  </table>\n"
                 << "</div>\n";
        }
    }

    html << "</div>\n"; // .container

    html << "<div class=\"footer\">\n"
         << "  Smart Agriculture Monitoring System &mdash; C++ / Linux / SQLite / TCP/HTTP<br>\n"
         << "  Server: 127.0.0.1:8080 &mdash; Local access only\n"
         << "</div>\n";

    html << "</body>\n</html>";
    return html.str();
}

std::string DashboardGenerator::generateStatusPage(bool serverRunning, bool dbConnected, bool serialConnected) {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<title>System Status</title>\n"
         << "<style>" << getCSS() << "</style>\n"
         << "</head>\n<body>\n"
         << "<div class=\"header\"><h1>&#128736; System <span>Status</span></h1></div>\n"
         << "<div class=\"container\">\n"
         << "<div class=\"section\">\n"
         << "<h2>Component Status</h2>\n"
         << "<table>\n"
         << "<tr><th>Component</th><th>Status</th></tr>\n"
         << "<tr><td>HTTP Server</td><td class=\"" << (serverRunning ? "normal-row" : "alert-row") << "\">"
         << (serverRunning ? "Running" : "Stopped") << "</td></tr>\n"
         << "<tr><td>Database</td><td class=\"" << (dbConnected ? "normal-row" : "alert-row") << "\">"
         << (dbConnected ? "Connected" : "Disconnected") << "</td></tr>\n"
         << "<tr><td>Serial Port</td><td class=\"" << (serialConnected ? "normal-row" : "alert-row") << "\">"
         << (serialConnected ? "Connected" : "Disconnected") << "</td></tr>\n"
         << "</table>\n"
         << "</div>\n</div>\n"
         << "</body>\n</html>";
    return html.str();
}

std::string DashboardGenerator::generate404Page(const std::string& path) {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<title>404 Not Found</title>\n"
         << "<style>" << getCSS() << "</style>\n"
         << "</head>\n<body>\n"
         << "<div class=\"header\"><h1>404 &mdash; Not Found</h1></div>\n"
         << "<div class=\"container\">\n"
         << "<div class=\"no-data\">\n"
         << "<p style=\"font-size:3em;\">&#128533;</p>\n"
         << "<p>The path <strong>" << escapeHTML(path) << "</strong> was not found.</p>\n"
         << "<p><a href=\"/\" style=\"color:#66bb6a;\">Return to Dashboard</a></p>\n"
         << "</div>\n</div>\n"
         << "</body>\n</html>";
    return html.str();
}

std::string DashboardGenerator::generate405Page(const std::string& method) {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<title>405 Method Not Allowed</title>\n"
         << "<style>" << getCSS() << "</style>\n"
         << "</head>\n<body>\n"
         << "<div class=\"header\"><h1>405 &mdash; Method Not Allowed</h1></div>\n"
         << "<div class=\"container\">\n"
         << "<div class=\"no-data\">\n"
         << "<p>HTTP method <strong>" << escapeHTML(method) << "</strong> is not supported.</p>\n"
         << "<p>Only GET requests are accepted.</p>\n"
         << "</div>\n</div>\n"
         << "</body>\n</html>";
    return html.str();
}
