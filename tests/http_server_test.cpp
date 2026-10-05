/*
 * HTTP Server Tests
 * Tests for socket startup, HTTP responses, routing, and error handling.
 *
 * These tests start the HTTP server in a background thread, connect to it
 * using a raw TCP socket, and verify responses.
 */

#include "../src/http_server.h"
#include "../src/database.h"
#include "../src/sensor_processor.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) \
    std::cout << "  TEST: " << name << " ... ";

#define PASS() \
    std::cout << "PASSED" << std::endl; testsPassed++;

#define FAIL(msg) \
    std::cout << "FAILED: " << msg << std::endl; testsFailed++;

static const char* TEST_DB = "test_http_server.db";
static const int TEST_PORT = 8081;  // Use different port to avoid conflicts

// Send an HTTP request to the server and return the response
static std::string sendRequest(const std::string& request) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TEST_PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(sock);
        return "";
    }

    send(sock, request.c_str(), request.size(), 0);

    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        response += buffer;
    }

    ::close(sock);
    return response;
}

void testServerStartup(Database& db) {
    TEST("Server binds and accepts connections");
    std::string response = sendRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.empty()) { FAIL("no response received"); return; }
    if (response.find("HTTP/1.1 200 OK") == std::string::npos) {
        FAIL("expected 200 OK");
        return;
    }
    PASS();
}

void testDashboardResponse() {
    TEST("GET / returns HTML dashboard");
    std::string response = sendRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("Content-Type: text/html") == std::string::npos) {
        FAIL("expected Content-Type: text/html");
        return;
    }
    if (response.find("Smart Agriculture") == std::string::npos) {
        FAIL("expected dashboard title in HTML");
        return;
    }
    PASS();
}

void testStatusRoute() {
    TEST("GET /status returns 200");
    std::string response = sendRequest("GET /status HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("HTTP/1.1 200 OK") == std::string::npos) {
        FAIL("expected 200 OK for /status");
        return;
    }
    PASS();
}

void testAlertsRoute() {
    TEST("GET /alerts returns 200");
    std::string response = sendRequest("GET /alerts HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("HTTP/1.1 200 OK") == std::string::npos) {
        FAIL("expected 200 OK for /alerts");
        return;
    }
    PASS();
}

void testHistoryRoute() {
    TEST("GET /history returns 200");
    std::string response = sendRequest("GET /history HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("HTTP/1.1 200 OK") == std::string::npos) {
        FAIL("expected 200 OK for /history");
        return;
    }
    PASS();
}

void test404Response() {
    TEST("GET /invalid returns 404");
    std::string response = sendRequest("GET /invalid HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("HTTP/1.1 404") == std::string::npos) {
        FAIL("expected 404 for /invalid");
        return;
    }
    PASS();
}

void test405Response() {
    TEST("POST / returns 405");
    std::string response = sendRequest("POST / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("HTTP/1.1 405") == std::string::npos) {
        FAIL("expected 405 for POST");
        return;
    }
    PASS();
}

void testContentLength() {
    TEST("Response includes Content-Length header");
    std::string response = sendRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("Content-Length:") == std::string::npos) {
        FAIL("expected Content-Length header");
        return;
    }
    PASS();
}

void testConnectionClose() {
    TEST("Response includes Connection: close");
    std::string response = sendRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("Connection: close") == std::string::npos) {
        FAIL("expected Connection: close header");
        return;
    }
    PASS();
}

void testDashboardWithData(Database& db) {
    TEST("Dashboard displays sensor data after insert");

    // Insert a reading
    SensorReading r;
    r.timestamp = "2025-01-01 12:00:00";
    r.temperature = 28.5f;
    r.humidity = 600.0f;
    r.soilMoisture = 450.0f;
    r.waterLevel = 610.0f;
    r.light = 720.0f;
    r.status = "NORMAL";
    db.insertReading(r);

    std::string response = sendRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    if (response.find("28.5") == std::string::npos || response.find("Water Level") == std::string::npos) {
        FAIL("expected temperature and Water Level in dashboard");
        return;
    }
    PASS();
}

int main() {
    std::cout << "=== HTTP Server Tests ===" << std::endl;

    // Setup
    std::remove(TEST_DB);
    Database db(TEST_DB);
    if (!db.open()) {
        std::cerr << "Failed to open test database" << std::endl;
        return 1;
    }

    // Start server in background thread
    HttpServer server(db, TEST_PORT);
    std::thread serverThread([&server]() {
        server.start();
    });

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run tests
    testServerStartup(db);
    testDashboardResponse();
    testStatusRoute();
    testAlertsRoute();
    testHistoryRoute();
    test404Response();
    test405Response();
    testContentLength();
    testConnectionClose();
    testDashboardWithData(db);

    // Cleanup
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }
    db.close();
    std::remove(TEST_DB);

    std::cout << "\nResults: " << testsPassed << " passed, "
              << testsFailed << " failed" << std::endl;

    return (testsFailed > 0) ? 1 : 0;
}
