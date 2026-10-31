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
RTC_DATA_ATTR PendingBatch pending_batch;
RTC_DATA_ATTR uint32_t rtc_buffer_front_id = 0;

static constexpr uint32_t RTC_BUFFER_MAGIC = 0x52544247u;

static void initRtcBuffer()
{
    if (rtc_buffer_magic != RTC_BUFFER_MAGIC)
    {
        rtc_buffer.clear();
        pending_batch = {0, 0, 0};
        rtc_buffer_front_id = 0;
        rtc_buffer_magic = RTC_BUFFER_MAGIC;
    }
}

static void finalizeBatch()
{
    SD::powerOn();
    if (SD::mount())
    {
        if (!SD::logBatch(rtc_buffer, pending_batch.count))
            DEBUG_PRINTLN("Could not write daily log");
    }
    else
    {
        DEBUG_PRINTLN("Could not mount SD for daily log");
    }
    SD::shutdown();

    rtc_buffer.discard(pending_batch.count);
    rtc_buffer_front_id += pending_batch.count;
    pending_batch = {0, 0, 0};
}

static void processBatch()
{
    if (!pending_batch.active())
    {
        if (rtc_buffer.size() < MIN_ITEMS_TO_ATTEMPT_SEND)
            return;
        pending_batch = {rtc_buffer_front_id, MIN_ITEMS_TO_ATTEMPT_SEND, 0};
    }

    if (pending_batch.first_sample != rtc_buffer_front_id ||
        pending_batch.count > rtc_buffer.size())
    {
        DEBUG_PRINTLN("Pending batch does not match the RTC buffer");
        pending_batch = {0, 0, 0};
        return;
    }

    bool sent = false;
    if (Lora::turnOn())
    {
        if (Lora::ensureSessionReady())
        {
            const uint8_t capacity = Lora::getMaxItemsPerPacket();
            if (pending_batch.attempts == 0 && capacity > 0)
            {
                if (rtc_buffer.size() < capacity)
                {
                    pending_batch = {0, 0, 0};
                    Lora::turnOff();
                    return;
                }
                pending_batch.count = capacity;
            }

            if (capacity > 0)
            {
                const PendingBatch attempted_batch = pending_batch.forCapacity(capacity);
                int16_t result = RADIOLIB_ERR_NONE;
                const uint8_t n_items_sent = Lora::sendBatch(rtc_buffer, attempted_batch.count, &result);

                if (result == RADIOLIB_ERR_UPLINK_UNAVAILABLE)
                {
                    DEBUG_PRINTLN("Batch deferred by duty-cycle limits");
                    Lora::turnOff();
                    return;
                }

                pending_batch = attempted_batch;
                if (n_items_sent > 0)
                {
                    pending_batch.count = n_items_sent;
                    sent = true;
                    Lora::saveLoRaSessionToRTC();
                }
            }
            else
            {
                DEBUG_PRINTLN("No sample fits in the current LoRaWAN payload");
            }
        }
        else if (Lora::getSessionStatus() == RADIOLIB_ERR_UPLINK_UNAVAILABLE)
        {
            DEBUG_PRINTLN("Session activation deferred by duty-cycle limits");
            Lora::turnOff();
            return;
        }
        Lora::turnOff();
    }

    if (sent)
    {
        finalizeBatch();
    }
    else
    {
        pending_batch = pending_batch.afterFailure();
        if (pending_batch.exhausted())
            finalizeBatch();
        else
            DEBUG_PRINTF("Batch %u: first attempt failed, retaining %u samples\n",
                         pending_batch.first_sample, pending_batch.count);
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

    if(Power::isColdBoot() || Clock::needsDailySync())
    {
        DEBUG_PRINTF("Trying to fix GPS, timeout: %d s\n", TIMEOUT_GPS_SECS);
        if (!Clock::syncWithGPS())
            DEBUG_PRINTLN("GNSS sync failed; using the ESP32 clock");
    }

    if (battery < 0 || battery > 100)
        DEBUG_PRINTLN("Battery percentage unavailable");

    data.battery = (battery >= 0 && battery <= 100) ? static_cast<uint8_t>(battery) : BATTERY_UNKNOWN;
    data.timestamp = Clock::getTime();

    Sensor::turnOn();
    bool sensor_ok = fakeData(data);//Sensor::read(data);
    Sensor::turnOff();

    if (sensor_ok && !rtc_buffer.push(data))
        DEBUG_PRINTLN("RTC buffer full; the new sample was not queued");

    processBatch();
    
    Power::enterDeepSleep();
}

void loop()
{

}