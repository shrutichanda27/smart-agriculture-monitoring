# Networking Concepts & Linux Socket Implementation

## 1. Executive Summary & Networking Scope

In this project, **networking** does not refer to the public internet, cellular towers, cloud services, or wireless mesh grids. Instead, networking is implemented at its purest and most foundational level: **inter-process communication over local TCP/IP Linux sockets**.

The primary purpose of the networking subsystem is to demonstrate:
- Clean **client/server architecture**.
- The **Linux Berkeley Sockets API** (`sys/socket.h`).
- Fundamental **TCP/IP transport principles**.
- Decoupled presentation using standard **HTTP/1.1** without bloated third-party frameworks.

---

## 2. Core Architecture & Interaction Diagram

```
+-------------------------------------------------------------------------+
|                               HOST COMPUTER                             |
|                                                                         |
|   +-----------------------+              +--------------------------+   |
|   |   Web Browser Client  |              |    C++ HTTP Web Server   |   |
|   | (Chrome / Firefox /   |              |  (src/http_server.cpp)   |   |
|   |  curl / Edge)         |              |                          |   |
|   +-----------------------+              +--------------------------+   |
|               |                                      ^                  |
|               | 1. HTTP GET /                        |                  |
|               |                                      |                  |
|               v                                      v                  |
|   +-------------------------------------------------------------+       |
|   |             Loopback Network Interface (lo)                 |       |
|   |                IPv4 Address: 127.0.0.1                      |       |
|   |                     TCP Port: 8080                          |       |
|   +-------------------------------------------------------------+       |
|               ^                                      |                  |
|               |                                      | 2. Query data    |
|               | 3. HTTP/1.1 200 OK (HTML/CSS)        v                  |
|               |                           +---------------------+       |
|               +---------------------------|   SQLite Database   |       |
|                                           | (smart_agriculture) |       |
|                                           +---------------------+       |
+-------------------------------------------------------------------------+
```

---

## 3. Fundamental Networking Concepts Explained

### 3.1 IP Address
An **Internet Protocol (IP) address** is a unique numerical label assigned to each device connected to a computer network that uses the Internet Protocol for communication. In IPv4, an address consists of 32 bits, typically represented as four decimal numbers separated by dots (e.g., `192.168.1.1` or `127.0.0.1`). It serves two principal functions: host or network interface identification and location addressing.

### 3.2 localhost
`localhost` is a standardized hostname that refers to the local computer currently executing the software. When your web browser accesses `http://localhost:8080`, your computer's name resolver translates `localhost` into the loopback IP address without contacting any external Domain Name System (DNS) server.

### 3.3 127.0.0.1
`127.0.0.1` is the IPv4 loopback address. In IPv4 networking, the entire `127.0.0.0/8` block is reserved for loopback operations. Any network packet sent to `127.0.0.1` never leaves the host machine; the Linux kernel routes it internally from the sending socket directly to the listening socket through the virtual loopback network interface (`lo`).

### 3.4 TCP (Transmission Control Protocol)
**TCP** is a connection-oriented, reliable transport layer protocol residing above IP in the network protocol stack. TCP provides:
1. **Connection-Oriented Flow**: A three-way handshake (`SYN`, `SYN-ACK`, `ACK`) establishes a session before data exchange begins.
2. **Guaranteed Delivery**: Every packet is acknowledged; missing packets are automatically retransmitted.
3. **In-Order Delivery**: Sequence numbers ensure that data received out of order is reconstructed correctly.
4. **Stream-Oriented**: Data is presented as a continuous byte stream without application-level packet boundaries.

### 3.5 Port & Port 8080
A **Port** is a 16-bit number (range: 0–65535) used to distinguish between different network services running on the same operating system:
- Ports 0–1023 are **Well-Known Ports** reserved for privileged system services (e.g., Port 80 for standard HTTP, Port 443 for HTTPS). Binding to these ports requires superuser (`root`) privileges on Linux.
- Ports 1024–49151 are **Registered Ports**.
- **Why Port 8080?** Port 8080 is the universally recognized alternative HTTP development port. Running on port 8080 allows our C++ application to run as an unprivileged, regular user without requiring `sudo` privileges.

### 3.6 Socket
A **Socket** is an operating system abstraction representing an endpoint for two-way communication across a network. In Linux, a socket is manipulated using a standard file descriptor integer, perfectly aligning with Unix's *"Everything is a file"* design paradigm.

### 3.7 Server
A **Server** is a software program or process that waits passively for incoming connection requests from other programs, processes incoming client requests, and sends back responses.

### 3.8 Client
A **Client** is a program (e.g., a web browser or `curl`) that initiates communication by establishing a connection to a server, submitting requests, and waiting for the server's reply.

---

## 4. The Linux Socket System Call Lifecycle

Our C++ server (`src/http_server.cpp`) executes the standard POSIX socket lifecycle:

```
Server Process                                  Client Process (Web Browser)
================                                ============================
socket()        [Create endpoint]
setsockopt()    [Configure SO_REUSEADDR]
bind()          [Attach to 127.0.0.1:8080]
listen()        [Mark as passive socket]
accept()        [Block waiting for client] <------ Connect (Three-way handshake)
                                                   send() [HTTP GET /]
recv()          [Read HTTP request] <-------------
                                                   ...
Generate HTML                                      ...
send()          [Send HTTP/1.1 200 OK + HTML] --->
close(clientFd) [Terminate client connection] ---> recv() [Receives EOF]
accept()        [Loop back to accept next]
```

### Detailed System Call Explanations

1. **`socket()`**:
   ```cpp
   serverFd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   ```
   - Requests a communication descriptor from the kernel.
   - `AF_INET`: Address Family for IPv4.
   - `SOCK_STREAM`: Provides sequenced, reliable, two-way byte streams (specifies TCP).
   - `IPPROTO_TCP`: Explicitly designates the TCP protocol.

2. **`setsockopt()`**:
   ```cpp
   int opt = 1;
   setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   ```
   - Configures socket options.
   - Setting `SO_REUSEADDR` allows immediate reuse of the local port if the server restarts, bypassing the standard TCP `TIME_WAIT` socket retention period.

3. **`bind()`**:
   ```cpp
   bind(serverFd_, (struct sockaddr*)&addr, sizeof(addr));
   ```
   - Binds the socket to a specific local IP address (`127.0.0.1`) and port (`8080`).
   - Ensures the kernel routes traffic arriving on `127.0.0.1:8080` to this specific socket descriptor.

4. **`listen()`**:
   ```cpp
   listen(serverFd_, 10);
   ```
   - Converts an active socket into a passive listening socket capable of accepting incoming connections.
   - The second argument (`10`) defines the **backlog queue length**—the maximum number of pending, unaccepted connections the kernel will queue before rejecting new attempts.

5. **`accept()`**:
   ```cpp
   int clientFd = accept(serverFd_, (struct sockaddr*)&clientAddr, &clientLen);
   ```
   - Blocks (suspends) process execution until a client connects.
   - When a connection arrives, it creates and returns a **brand new file descriptor** (`clientFd`) representing the dedicated connection to that specific client, leaving `serverFd_` open to listen for subsequent connections.

6. **`recv()`**:
   ```cpp
   ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
   ```
   - Reads incoming data transmitted by the client across the TCP channel into a user-space memory buffer.

7. **`send()`**:
   ```cpp
   send(clientFd, responseStr.c_str(), responseStr.size(), 0);
   ```
   - Transmits response data across the socket channel back to the client.

8. **`close()`**:
   ```cpp
   close(clientFd);
   ```
   - Gracefully terminates the client connection, sending a TCP `FIN` packet to complete the four-way termination handshake.

---

## 5. HTTP Protocol Mechanics

### 5.1 What is an HTTP Request?
An **HTTP Request** is an ASCII text message sent by an HTTP client requesting an action or resource from a server.

**Example Request:**
```http
GET / HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0
Accept: text/html
Connection: close
```
- Line 1: Request Method (`GET`), Request URI Path (`/`), and Protocol Version (`HTTP/1.1`).
- Lines 2–5: Request Headers providing context to the server.

### 5.2 What is an HTTP Response?
An **HTTP Response** is an ASCII text message sent by the server containing a status line, metadata headers, and an optional message body.

**Example Response:**
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1842
Connection: close

<!DOCTYPE html>
<html>
...
</html>
```

### 5.3 Key Response Headers
- **`Content-Type`**: Tells the client how to interpret the payload bytes (e.g., `text/html` indicates an HTML document to be parsed and rendered).
- **`Content-Length`**: Specifies the exact size of the payload body in octets (bytes), allowing the client to know when the message body ends.
- **`Connection: close`**: Informs the client that the server will close the underlying TCP socket once the response body has been delivered.

### 5.4 HTTP Status Codes Used
- **`200 OK`**: The request succeeded, and the requested dashboard content is provided.
- **`400 Bad Request`**: The client sent malformed syntax that the parser cannot understand.
- **`404 Not Found`**: The requested path (e.g., `GET /random`) does not exist on the server.
- **`405 Method Not Allowed`**: The HTTP method (e.g., `POST`, `PUT`, `DELETE`) is recognized but not supported by this read-only dashboard.

---

## 6. Architectural Rationale & FAQ

### Why Localhost & 127.0.0.1?
Binding strictly to `127.0.0.1` ensures that the network socket is only accessible from programs running on the same physical host machine. It guarantees zero external exposure to local area networks (LAN) or public networks, eliminating remote attack vectors without requiring firewall rules.

### Why Isn't the Cloud Used?
Cloud architectures introduce unnecessary operational complexity, recurring subscription costs, vendor lock-in, reliance on continuous internet connectivity, and external latency. An agricultural monitoring system in a remote greenhouse or field cannot rely on guaranteed internet connectivity. A local-first architecture ensures 100% operational resilience.

### Why Isn't Wi-Fi Used?
Direct USB CDC-ACM serial connection delivers bus-powered hardware operation, deterministic transmission latency, zero radio-frequency interference, and requires no wireless credentials or complex network provisioning.

### Why Isn't MQTT Used?
MQTT requires a separate central message broker (such as Eclipse Mosquitto) running as a daemon service, alongside client publish-subscribe libraries. For a single-node monitoring system displaying sensor readings on a local dashboard, an embedded C++ HTTP server eliminates the overhead of managing a multi-tiered message bus.
