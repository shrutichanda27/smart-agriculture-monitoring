#ifndef DRIVER_MONITOR_H
#define DRIVER_MONITOR_H

#include <string>

// Monitors the Linux CDC-ACM / TTY device for Arduino connectivity.
// Reports the status of /dev/ttyACM0 without writing a custom kernel driver.
class DriverMonitor {
public:
    explicit DriverMonitor(const std::string& devicePath = "/dev/ttyACM0");

    // Check if the device file exists.
    bool isDevicePresent() const;

    // Print a status report to stdout.
    void printStatus() const;

    // Get a status summary string.
    std::string getStatusString() const;

    const std::string& getDevicePath() const;

private:
    std::string devicePath_;
};

#endif // DRIVER_MONITOR_H
