#include "driver_monitor.h"
#include <iostream>
#include <sys/stat.h>

DriverMonitor::DriverMonitor(const std::string& devicePath)
    : devicePath_(devicePath) {}

const std::string& DriverMonitor::getDevicePath() const {
    return devicePath_;
}

bool DriverMonitor::isDevicePresent() const {
    struct stat st;
    return (stat(devicePath_.c_str(), &st) == 0);
}

std::string DriverMonitor::getStatusString() const {
    std::string status;
    status += "Device:     " + devicePath_ + "\n";
    status += "Driver:     CDC-ACM / TTY\n";
    if (isDevicePresent()) {
        status += "Connection: Connected\n";
    } else {
        status += "Connection: Not Connected\n";
    }
    return status;
}

void DriverMonitor::printStatus() const {
    std::cout << "===== Driver Monitor =====" << std::endl;
    std::cout << getStatusString();
    std::cout << "==========================" << std::endl;
}
