#include "Power.h"
#include "Clock.h"
#include "Sensor.h"
#include "Storage.h"
#include "utilities.h"

#define N_DATA_TO_SEND 10
RTC_DATA_ATTR static uint8_t writes_counter = 0;

void setup() 
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    Serial2.begin(4800, SERIAL_8N1, RS485_RXD, RS485_TXD);

    Power::Begin();
    bool sdOk = Storage::Begin();
    
    if(Power::isFirstBoot() || Clock::needsDailySync())
        Clock::syncWithGPS();
    
    Data data;
    data.timestamp = Clock::getTime();
    data.battery = Power::getBattPercent();
    
    if(Sensor::read(data))
    {
        if(sdOk)
        {
            //Storage::writeToDailyLog(data);
            //Storage::writeToPending(data);
            writes_counter++;

            if(writes_counter >= N_DATA_TO_SEND)
            {
                writes_counter = 0;
                //Lora::sendBatch();
                //Storage::clearPending();
            }
        }
        else
            ;
            //Lora::sendSingle(data);
    }
    
    Power::enterDeepSleep();
}

void loop()
{

}
