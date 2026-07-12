#include "Power.h"
#include "Lora.h"
#include "Clock.h"
#include "Sensor.h"
#include "SD.h"
#include "config.h"
#include "types.h"
#include "CircularBuffer.h"
#include "utilities.h"
#include "debug.h"

RTC_DATA_ATTR uint32_t rtc_buffer_magic = 0;
RTC_DATA_ATTR CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE> rtc_buffer;

static constexpr uint32_t RTC_BUFFER_MAGIC = 0x52544246u;

static void initRtcBuffer()
{
    if (rtc_buffer_magic != RTC_BUFFER_MAGIC)
    {
        rtc_buffer.clear();
        rtc_buffer_magic = RTC_BUFFER_MAGIC;
    }
}

void setup() 
{
    int battery = 0;
    Data data;

    //TODO: Remove for production
    Serial.begin(115200);
    delay(2000);   

    DEBUG_PRINTLN("Turning ON minimal HW");
    Power::turnOnMinimalHW();

    battery = Power::getBattPercent();
    if((battery < CRITICAL_LEVEL) && (battery >= 0)) 
        Power::enterDeepSleep();


    initRtcBuffer();

    // if(Power::isColdBoot() || Clock::needsDailySync())
    // {
    //     DEBUG_PRINTLN("Trying to fix GPS, this will take 1-5mins");
    //     Clock::syncWithGPS();
    // }

    data.battery = battery;
    data.timestamp = Clock::getTime();

    Sensor::turnOn();
    bool sensor_ok = fakeData(data);//Sensor::read(data);
    Sensor::turnOff();

    if (sensor_ok) 
        rtc_buffer.push(data);

    if (rtc_buffer.size() >= MIN_ITEMS_TO_ATTEMPT_SEND)
    {
        if (Lora::turnOn())
        {
            bool session_ok = Lora::ensureSessionReady();

            if (session_ok)
            {
                const uint8_t max_items_per_packet = Lora::getMaxItemsPerPacket();

                if (max_items_per_packet > 0 && rtc_buffer.size() >= max_items_per_packet)
                {
                    // SD::powerOn();
                    // if (SD::mount())
                    // {
                    //     SD::logBatch(rtc_buffer);
                    // }
                    // SD::shutdown();

                    uint8_t n_items_sent = Lora::sendBatch(rtc_buffer);

                    if(n_items_sent > 0)
                    {
                        rtc_buffer.discard(n_items_sent);
                        Lora::saveLoRaSessionToRTC();
                    }
                }
            }
            Lora::turnOff();
        }
    }
    
    Power::enterDeepSleep();
}

void loop()
{

}