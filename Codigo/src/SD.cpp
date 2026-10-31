#include "SD.h"


void SD::powerOn()
{
    Power::enableSDCardPower();
}


bool SD::mount()
{
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);   // set sdcard pin use 1-bit mode
    return SD_MMC.begin("/sdcard", true);
}


bool SD::logBatch(const CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& buffer, uint8_t count)
{
    char file_name[32];
    char date_buffer[11];
    snprintf(file_name, sizeof(file_name), "/%s.jsonl", Clock::getDate(date_buffer, sizeof(date_buffer)));

    File file = SD_MMC.open(file_name, FILE_APPEND);
    if (!file)
        return false;

    uint8_t written = 0;
    for (const auto& data : buffer)
    {
        if (written >= count)
            break;

        char json_buffer[256];
        snprintf(json_buffer, sizeof(json_buffer),
            "{\"ts\":%u,\"temp\":%.2f,\"hum\":%.2f,\"ec\":%u,\"ph\":%.1f,\"n\":%u,\"p\":%u,\"k\":%u,\"bat\":%u}",
            data.timestamp, data.temperature / 100.0, data.humidity / 100.0,
            data.conductivity, data.ph / 10.0, data.nitrogen,
            data.phosphorus, data.potassium, data.battery
        );
        if (file.println(json_buffer) == 0)
            break;
        written++;
    }

    file.close();
    return written == count;
}


void SD::shutdown()
{
    SD_MMC.end();
    Power::disableSDCardPower();
}
