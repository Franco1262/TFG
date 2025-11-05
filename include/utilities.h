#pragma once
#define LILYGO_ESP32S3_CAM_SIM7080G  // Has PSRAM

#if defined(LILYGO_ESP32S3_CAM_SIM7080G)

//Pins for handling communication with RS485 module
#define RS485_RXD 11
#define RS485_TXD 14
#define RS485_DIR1 13

//Pins for handling communications between LoRa Module and ESP32
#define LORA_SCK 9
#define LORA_MISO 3
#define LORA_MOSI 12
#define LORA_SS 10
#define LORA_DI0 35
#define LORA_RST 0

//Pins for handling SSDMMC communication
#define SDMMC_CMD 39
#define SDMMC_CLK 38
#define SDMMC_DATA 40

#ifdef I2C_SDA
#undef I2C_SDA
#endif

#ifdef I2C_SCL
#undef I2C_SCL
#endif

//Used to communicate with the PMU
#define I2C_SDA                     (15)
#define I2C_SCL                     (7)

#define MODEM_RX 4
#define MODEM_TX 5


#endif