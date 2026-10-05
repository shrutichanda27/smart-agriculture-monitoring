#!/usr/bin/env bash
# ==============================================================================
# Smart Agriculture Monitoring System - Deployment & Architecture Validation
# ==============================================================================
# This script performs static review, file integrity, prohibited tech checks,
# build configuration check, and test verification.
# Returns non-zero on any failure.
# ==============================================================================

set -o pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

TOTAL_CHECKS=0
PASSED_CHECKS=0
FAILED_CHECKS=0

check_start() {
    ((TOTAL_CHECKS++))
    echo -ne "  [CHECK $TOTAL_CHECKS] $1 ... "
}

check_pass() {
    ((PASSED_CHECKS++))
    echo -e "${GREEN}PASSED${NC}"
}

check_fail() {
    ((FAILED_CHECKS++))
    echo -e "${RED}FAILED${NC} - $1"
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

echo -e "${BLUE}==============================================================================${NC}"
echo -e "${BLUE}  Smart Agriculture Monitoring System: Comprehensive Validation Pipeline    ${NC}"
echo -e "${BLUE}==============================================================================${NC}"

# ------------------------------------------------------------------------------
# 1. Project Structure & Required Files Check
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 1. Project Structure & Required Files ===${NC}"

REQUIRED_FILES=(
    "CMakeLists.txt"
    "README.md"
    "LICENSE"
    "tinkercad/arduino_code.ino"
    "tinkercad/circuit.md"
    "tinkercad/pin_configuration.md"
    "src/main.cpp"
    "src/serial_reader.cpp"
    "src/serial_reader.h"
    "src/sensor_processor.cpp"
    "src/sensor_processor.h"
    "src/alert_engine.cpp"
    "src/alert_engine.h"
    "src/database.cpp"
    "src/database.h"
    "src/http_server.cpp"
    "src/http_server.h"
    "src/dashboard_generator.cpp"
    "src/dashboard_generator.h"
    "src/driver_monitor.cpp"
    "src/driver_monitor.h"
    "database/schema.sql"
    "dashboard/index.html"
    "dashboard/style.css"
    "tests/parser_test.cpp"
    "tests/alert_test.cpp"
    "tests/database_test.cpp"
    "tests/http_server_test.cpp"
    "docs/architecture.md"
    "docs/hardware-design.md"
    "docs/software-design.md"
    "docs/linux-driver.md"
    "docs/serial-communication.md"
    "docs/networking.md"
    "docs/database-design.md"
    "docs/testing.md"
    "docs/evaluation-guide.md"
    "scripts/validate.sh"
)

check_start "All required project files exist"
MISSING_FILES=0
for f in "${REQUIRED_FILES[@]}"; do
    if [[ ! -f "$f" ]]; then
        echo -e "\n    Missing: $f"
        MISSING_FILES=$((MISSING_FILES + 1))
    fi
done

if [[ $MISSING_FILES -eq 0 ]]; then
    check_pass
else
    check_fail "$MISSING_FILES required file(s) missing"
fi

# ------------------------------------------------------------------------------
# 2. Strict Technology Restrictions Check (No Python, JS, React, Cloud, ML, etc.)
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 2. Prohibited Technologies Audit ===${NC}"

check_start "Zero Python files (.py, .pyc, .ipynb)"
PYTHON_COUNT=$(find . -not -path '*/.*' -type f \( -name "*.py" -o -name "*.pyc" -o -name "*.ipynb" \) | wc -l)
if [[ $PYTHON_COUNT -eq 0 ]]; then
    check_pass
else
    check_fail "Found $PYTHON_COUNT Python files"
fi

check_start "Zero JavaScript/TypeScript files (.js, .ts, .jsx, .tsx, .mjs)"
JS_COUNT=$(find . -not -path '*/.*' -type f \( -name "*.js" -o -name "*.ts" -o -name "*.jsx" -o -name "*.tsx" -o -name "*.mjs" \) | wc -l)
if [[ $JS_COUNT -eq 0 ]]; then
    check_pass
else
    check_fail "Found $JS_COUNT JavaScript/TypeScript files"
fi

check_start "Zero Node.js / NPM configuration files (package.json, node_modules)"
NODE_COUNT=$(find . -not -path '*/.*' \( -name "package.json" -o -name "package-lock.json" -o -name "node_modules" \) | wc -l)
if [[ $NODE_COUNT -eq 0 ]]; then
    check_pass
else
    check_fail "Found $NODE_COUNT Node.js / npm artifacts"
fi

check_start "Zero Java files (.java, .class, .jar)"
JAVA_COUNT=$(find . -not -path '*/.*' -type f \( -name "*.java" -o -name "*.class" -o -name "*.jar" \) | wc -l)
if [[ $JAVA_COUNT -eq 0 ]]; then
    check_pass
else
    check_fail "Found $JAVA_COUNT Java files"
fi

check_start "Source scan for forbidden libraries (MQTT, Firebase, Cloud SDKs, TensorFlow, PyTorch)"
FORBIDDEN_KEYWORDS=(
    "mqtt"
    "firebase"
    "tensorflow"
    "pytorch"
    "scikit-learn"
    "aws-sdk"
    "azure-sdk"
    "google-cloud"
    "django"
    "flask"
    "express"
    "react"
)
FORBIDDEN_FOUND=0
for kw in "${FORBIDDEN_KEYWORDS[@]}"; do
    MATCHES=$(grep -rnI --exclude="validate.sh" --exclude="*.md" -i "$kw" src/ 2>/dev/null | wc -l)
    if [[ $MATCHES -gt 0 ]]; then
        echo -e "\n    Forbidden keyword '$kw' found in C++ sources ($MATCHES matches)"
        FORBIDDEN_FOUND=$((FORBIDDEN_FOUND + 1))
    fi
done

if [[ $FORBIDDEN_FOUND -eq 0 ]]; then
    check_pass
else
    check_fail "Found $FORBIDDEN_FOUND forbidden library references in source code"
fi

# ------------------------------------------------------------------------------
# 3. Linux Networking & Socket Audit
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 3. Linux Socket & Networking Verification ===${NC}"

check_start "Socket API usage in src/http_server.cpp (socket, setsockopt, bind, listen, accept, recv, send, close)"
SOCKET_APIS=("socket" "setsockopt" "bind" "listen" "accept" "recv" "send" "close")
MISSING_SOCKET_APIS=0
for api in "${SOCKET_APIS[@]}"; do
    if ! grep -q "$api(" src/http_server.cpp; then
        echo -e "\n    Missing socket API: $api()"
        MISSING_SOCKET_APIS=$((MISSING_SOCKET_APIS + 1))
    fi
done

if [[ $MISSING_SOCKET_APIS -eq 0 ]]; then
    check_pass
else
    check_fail "Missing $MISSING_SOCKET_APIS socket API calls in http_server.cpp"
fi

check_start "Strict local binding to 127.0.0.1 (localhost isolation)"
if grep -q "127.0.0.1" src/http_server.cpp && ! grep -q "INADDR_ANY" src/http_server.cpp; then
    check_pass
else
    check_fail "Server must bind strictly to 127.0.0.1 and not INADDR_ANY"
fi

check_start "Default port configured to 8080"
if grep -q "8080" src/http_server.h && grep -q "8080" src/main.cpp; then
    check_pass
else
    check_fail "Port 8080 must be configured as default in http_server.h and main.cpp"
fi

check_start "HTTP response codes present (200, 400, 404, 405)"
HTTP_CODES=("200" "400" "404" "405")
MISSING_CODES=0
for code in "${HTTP_CODES[@]}"; do
    if ! grep -q "$code" src/http_server.cpp; then
        MISSING_CODES=$((MISSING_CODES + 1))
    fi
done

if [[ $MISSING_CODES -eq 0 ]]; then
    check_pass
else
    check_fail "Missing HTTP response code handling in http_server.cpp"
fi

# ------------------------------------------------------------------------------
# 4. Linux Driver & Serial Communication Audit
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 4. Linux Driver & Serial Interface Verification ===${NC}"

check_start "Serial device reference to /dev/ttyACM0"
if grep -q "/dev/ttyACM0" src/serial_reader.h && grep -q "/dev/ttyACM0" src/driver_monitor.h; then
    check_pass
else
    check_fail "/dev/ttyACM0 not properly referenced as default device"
fi

check_start "POSIX termios API usage in src/serial_reader.cpp (tcgetattr, tcsetattr, cfsetispeed, cfsetospeed)"
TERMIOS_CALLS=("tcgetattr" "tcsetattr" "cfsetispeed" "cfsetospeed" "B9600")
MISSING_TERMIOS=0
for call in "${TERMIOS_CALLS[@]}"; do
    if ! grep -q "$call" src/serial_reader.cpp; then
        MISSING_TERMIOS=$((MISSING_TERMIOS + 1))
    fi
done

if [[ $MISSING_TERMIOS -eq 0 ]]; then
    check_pass
else
    check_fail "Missing termios configuration calls in serial_reader.cpp"
fi

# ------------------------------------------------------------------------------
# 5. Database & Schema Audit
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 5. SQLite Database & Schema Verification ===${NC}"

check_start "SQLite table schema in database/schema.sql"
if grep -q "CREATE TABLE IF NOT EXISTS sensor_data" database/schema.sql && \
   grep -q "temperature REAL" database/schema.sql && \
   grep -q "humidity REAL" database/schema.sql && \
   grep -q "soil_moisture REAL" database/schema.sql && \
   (grep -q "water_level REAL" database/schema.sql || grep -q "ph REAL" database/schema.sql) && \
   grep -q "light REAL" database/schema.sql; then
    check_pass
else
    check_fail "schema.sql missing expected table definition or sensor columns"
fi

check_start "Database prepared statement usage in src/database.cpp"
if grep -q "sqlite3_prepare_v2" src/database.cpp && \
   grep -q "sqlite3_bind_" src/database.cpp && \
   grep -q "sqlite3_step" src/database.cpp && \
   grep -q "sqlite3_finalize" src/database.cpp; then
    check_pass
else
    check_fail "src/database.cpp does not properly use SQLite prepared statements"
fi

# ------------------------------------------------------------------------------
# 6. Arduino Firmware & Tinkercad Pin Map Alignment
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 6. Arduino Firmware & Circuit Pin Mapping ===${NC}"

check_start "Arduino pin mappings (A0-A4 analog sensors, alert LEDs configured)"
if grep -q "A0" tinkercad/arduino_code.ino && \
   grep -q "A1" tinkercad/arduino_code.ino && \
   grep -q "A2" tinkercad/arduino_code.ino && \
   grep -q "A3" tinkercad/arduino_code.ino && \
   grep -q "A4" tinkercad/arduino_code.ino && \
   (grep -q "LED" tinkercad/arduino_code.ino || grep -q "13" tinkercad/arduino_code.ino || grep -q "12" tinkercad/arduino_code.ino); then
    check_pass
else
    check_fail "Arduino code pin definitions do not match specification"
fi

check_start "Arduino serial baud rate 9600"
if grep -q "Serial.begin(9600)" tinkercad/arduino_code.ino; then
    check_pass
else
    check_fail "Arduino firmware must initialize Serial at 9600 baud"
fi

# ------------------------------------------------------------------------------
# 7. Dashboard Files & HTML/CSS Integrity
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 7. Dashboard HTML & CSS Integrity ===${NC}"

check_start "Static dashboard HTML structure & required sensor cards"
if grep -q "card-temperature" dashboard/index.html && \
   grep -q "card-humidity" dashboard/index.html && \
   grep -q "card-soil" dashboard/index.html && \
   (grep -q "card-water" dashboard/index.html || grep -q "card-ph" dashboard/index.html) && \
   grep -q "card-light" dashboard/index.html; then
    check_pass
else
    check_fail "dashboard/index.html is missing required sensor card elements"
fi

check_start "Zero JavaScript in dashboard HTML"
if grep -i -q "<script" dashboard/index.html; then
    check_fail "Found <script> tag in dashboard/index.html. JavaScript is strictly prohibited."
else
    check_pass
fi

# ------------------------------------------------------------------------------
# 8. CMake Build & Test Execution
# ------------------------------------------------------------------------------
echo -e "\n${YELLOW}=== 8. Build System & Compilation ===${NC}"

check_start "CMake configuration and compilation check"
if command -v cmake &> /dev/null && command -v g++ &> /dev/null; then
    mkdir -p build_validation
    cd build_validation || exit 1
    if cmake .. > cmake_log.txt 2>&1 && make -j$(nproc 2>/dev/null || echo 2) > make_log.txt 2>&1; then
        cd "$PROJECT_ROOT" || exit 1
        check_pass

        echo -e "\n${YELLOW}=== 9. Running Test Suite ===${NC}"
        TEST_BINARIES=("parser_test" "alert_test" "database_test" "http_server_test")
        ALL_TESTS_OK=true
        for tb in "${TEST_BINARIES[@]}"; do
            check_start "Run $tb"
            if "./build_validation/$tb" > "build_validation/${tb}.log" 2>&1; then
                check_pass
            else
                check_fail "Execution of $tb failed. See build_validation/${tb}.log"
                ALL_TESTS_OK=false
            fi
        done
        rm -rf build_validation
    else
        cd "$PROJECT_ROOT" || exit 1
        rm -rf build_validation
        check_fail "Compilation failed. Check CMake and g++ installation and logs."
    fi
else
    echo -e "${YELLOW}SKIPPED (CMake or g++ not found in current environment path)${NC}"
    echo "    Static analysis and structural verification succeeded."
fi

# ------------------------------------------------------------------------------
# Summary
# ------------------------------------------------------------------------------
echo -e "\n${BLUE}==============================================================================${NC}"
echo -e "Validation Summary: ${GREEN}$PASSED_CHECKS passed${NC}, ${RED}$FAILED_CHECKS failed${NC} out of $TOTAL_CHECKS checks."
echo -e "${BLUE}==============================================================================${NC}"

if [[ $FAILED_CHECKS -gt 0 ]]; then
    exit 1
fi
exit 0
