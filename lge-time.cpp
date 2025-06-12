/*
 * lge-time.cpp
 *
 * Main daemon loop that:
 * 1. Continuously reads datetime from RTC device
 * 2. Validates the datetime format
 * 3. Updates system time only when it changes, setting timezone to UTC
 * 4. Uses different sleep intervals based on whether time changed
 * 5. Logs operations to kernel message buffer
 *
 * Key components:
 * - RTC_DEVICE: Path to RTC hardware device file
 * - sleep_interval: Dynamic sleep duration (shorter when time changes)
 * - old_time: Tracks previous time to detect changes
 * - DEFAULT_DATETIME: Fallback time when RTC read fails
 *
 * Timezone handling:
 * - All times are set to UTC timezone (GMT+0)
 * - No timezone conversion is performed
 * - System time is maintained in UTC
 *
 * Error handling:
 * - Checks if RTC file can be opened
 * - Validates datetime format
 * - Logs all operations and errors
 *
 * Performance considerations:
 * - Uses longer sleep interval when time is stable
 * - Only updates system time when necessary
 */

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <sys/time.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <regex>

class LgeTimeDaemon {
private:
    static const std::string RTC_DEVICE;
    static const std::string DEFAULT_DATETIME;
    static const int NORMAL_SLEEP_INTERVAL = 60;  // seconds
    static const int FAST_SLEEP_INTERVAL = 5;     // seconds
    
    std::string old_time;
    int sleep_interval;
    bool running;
    
public:
    LgeTimeDaemon() : sleep_interval(NORMAL_SLEEP_INTERVAL), running(true) {
        openlog("lge-time", LOG_PID | LOG_CONS, LOG_DAEMON);
        syslog(LOG_INFO, "LGE Time Daemon started");
    }
    
    ~LgeTimeDaemon() {
        syslog(LOG_INFO, "LGE Time Daemon stopped");
        closelog();
    }
    
    void stop() {
        running = false;
    }
    
    std::string readRtcTime() {
        std::ifstream rtc_file(RTC_DEVICE);
        if (!rtc_file.is_open()) {
            syslog(LOG_ERR, "Failed to open RTC device: %s", RTC_DEVICE.c_str());
            return DEFAULT_DATETIME;
        }
        
        std::string datetime;
        if (std::getline(rtc_file, datetime)) {
            rtc_file.close();
            return datetime;
        } else {
            syslog(LOG_ERR, "Failed to read from RTC device");
            rtc_file.close();
            return DEFAULT_DATETIME;
        }
    }
    
    bool validateDatetime(const std::string& datetime) {
        std::regex datetime_pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
        return std::regex_match(datetime, datetime_pattern);
    }
    
    bool setSystemTime(const std::string& datetime) {
        if (!validateDatetime(datetime)) {
            syslog(LOG_ERR, "Invalid datetime format: %s", datetime.c_str());
            return false;
        }
        
        struct tm tm_time = {};
        if (strptime(datetime.c_str(), "%Y-%m-%d %H:%M:%S", &tm_time) == nullptr) {
            syslog(LOG_ERR, "Failed to parse datetime: %s", datetime.c_str());
            return false;
        }
        
        tm_time.tm_isdst = 0;
        time_t new_time = timegm(&tm_time);
        
        struct timeval tv;
        tv.tv_sec = new_time;
        tv.tv_usec = 0;
        
        if (settimeofday(&tv, nullptr) != 0) {
            syslog(LOG_ERR, "Failed to set system time: %s", datetime.c_str());
            return false;
        }
        
        syslog(LOG_INFO, "System time updated to: %s UTC", datetime.c_str());
        return true;
    }
    
    void run() {
        syslog(LOG_INFO, "Starting main daemon loop");
        
        while (running) {
            std::string current_time = readRtcTime();
            
            if (current_time != old_time) {
                if (setSystemTime(current_time)) {
                    old_time = current_time;
                    sleep_interval = FAST_SLEEP_INTERVAL;
                    syslog(LOG_INFO, "Time changed, using fast sleep interval");
                } else {
                    syslog(LOG_ERR, "Failed to update system time");
                }
            } else {
                sleep_interval = NORMAL_SLEEP_INTERVAL;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(sleep_interval));
        }
    }
};

const std::string LgeTimeDaemon::RTC_DEVICE = "/dev/rtc0";
const std::string LgeTimeDaemon::DEFAULT_DATETIME = "2024-01-01 00:00:00";

LgeTimeDaemon* g_daemon = nullptr;

void signalHandler(int signal) {
    if (g_daemon) {
        syslog(LOG_INFO, "Received signal %d, shutting down", signal);
        g_daemon->stop();
    }
}

int main() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    
    LgeTimeDaemon daemon;
    g_daemon = &daemon;
    
    try {
        daemon.run();
    } catch (const std::exception& e) {
        syslog(LOG_ERR, "Exception in main loop: %s", e.what());
        return 1;
    }
    
    return 0;
}
