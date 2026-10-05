#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "database.h"
#include <string>
#include <atomic>

// Minimal HTTP server using Linux POSIX sockets.
// Binds to 127.0.0.1:8080 and serves the HTML dashboard.
class HttpServer {
public:
    HttpServer(Database& db, int port = 8080);
    ~HttpServer();

    // Start listening. Blocks and handles connections until stop() is called.
    bool start();

    // Signal the server to stop accepting new connections.
    void stop();

    // Check if server is running.
    bool isRunning() const;

    // Set serial connection status (for status page).
    void setSerialConnected(bool connected);

private:
    Database& db_;
    int port_;
    int serverFd_;
    std::atomic<bool> running_;
    bool serialConnected_;

    // Handle a single client connection.
    void handleClient(int clientFd);

    // Parse HTTP request and extract method and path.
    bool parseRequest(const std::string& request, std::string& method, std::string& path);

    // Send an HTTP response to the client.
    void sendResponse(int clientFd, int statusCode, const std::string& statusText,
                      const std::string& contentType, const std::string& body);
};

#endif // HTTP_SERVER_H
