#include "SD.h"

static const char* PENDING_FILE = "/pending.bin";


void SD::powerOn()
{
    Power::enableSDCardPower();
}


bool SD::mount()
{
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);   // set sdcard pin use 1-bit mode
    return SD_MMC.begin("/sdcard", true);
}


void SD::printSDFile(const char* path) 
{
    if (!SD_MMC.begin()) {
        Serial.println("Error inicializando SD");
        return;
    }

    File file = SD_MMC.open(path, FILE_READ);
    if (!file) {
        Serial.println("No se pudo abrir el archivo");
        return;
    }

    Serial.printf("Contenido de %s:\n", path);

    // Leer línea por línea
    while (file.available()) {
        String line = file.readStringUntil('\n');
        Serial.println(line);
    }

    file.close();
    Serial.println("--- Fin del archivo ---");
}


void SD::writeToDailyLog(Data& data)
{
    char file_name[32];
    char date_buffer[11];
    snprintf(file_name, sizeof(file_name), "/%s.jsonl", Clock::getDate(date_buffer, sizeof(date_buffer)));

    File file = SD_MMC.open(file_name, FILE_APPEND);

    if (!file) 
        return;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "{\"ts\":%u,\"temp\":%.2f,\"hum\":%.2f,\"ec\":%d,\"ph\":%d,\"n\":%d,\"p\":%d,\"k\":%d,\"bat\":%u}",
        data.timestamp, data.temperature, data.humidity, data.conductivity,
        data.ph, data.nitrogen, data.phosphorus, data.potassium, data.battery
    );

    file.println(buffer);
    file.close();
}


void SD::logBatch(const CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& buffer)
{
    char file_name[32];
    char date_buffer[11];
    snprintf(file_name, sizeof(file_name), "/%s.jsonl", Clock::getDate(date_buffer, sizeof(date_buffer)));

    File file = SD_MMC.open(file_name, FILE_APPEND);
    if (!file)
        return;

    for (const auto& data : buffer)
    {
        char json_buffer[256];
        snprintf(json_buffer, sizeof(json_buffer),
            "{\"ts\":%u,\"temp\":%.2f,\"hum\":%.2f,\"ec\":%d,\"ph\":%d,\"n\":%d,\"p\":%d,\"k\":%d,\"bat\":%u}",
            data.timestamp, data.temperature, data.humidity,
            data.conductivity, data.ph, data.nitrogen,
            data.phosphorus, data.potassium, data.battery
        );
        file.println(json_buffer);
    }

    file.close();
}


void SD::writeToPending(Data& data)
{
    File file = SD_MMC.open(PENDING_FILE, FILE_APPEND);
    
    if (!file) 
        return;
    
    size_t written = file.write((const uint8_t*)&data, sizeof(Data));

    file.close();
}


void SD::shutdown()
{
    SD_MMC.end();
    Power::disableSDCardPower();
}


void SD::clearPending()
{
    if (SD_MMC.exists(PENDING_FILE)) 
        SD_MMC.remove(PENDING_FILE);
}