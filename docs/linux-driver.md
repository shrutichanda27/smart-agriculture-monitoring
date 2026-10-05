# Linux Device Driver & CDC-ACM Architecture

## 1. Overview

In modern Linux operating systems, USB devices that implement serial communication protocols are integrated through existing kernel subsystems rather than requiring bespoke kernel modules.

This project utilizes the standard **Linux USB Communication Device Class Abstract Control Model (CDC-ACM)** driver. No custom kernel module is written or compiled; instead, we leverage the kernel's proven character device abstraction.

---

## 2. Kernel-to-User Space Architectural Flow

```
+------------------------------------------------------------------------+
|                              USER SPACE                                |
|                                                                        |
|   +----------------------------------------------------------------+   |
|   |         Smart Agriculture Application (C++ Executable)          |   |
|   |             open("/dev/ttyACM0", O_RDONLY | O_NOCTTY)           |   |
|   |             tcgetattr() / tcsetattr()                          |   |
|   |             read() / close()                                   |   |
|   +----------------------------------------------------------------+   |
|                                   ^                                    |
|                                   | System Calls (VFS Interface)       |
|                                   v                                    |
|   +----------------------------------------------------------------+   |
|   |             Device Node: /dev/ttyACM0 (Character Device)       |   |
|   |             Major: 166, Minor: 0                               |   |
|   +----------------------------------------------------------------+   |
+-----------------------------------|------------------------------------+
                                    |
+-----------------------------------|------------------------------------+
|                              KERNEL SPACE                              |
|                                   v                                    |
|   +----------------------------------------------------------------+   |
|   |                         TTY Subsystem                          |   |
|   |              Line Discipline (termios: raw, 8N1)                |   |
|   +----------------------------------------------------------------+   |
|                                   ^                                    |
|                                   v                                    |
|   +----------------------------------------------------------------+   |
|   |                       CDC-ACM Driver                           |   |
|   |                     (kernel module: cdc_acm)                   |   |
|   +----------------------------------------------------------------+   |
|                                   ^                                    |
|                                   v                                    |
|   +----------------------------------------------------------------+   |
|   |                    Linux USB Host Controller                   |   |
|   |                      (xHCI / ehci-pci / usbcore)               |   |
|   +----------------------------------------------------------------+   |
+-----------------------------------|------------------------------------+
                                    | Physical USB Bus (D+ / D- / VBUS / GND)
+-----------------------------------|------------------------------------+
|                            HARDWARE LAYER                              |
|                                   v                                    |
|   +----------------------------------------------------------------+   |
|   |             Arduino UNO ATmega16U2 (USB-to-Serial Bridge)      |   |
|   |                               ^                                |   |
|   |                               v UART                           |   |
|   |                 ATmega328P Primary Microcontroller             |   |
|   +----------------------------------------------------------------+   |
+------------------------------------------------------------------------+
```

---

## 3. Core Operating System Concepts

### 3.1 Kernel Space vs. User Space
- **Kernel Space**: The privileged execution ring (Ring 0 on x86-64 / EL1 on ARM) where the Linux kernel, device drivers, memory managers, and scheduler execute. Has unrestricted access to physical hardware registers and memory.
- **User Space**: The unprivileged execution ring (Ring 3 on x86-64 / EL0 on ARM) where user applications (including our C++ binary) execute. Cannot directly manipulate physical hardware registers; must make controlled **system calls** through the kernel.

### 3.2 USB CDC-ACM Class Specification
- The USB Implementers Forum defines standard device classes. The Communication Device Class (CDC) Abstract Control Model (ACM) standardizes serial communication over USB.
- The Arduino UNO's secondary ATmega16U2 microcontroller presents standard USB CDC-ACM descriptors to the host.
- When connected, the Linux `usbcore` subsystem detects these descriptors and automatically binds the kernel driver module `cdc_acm.ko`.

### 3.3 TTY Subsystem & Character Devices
- Linux treats peripherals through the Unix philosophy: *"Everything is a file"*.
- Devices are represented as filesystem entries in `/dev`.
- A **Character Device** transfers data as an unbuffered, sequential stream of characters (bytes), unlike block devices (e.g., hard drives) which transfer fixed-size blocks.
- The **TTY (TeleTYpewriter) Subsystem** sits between the low-level serial driver and user space, providing line buffering, control character parsing, and echo services (which our C++ code disables to achieve raw byte transfers).

### 3.4 `/dev/ttyACM0` Device Node
- `/dev/ttyACM0` is the default character device node created by `udev` when the first CDC-ACM USB device is plugged in.
- Typical properties:
  ```bash
  crw-rw---- 1 root dialout 166, 0 /dev/ttyACM0
  ```
  where `c` designates a character device, `166` is the major number (identifying driver `cdc_acm`), and `0` is the minor number (identifying the specific device instance).

---

## 4. POSIX System Calls & Termios API

The C++ module `SerialReader` interacts with the device node via POSIX system calls:

1. **`open()`**:
   ```cpp
   int fd = open("/dev/ttyACM0", O_RDONLY | O_NOCTTY);
   ```
   - `O_RDONLY`: Opens the device node for reading only.
   - `O_NOCTTY`: Informs Linux that this terminal port should not become the controlling terminal of the calling process.

2. **`tcgetattr()` & `tcsetattr()`**:
   - `tcgetattr(fd, &tty)` retrieves the current terminal parameters into a `struct termios`.
   - `tcsetattr(fd, TCSANOW, &tty)` applies modified configuration attributes immediately.

3. **`cfsetispeed()` & `cfsetospeed()`**:
   - Explicitly assigns input and output baud rates to `B9600` (9600 bits per second).

4. **Raw Configuration Flags**:
   - `c_cflag &= ~PARENB`: Disables parity generation and detection.
   - `c_cflag &= ~CSTOPB`: Sets 1 stop bit (clearing the 2 stop bits flag).
   - `c_cflag |= CS8`: Configures 8 data bits per character.
   - `c_cflag |= CREAD | CLOCAL`: Enables the serial receiver and disables modem status line checks.
   - `c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG)`: Disables canonical mode (line buffering), local echo, and signal generation.

5. **`read()`**:
   - Synchronously consumes assembled bytes from the kernel character device buffer.

6. **`close()`**:
   - Flushes descriptors and releases kernel file table resources.
