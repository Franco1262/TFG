#include "SD.h"
#include "driver/gpio.h"


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
    if (count == 0)
        return true;
    if (count > buffer.size())
        return false;

    char file_name[32];
    char date_buffer[11];
    const int file_name_length = snprintf(file_name, sizeof(file_name), "/%s.jsonl",
                                          Clock::getDate(date_buffer, sizeof(date_buffer)));
    if (file_name_length < 0 || static_cast<size_t>(file_name_length) >= sizeof(file_name))
        return false;

    File file = SD_MMC.open(file_name, FILE_APPEND);
    if (!file)
        return false;

    uint8_t written = 0;
    for (const auto& data : buffer)
    {
        if (written >= count)
            break;

        char json_buffer[256];
        const int json_length = snprintf(json_buffer, sizeof(json_buffer) - 1,
            "{\"ts\":%u,\"temp\":%.2f,\"hum\":%.2f,\"ec\":%u,\"ph\":%.1f,\"n\":%u,\"p\":%u,\"k\":%u,\"bat\":%u}",
            data.timestamp, data.temperature / 100.0, data.humidity / 100.0,
            data.conductivity, data.ph / 10.0, data.nitrogen,
            data.phosphorus, data.potassium, data.battery
        );
        if (json_length < 0 || static_cast<size_t>(json_length) >= sizeof(json_buffer) - 1)
            break;

        json_buffer[json_length] = '\n';
        const size_t line_length = static_cast<size_t>(json_length) + 1;
        if (file.write(reinterpret_cast<const uint8_t*>(json_buffer), line_length) != line_length)
            break;

        written++;
    }

    file.close();
    return written == count;
}


void SD::shutdown()
{
    SD_MMC.end();
    gpio_reset_pin((gpio_num_t)SDMMC_CLK);
    gpio_reset_pin((gpio_num_t)SDMMC_CMD);
    gpio_reset_pin((gpio_num_t)SDMMC_DATA);
    Power::disableSDCardPower();
}
