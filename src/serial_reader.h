#ifndef SERIAL_READER_H
#define SERIAL_READER_H

#include <string>

// Manages the Linux serial port connection to the Arduino.
// Opens /dev/ttyACM0 (or a specified device), configures 9600 8N1,
// and reads complete newline-terminated lines.
class SerialReader {
public:
    // Construct with device path (default: /dev/ttyACM0)
    explicit SerialReader(const std::string& devicePath = "/dev/ttyACM0");
    ~SerialReader();

    // Open and configure the serial port. Returns true on success.
    bool open();

    // Read one complete line (up to newline). Returns true on success.
    // Blocks until a full line is available or an error occurs.
    bool readLine(std::string& line);

    // Close the serial port.
    void close();

    // Check if the device file exists and is accessible.
    bool isDeviceAvailable() const;

    // Get the device path.
    const std::string& getDevicePath() const;

private:
    std::string devicePath_;
    int fd_;  // File descriptor
};

#endif // SERIAL_READER_H
