#include "serial_reader.h"

#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <errno.h>

SerialReader::SerialReader(const std::string& devicePath)
    : devicePath_(devicePath), fd_(-1) {}

SerialReader::~SerialReader() {
    close();
}

bool SerialReader::isDeviceAvailable() const {
    struct stat st;
    return (stat(devicePath_.c_str(), &st) == 0);
}

const std::string& SerialReader::getDevicePath() const {
    return devicePath_;
}

bool SerialReader::open() {
    if (!isDeviceAvailable()) {
        std::cerr << "[SerialReader] Device not found: " << devicePath_ << std::endl;
        return false;
    }

    // Open device in read-only, non-controlling terminal mode
    fd_ = ::open(devicePath_.c_str(), O_RDONLY | O_NOCTTY);
    if (fd_ < 0) {
        std::cerr << "[SerialReader] Failed to open " << devicePath_
                  << ": " << strerror(errno) << std::endl;
        return false;
    }

    // Configure serial port using termios
    struct termios tty;
    std::memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd_, &tty) != 0) {
        std::cerr << "[SerialReader] tcgetattr failed: " << strerror(errno) << std::endl;
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    // Set baud rate: 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // 8 data bits, no parity, 1 stop bit (8N1)
    tty.c_cflag &= ~PARENB;   // No parity
    tty.c_cflag &= ~CSTOPB;   // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;        // 8 data bits
    tty.c_cflag |= CREAD;      // Enable receiver
    tty.c_cflag |= CLOCAL;     // Ignore modem control lines

    // Raw input mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // No software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // Raw output
    tty.c_oflag &= ~OPOST;

    // Read returns after 1 byte, with 10-second timeout (in tenths of seconds)
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 100;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        std::cerr << "[SerialReader] tcsetattr failed: " << strerror(errno) << std::endl;
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    // Flush any stale data
    tcflush(fd_, TCIFLUSH);

    std::cout << "[SerialReader] Opened " << devicePath_ << " at 9600 baud" << std::endl;
    return true;
}

bool SerialReader::readLine(std::string& line) {
    if (fd_ < 0) {
        return false;
    }

    line.clear();
    char ch;

    while (true) {
        ssize_t n = ::read(fd_, &ch, 1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;  // Interrupted, retry
            }
            std::cerr << "[SerialReader] Read error: " << strerror(errno) << std::endl;
            return false;
        }
        if (n == 0) {
            // Timeout or EOF
            return false;
        }

        if (ch == '\n') {
            return !line.empty();
        }

        if (ch != '\r') {
            line += ch;
        }

        // Guard against excessively long lines
        if (line.size() > 256) {
            std::cerr << "[SerialReader] Line too long, discarding" << std::endl;
            line.clear();
        }
    }
}

void SerialReader::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
        std::cout << "[SerialReader] Closed " << devicePath_ << std::endl;
    }
}
