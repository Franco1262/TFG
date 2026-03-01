#include "Clock.h"

#define SECONDS_DAY 86400
#define POLL_INTERVAL 10000
#define FIX_TIMEOUT 600000
RTC_DATA_ATTR static time_t last_sync_timestamp = 0;


time_t Clock::getGpsTime()
{
    char buffer[256];
    char gps_time_response[30];
    time_t unix_time = 0;

    Power::enableModemAndGps();

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
                Serial.println(buffer);
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
    
    Power::disableModemAndGps();

    return unix_time;
}


time_t Clock::fromGpsToUnix(const char* gps_string) 
{
    struct tm t = {0};
    int year, month, day, hour, min, sec;

    if (sscanf(gps_string, "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &min, &sec) == 6) 
    {
        t.tm_year = year - 1900;
        t.tm_mon  = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min  = min;
        t.tm_sec  = sec;
        t.tm_isdst = -1;

        return mktime(&t);
    }

    return 0;
}


bool Clock::needsDailySync()
{
    if (last_sync_timestamp == 0) return true;

    time_t now = time(NULL);
    return (now - last_sync_timestamp) > SECONDS_DAY;
}


void Clock::syncWithGPS()
{
    time_t gps_time = getGpsTime();

    if (gps_time > 0) 
    {
        struct timeval tv;
        tv.tv_sec = gps_time;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        last_sync_timestamp = time(NULL);
    }
}


time_t Clock::getTime()
{
    return time(NULL);
}

const char* Clock::getDate()
{
    static char buffer[11]; 
    
    time_t now = getTime(); 
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo); 
    
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    
    return buffer;
}