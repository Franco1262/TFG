#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

#define SENSOR_READ_TIMEOUT 2000
XPowersPMU   PMU;

File file;
HardwareSerial sim7080(1);
RTC_DATA_ATTR uint8_t writes_counter = 0;

//Struct to store data read from the sensor.
struct DataSensor
{
    float temperature;
    float humidity;
    int16_t conductivity;
    int16_t ph;
    int16_t nitrogen;
    int16_t phosphorus;
    int16_t potassium;
} dataReadSensor;

void printSDFile(const char* path) 
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

void setup() 
{
    Serial.begin(115200);
    delay(100);
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
    {
        while (1)
        {
            delay(5000);
            Serial.println("Failed to initialize power.....");
        }
    }
    else
        Serial.printf("getID:0x%x\n", PMU.getChipID());

    Serial2.begin(4800, SERIAL_8N1, RS485_RXD, RS485_TXD);
    PMU.setDC5Voltage(3300); // RTC main power channel 2700~ 3400V
    PMU.enableDC5();

    PMU.setALDO3Voltage(3300);   // SD Card VDD 3300
    PMU.enableALDO3();

    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);   // set sdcard pin use 1-bit mode
    if (!SD_MMC.begin("/sdcard", true)) 
    {
        Serial.println("Card Mount Failed");
        while (1) 
            delay(1000);
    }

    pinMode(RS485_DIR1, OUTPUT);
    delay(5000);
}

void decodeData(const uint8_t (&response)[19])
{
    dataReadSensor.temperature  = (((uint16_t)response[4] << 8) | (uint16_t)response[5]) / 10.0; //C
    dataReadSensor.humidity     = (((uint16_t)response[6] << 8) | (uint16_t)response[7]) / 10.0; //%
    dataReadSensor.conductivity = ((uint16_t)response[8]  << 8) | (uint16_t)response[9]; //1µS/cm
    dataReadSensor.ph           = (((uint16_t)response[10] << 8) | (uint16_t)response[11]) / 10; //
    dataReadSensor.nitrogen     = ((uint16_t)response[12] << 8) | (uint16_t)response[13]; //1mg/kg(mg/L)
    dataReadSensor.phosphorus   = ((uint16_t)response[14] << 8) | (uint16_t)response[15]; //1mg/kg(mg/L)
    dataReadSensor.potassium    = ((uint16_t)response[16] << 8) | (uint16_t)response[17]; //1mg/kg(mg/L)
}

void readSensor() 
{
    uint8_t request[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
    uint8_t response[19];
    int k = 0;

    digitalWrite(RS485_DIR1, HIGH); // Transmitir
    for(int i = 0; i < 8; i++) 
        Serial2.write(request[i]);
    Serial2.flush();
    digitalWrite(RS485_DIR1, LOW); // Recibir

    // Leer respuesta con timeout
    unsigned long start = millis();
    while((k < 19) && ((millis() - start) < SENSOR_READ_TIMEOUT)) 
    {
        if(Serial2.available())
            response[k++] = Serial2.read();      
    }

    decodeData(response);
}


void writeIntoSDCard()
{
    static uint8_t fileLinesCounter = 0;
    file = SD_MMC.open("/sensor_data.jsonl", FILE_APPEND);
    if (!file) 
    {
        Serial.println("Error abriendo archivo");
        return;
    }

    String jsonLine = "{";
    jsonLine += "\"temperature\":" + String(dataReadSensor.temperature) + ",";
    jsonLine += "\"humidity\":" + String(dataReadSensor.humidity) + ",";
    jsonLine += "\"conductivity\":" + String(dataReadSensor.conductivity) + ",";
    jsonLine += "\"ph\":" + String(dataReadSensor.ph) + ",";
    jsonLine += "\"nitrogen\":" + String(dataReadSensor.nitrogen) + ",";
    jsonLine += "\"phosphorus\":" + String(dataReadSensor.phosphorus) + ",";
    jsonLine += "\"potassium\":" + String(dataReadSensor.potassium);
    jsonLine += "}\n";

    file.print(jsonLine);
    file.close();

    writes_counter++;
}

void sendData()
{
    
}

void loop() 
{
    readSensor();
    writeIntoSDCard();
    if(writes_counter == 10)
    {
        //Logic to send lora packets
        sendData();
        writes_counter == 0;
    }
    delay(2000);
}
