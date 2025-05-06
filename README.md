CS 2300 Temperature Monitor Project


Jeb Alford

Summary

This program monitors temperature using a TMP102 I2C temperature sensor connected to a Raspberry Pi. It collects temperature data, logs it to a CSV file, and displays real-time readings and statistics in a terminal-based dashboard. The program supports interactive threshold adjustments and log resetting, running via SSH on a Raspberry Pi.



C++ Programming:




Object-Oriented Programming: Classes (TemperatureSensor, DataLogger, Dashboard, InputHandler, TemperatureMonitor) for modularity.



File I/O: Logging temperature data to a CSV file (std::ofstream).



Time handling: Timestamping data (std::chrono, std::put_time).



Exception handling: Managing I2C errors (std::cerr, exit).



Multithreading: Non-blocking input and periodic updates (std::thread, std::this_thread::sleep_for).



Data structures: Storing temperature history (std::vector).



Raspberry Pi I2C:





Interfacing with the TMP102 sensor using Linux I2C device interface (/dev/i2c-1, linux/i2c-dev.h).



I2C communication: Reading temperature data (ioctl, read).



Terminal Control:





Non-blocking keyboard input (termios.h, fcntl.h).



Terminal clearing and formatting (std::system("clear"), std::fixed, std::setprecision).



Standard C++ Libraries:





iostream: Console output.



fstream: File operations.



vector: Temperature history storage.



chrono, iomanip: Time and formatting.



string: String manipulation.



Linux System Libraries:





termios.h, unistd.h, fcntl.h: Terminal input control.



sys/ioctl.h, linux/i2c-dev.h: I2C communication.

Operating Systems





Tested on Raspberry Pi OS (Debian-based, latest version as of May 2025).



Run via SSH from a terminal on macOS, Windows (WSL), or Linux.

Installation Requirements

To run this program, you need:





Raspberry Pi with a TMP102 I2C temperature sensor connected to I2C pins (SDA, SCL, 3.3V, GND).



Raspberry Pi OS installed and SSH enabled.



I2C Enabled:

sudo raspi-config





Navigate to "Interface Options" -> "I2C" -> Enable.



Reboot if prompted.



Install I2C Tools:

sudo apt update
sudo apt install i2c-tools



Verify TMP102 Sensor:

i2cdetect -y 1

Confirm the sensor appears at address 0x48.



C++ Compiler: Ensure g++ is installed (pre-installed on Raspberry Pi OS):

sudo apt install g++



Required Libraries:





Standard C++ libraries (included with g++).



Linux I2C development headers (install if missing):

sudo apt install libi2c-dev

Setup and Running Instructions





Clone the Repository:

git clone <repository-url>
cd <repository-directory>



Connect to Raspberry Pi via SSH:

ssh pi@<raspberry-pi-ip-address>

Default password is usually raspberry (change it for security).



Compile the Program:

g++ -o temp_monitor temp_monitor.cpp -std=c++17



Run the Program:

./temp_monitor





The dashboard displays current temperature, alert status, thresholds, and statistics.



Data is logged to temperature_log.csv in the same directory.



Use the following controls:





h: Increase high threshold by 1°C.



l: Increase low threshold by 1°C.



H: Decrease high threshold by 1°C.



L: Decrease low threshold by 1°C.



r: Reset the log file.



q: Quit the program.



Stop the Program: Press q to exit gracefully.

Citations





I2C communication code adapted from Linux I2C documentation and TMP102 datasheet examples:





https://www.kernel.org/doc/html/latest/i2c/dev-interface.html



https://www.ti.com/lit/ds/symlink/tmp102.pdf





No third-party code was used beyond standard libraries, Linux headers, and instructor-provided concepts.

Grading Rubric and Points

Based on the CS 2300 grading rubric, I believe this project deserves 95/100 points:





Functionality (40/40): Fully functional temperature monitoring with real-time dashboard, logging, and interactive controls.



Code Quality (20/20): Well-structured, modular C++ code with clear comments and error handling.



Documentation (20/20): Comprehensive README with detailed setup, usage, and citations.



Creativity/Complexity (15/20): Advanced features like interactive thresholds and non-blocking input, but lacks visualization or network capabilities.



Total: 95/100
