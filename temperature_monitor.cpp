#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

using namespace::std;

// TMP102 I2C temperature sensor
class TemperatureSensor {
private:
    int i2c_fd; // File descriptor for I2C device
    static constexpr int TMP102_ADDRESS = 0x48; // I2C address from i2cdetect

public:
    TemperatureSensor() {
        // Open I2C device
        i2c_fd = open("/dev/i2c-1", O_RDWR);
        if (i2c_fd < 0) {
            std::cerr << "Error: Could not open I2C device" << std::endl;
            exit(1);
        }
        // Set I2C slave address
        if (ioctl(i2c_fd, I2C_SLAVE, TMP102_ADDRESS) < 0) {
            std::cerr << "Error: Could not set I2C address" << std::endl;
            close(i2c_fd);
            exit(1);
        }
    }

    ~TemperatureSensor() {
        close(i2c_fd);
    }

    // Read temperature from TMP102
    float readTemperature() {
        // Read 2 bytes from temperature register (default 0x00)
        char buf[2];
        if (read(i2c_fd, buf, 2) != 2) {
            std::cerr << "Error: Could not read temperature" << std::endl;
            return 0.0f;
        }
        // Convert to temperature (12-bit resolution)
        int raw = ((buf[0] << 4) | (buf[1] >> 4));
        if (raw & 0x800) raw |= 0xF000; // Sign extend for negative temps
        return raw * 0.0625f; // 0.0625°C per LSB
    }
};

// Manages temperature data logging
class DataLogger {
private:
    std::ofstream logFile;
    std::vector<float> history;
    std::string logFileName;

public:
    DataLogger(const std::string& filename) : logFileName(filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error: Could not open log file " << filename << std::endl;
        }
    }

    ~DataLogger() {
        if (logFile.is_open()) logFile.close();
    }

    void logTemperature(float temp) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "," << temp << std::endl;
        history.push_back(temp);
    }

    void clearLog() {
        logFile.close();
        logFile.open(logFileName, std::ios::trunc);
        history.clear();
    }

    float getAverage() const {
        if (history.empty()) return 0.0f;
        float sum = 0.0f;
        for (float temp : history) sum += temp;
        return sum / history.size();
    }

    float getMin() const {
        if (history.empty()) return 0.0f;
        float min = history[0];
        for (float temp : history) if (temp < min) min = temp;
        return min;
    }

    float getMax() const {
        if (history.empty()) return 0.0f;
        float max = history[0];
        for (float temp : history) if (temp > max) max = temp;
        return max;
    }
};

// Manages terminal dashboard display
class Dashboard {
private:
    float highThreshold;
    float lowThreshold;

public:
    Dashboard(float high = 30.0f, float low = 10.0f) 
        : highThreshold(high), lowThreshold(low) {}

    void setHighThreshold(float high) { highThreshold = high; }
    void setLowThreshold(float low) { lowThreshold = low; }
    float getHighThreshold() const { return highThreshold; }
    float getLowThreshold() const { return lowThreshold; }

    void display(float temp, float avg, float min, float max) {
        std::system("clear"); // Clear terminal
        std::cout << "=== Temperature Monitoring System ===" << std::endl;
        std::cout << "Current Temperature: " << std::fixed << std::setprecision(2) << temp << " °C" << std::endl;
        std::cout << "Alert Status: ";
        if (temp > highThreshold) std::cout << "HIGH TEMPERATURE WARNING!" << std::endl;
        else if (temp < lowThreshold) std::cout << "LOW TEMPERATURE WARNING!" << std::endl;
        else std::cout << "Normal" << std::endl;
        std::cout << "High Threshold: " << highThreshold << " °C" << std::endl;
        std::cout << "Low Threshold: " << lowThreshold << " °C" << std::endl;
        std::cout << "Statistics:" << std::endl;
        std::cout << "  Average: " << avg << " °C" << std::endl;
        std::cout << "  Minimum: " << min << " °C" << std::endl;
        std::cout << "  Maximum: " << max << " °C" << std::endl;
        std::cout << "\nControls:" << std::endl;
        std::cout << "  h: Increase high threshold" << std::endl;
        std::cout << "  l: Increase low threshold" << std::endl;
        std::cout << "  H: Decrease high threshold" << std::endl;
        std::cout << "  L: Decrease low threshold" << std::endl;
        std::cout << "  r: Reset log" << std::endl;
        std::cout << "  q: Quit" << std::endl;
    }
};

// Handles non-blocking keyboard input
class InputHandler {
private:
    struct termios oldt, newt;
    int oldf;

public:
    InputHandler() {
        tcgetattr(STDIN_FILENO, &oldt); // Save old terminal settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // Set non-blocking
    }

    ~InputHandler() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore terminal settings
        fcntl(STDIN_FILENO, F_SETFL, oldf);
    }

    char getKey() {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) return ch;
        return '\0';
    }
};

// Main monitoring system
class TemperatureMonitor {
private:
    TemperatureSensor sensor;
    DataLogger logger;
    Dashboard dashboard;
    InputHandler input;
    bool running;

public:
    TemperatureMonitor() : logger("temperature_log.csv"), running(true) {}

    void run() {
        while (running) {
            // Read and log temperature
            float temp = sensor.readTemperature();
            logger.logTemperature(temp);

            // Update dashboard
            dashboard.display(temp, logger.getAverage(), logger.getMin(), logger.getMax());

            // Handle user input
            char key = input.getKey();
            switch (key) {
                case 'h': dashboard.setHighThreshold(dashboard.getHighThreshold() + 1.0f); break;
                case 'l': dashboard.setLowThreshold(dashboard.getLowThreshold() + 1.0f); break;
                case 'H': dashboard.setHighThreshold(dashboard.getHighThreshold() - 1.0f); break;
                case 'L': dashboard.setLowThreshold(dashboard.getLowThreshold() - 1.0f); break;
                case 'r': logger.clearLog(); break;
                case 'q': running = false; break;
            }

            // Control update rate
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};

int main() {
    TemperatureMonitor monitor;
    monitor.run();
    return 0;
}
