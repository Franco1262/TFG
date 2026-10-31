#include "Clock.h"

#define SECONDS_DAY 86400
#define POLL_INTERVAL 10000
#define FIX_TIMEOUT (TIMEOUT_GPS_SECS * 1000UL)
RTC_DATA_ATTR static time_t last_sync_timestamp = 0;
RTC_DATA_ATTR static time_t last_sync_attempt_timestamp = 0;
RTC_DATA_ATTR static bool sync_attempted = false;


time_t Clock::getGpsTime()
{
    char buffer[256];
    char gps_time_response[30];
    char fix_status[2];
    time_t unix_time = 0;

    Power::enableModemPower();
    if (!Modem::turnOn() || !Modem::enableGps())
    {
        Modem::disableGps();
        Modem::turnOff();
        Power::disableModemPower();
        return unix_time;
    }

    unsigned long last_poll = millis() - POLL_INTERVAL;
    bool time_fixed = false;
    unsigned long t = millis();

    while(((millis() - t) < FIX_TIMEOUT) && !time_fixed)
    {
        if(millis() - last_poll >= POLL_INTERVAL)
        {
            last_poll = millis();
            memset(buffer, 0, sizeof(buffer));

            if(Modem::sendAtAndRead("AT+CGNSINF", buffer, sizeof(buffer), 2000))
            {
                DEBUG_PRINTLN(buffer);
                if (Modem::getGnssField(buffer, 1, fix_status, sizeof(fix_status)) != 1 ||
                    fix_status[0] != '1')
                    continue;

                int len = Modem::getGnssField(buffer, 2, gps_time_response, 30);
                if(len >= 14)
                {
                    unix_time = fromGpsToUnix(gps_time_response);
                    if(unix_time > 0)
                        time_fixed = true;
                }   
            }
        }
        yield();
    }
    
    Modem::disableGps();
    Modem::turnOff();
    Power::disableModemPower();

    return unix_time;
}


time_t Clock::fromGpsToUnix(const char* gps_string) 
{
    if (gps_string == NULL)
        return 0;

    const size_t length = strlen(gps_string);
    if (length != 14 && length != 18)
        return 0;

    for (size_t index = 0; index < length; index++)
    {
        if (index == 14)
        {
            if (gps_string[index] != '.')
                return 0;
        }
        else if (gps_string[index] < '0' || gps_string[index] > '9')
            return 0;
    }
    
    struct tm t = {0};
    int year, month, day, hour, min, sec;

    if (sscanf(gps_string, "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &min, &sec) != 6) 
        return 0;
    
    if (year < 2000 || year > 2099)
        return 0;
    if (month < 1 || month > 12)
        return 0;
    if (day < 1 || day > 31)
        return 0;
    if (hour > 23 || min > 59 || sec > 59)
        return 0;

    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    t.tm_isdst = 0;

    if (setenv("TZ", "UTC0", 1) != 0)
        return 0;
    tzset();

    time_t result = mktime(&t);
    if (result <= 0 || t.tm_year != year - 1900 || t.tm_mon != month - 1 ||
        t.tm_mday != day || t.tm_hour != hour || t.tm_min != min || t.tm_sec != sec)
        return 0;

    return result;
}


bool Clock::needsDailySync()
{
    time_t now = time(NULL);
    if (last_sync_timestamp != 0 && now >= last_sync_timestamp &&
        (now - last_sync_timestamp) < SECONDS_DAY)
        return false;

    if (!sync_attempted)
        return true;

    return now < last_sync_attempt_timestamp ||
           (now - last_sync_attempt_timestamp) >= DEEP_SLEEP_TIME_S;
}


bool Clock::syncWithGPS()
{
    time_t gps_time = getGpsTime();
    last_sync_attempt_timestamp = time(NULL);
    sync_attempted = true;

    if (gps_time > 0) 
    {
        struct timeval tv;
        tv.tv_sec = gps_time;
        tv.tv_usec = 0;
        if (settimeofday(&tv, NULL) != 0)
            return false;

        last_sync_timestamp = time(NULL);
        last_sync_attempt_timestamp = last_sync_timestamp;
        return true;
    }

    else
        return false;
}


time_t Clock::getTime()
{
    return time(NULL);
}


const char* Clock::getDate(char* buffer, size_t len)
{
    if (buffer == NULL || len < 11)
        return "1970-01-01";
    
    time_t now = getTime(); 
    struct tm timeinfo;
    if (gmtime_r(&now, &timeinfo) == NULL)
        return "1970-01-01";
    
    strftime(buffer, len, "%Y-%m-%d", &timeinfo);
    
    return buffer;
}