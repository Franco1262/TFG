#include "Clock.h"

#define SECONDS_DAY 86400

//TODO: Fix while timeout
String Clock::getGPSTime()
{
    String rawGPSData;

    Power::enableModemAndGPS();

    unsigned long lastPoll = 0;
    const unsigned long POLL_INTERVAL = 30000;

    bool timeFixed = false;
    while(!timeFixed)
    {
        if (millis() - lastPoll >= POLL_INTERVAL) 
        {
            lastPoll = millis();
            String fullRawGPSData = Modem::sendATandRead("AT+CGNSINF", 3000);
            rawGPSData = Modem::obtenerCampoGNSS(fullRawGPSData, 2);
            Serial.println(fullRawGPSData);

            if(rawGPSData.length() == 0)
                Serial.println("Time still not fixed");
                 
            else
                timeFixed = true;
        }
        delay(100);
    }

    Power::disableModemAndGPS();

    return rawGPSData;
}


time_t Clock::fromGPSToUnix(String fechaGPS) 
{
    struct tm t;

    t.tm_year = fechaGPS.substring(0, 4).toInt() - 1900;
    t.tm_mon  = fechaGPS.substring(4, 6).toInt() - 1;
    t.tm_mday = fechaGPS.substring(6, 8).toInt();
    t.tm_hour = fechaGPS.substring(8, 10).toInt();
    t.tm_min  = fechaGPS.substring(10, 12).toInt();
    t.tm_sec  = fechaGPS.substring(12, 14).toInt();
    t.tm_isdst = -1;

    return mktime(&t);
}

bool Clock::needsDailySync()
{
    bool needsSync = false;
    time_t ahora;
    time(&ahora);

    if ((ahora - lastSyncTimestamp) > SECONDS_DAY)
        needsSync = true;

    return needsSync;
}

void Clock::syncWithGPS()
{
    String gpsString = getGPSTime();
    if (gpsString != "") 
    {
        time_t timeFromGPS = fromGPSToUnix(gpsString);

        struct timeval tv;
        tv.tv_sec = timeFromGPS;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);

        lastSyncTimestamp = timeFromGPS;
    }
}

time_t Clock::getTime()
{
    return time(NULL);
}