#include "Storage.h"

static const char* PENDING_FILE = "/pending.bin";

bool Storage::Begin()
{
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);   // set sdcard pin use 1-bit mode

    return SD_MMC.begin("/sdcard", true);
}

void Storage::printSDFile(const char* path) 
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


void Storage::writeToDailyLog(Data& data)
{
    char file_name[32];
    snprintf(file_name, sizeof(file_name), "/%s.jsonl", Clock::getDate());

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


void Storage::writeToPending(Data& data)
{
    File file = SD_MMC.open(PENDING_FILE, FILE_APPEND);
    
    if (!file) 
        return;
    
    size_t written = file.write((const uint8_t*)&data, sizeof(Data));

    file.close();
}

void Storage::clearPending()
{
    if (SD_MMC.exists(PENDING_FILE)) 
        SD_MMC.remove(PENDING_FILE);
}