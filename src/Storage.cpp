#include "Storage.h"

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


void Storage::write(Data& data)
{

}