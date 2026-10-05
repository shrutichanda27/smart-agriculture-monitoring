#include "http_server.h"
#include "dashboard_generator.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Maximum HTTP request size (8 KB — prevents abuse)
static const size_t MAX_REQUEST_SIZE = 8192;

HttpServer::HttpServer(Database& db, int port)
    : db_(db), port_(port), serverFd_(-1), running_(false), serialConnected_(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::setSerialConnected(bool connected) {
    serialConnected_ = connected;
}

bool HttpServer::isRunning() const {
    return running_.load();
}

bool HttpServer::start() {
    // 1. Create TCP socket
    serverFd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverFd_ < 0) {
        std::cerr << "[HttpServer] socket() failed: " << strerror(errno) << std::endl;
        return false;
    }

    // 2. Configure socket — allow address reuse
    int opt = 1;
    if (setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[HttpServer] setsockopt() failed: " << strerror(errno) << std::endl;
        ::close(serverFd_);
        serverFd_ = -1;
        return false;
    }

    // 3. Bind to 127.0.0.1:port
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (bind(serverFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[HttpServer] bind() failed on port " << port_
                  << ": " << strerror(errno) << std::endl;
        ::close(serverFd_);
        serverFd_ = -1;
        return false;
    }

    // 4. Listen — backlog of 10
    if (listen(serverFd_, 10) < 0) {
        std::cerr << "[HttpServer] listen() failed: " << strerror(errno) << std::endl;
        ::close(serverFd_);
        serverFd_ = -1;
        return false;
    }

    running_ = true;
    std::cout << "[HttpServer] Listening on http://127.0.0.1:" << port_ << std::endl;

    // 5. Accept loop
    while (running_.load()) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = accept(serverFd_, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
        if (clientFd < 0) {
            if (running_.load()) {
                std::cerr << "[HttpServer] accept() failed: " << strerror(errno) << std::endl;
            }
            continue;
        }

        handleClient(clientFd);
        ::close(clientFd);
    }

    ::close(serverFd_);
    serverFd_ = -1;
    return true;
}

void HttpServer::stop() {
    running_ = false;
    if (serverFd_ >= 0) {
        // Closing the socket unblocks accept()
        shutdown(serverFd_, SHUT_RDWR);
        ::close(serverFd_);
        serverFd_ = -1;
    }
}

void HttpServer::handleClient(int clientFd) {
    // 6. Read HTTP request
    char buffer[MAX_REQUEST_SIZE];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        return;
    }
    buffer[bytesRead] = '\0';
    std::string request(buffer);

    // 7. Parse method and path
    std::string method, path;
    if (!parseRequest(request, method, path)) {
        sendResponse(clientFd, 400, "Bad Request", "text/plain", "Malformed HTTP request");
        return;
    }

    std::cout << "[HttpServer] " << method << " " << path << std::endl;

    // Only accept GET
    if (method != "GET") {
        std::string body = DashboardGenerator::generate405Page(method);
        sendResponse(clientFd, 405, "Method Not Allowed", "text/html", body);
        return;
    }

    // 8. Route request
    if (path == "/" || path == "/index.html") {
        // Main dashboard
        SensorReading latest;
        bool hasData = db_.getLatestReading(latest);
        auto history = db_.getHistory(20);
        auto alerts = db_.getRecentAlerts(10);

        std::string body = DashboardGenerator::generateDashboard(latest, history, alerts, hasData);
        sendResponse(clientFd, 200, "OK", "text/html", body);

    } else if (path == "/status") {
        std::string body = DashboardGenerator::generateStatusPage(true, true, serialConnected_);
        sendResponse(clientFd, 200, "OK", "text/html", body);

    } else if (path == "/alerts") {
        // Alerts-only view — reuse dashboard with empty history
        SensorReading latest;
        bool hasData = db_.getLatestReading(latest);
        std::vector<SensorReading> emptyHistory;
        auto alerts = db_.getRecentAlerts(50);

        std::string body = DashboardGenerator::generateDashboard(latest, emptyHistory, alerts, hasData);
        sendResponse(clientFd, 200, "OK", "text/html", body);

    } else if (path == "/history") {
        // History-only view — reuse dashboard with empty alerts
        SensorReading latest;
        bool hasData = db_.getLatestReading(latest);
        auto history = db_.getHistory(50);
        std::vector<Alert> emptyAlerts;

        std::string body = DashboardGenerator::generateDashboard(latest, history, emptyAlerts, hasData);
        sendResponse(clientFd, 200, "OK", "text/html", body);

    } else {
        // 404
        std::string body = DashboardGenerator::generate404Page(path);
        sendResponse(clientFd, 404, "Not Found", "text/html", body);
    }
}

bool HttpServer::parseRequest(const std::string& request, std::string& method, std::string& path) {
    // HTTP request line: METHOD /path HTTP/1.x\r\n
    std::istringstream stream(request);
    std::string version;
    if (!(stream >> method >> path >> version)) {
        return false;
    }
    // Basic validation
    if (method.empty() || path.empty() || path[0] != '/') {
        return false;
    }
    // Limit path length
    if (path.size() > 256) {
        return false;
    }
    return true;
}

void HttpServer::sendResponse(int clientFd, int statusCode, const std::string& statusText,
                               const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;

    std::string responseStr = response.str();
    send(clientFd, responseStr.c_str(), responseStr.size(), 0);
}
