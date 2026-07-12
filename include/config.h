#pragma once
#include <cstddef>

static constexpr int DEEP_SLEEP_TIME_S = 300;
static constexpr int CRITICAL_LEVEL = 10;
static constexpr int TIMEOUT_GPS_SECS = 60;
static constexpr int TIMEOUT_SENSOR_SECS = 5;
static constexpr int TIMEOUT_LORA_SECS = 15;
static constexpr int SENSOR_READ_TIMEOUT = 2000;

static constexpr int MAX_PAYLOAD_SF7_SF8 = 222;
static constexpr int MAX_PAYLOAD_SF9 = 115;
static constexpr int MAX_PAYLOAD_SF10_SF12 = 51;

static constexpr uint8_t MIN_ITEMS_TO_ATTEMPT_SEND = 4;
static constexpr int MAX_ABSOLUTE_BATCH_SIZE = 256;


//Pins for handling communication with RS485 module
//TODO: find new pins for these
static constexpr int RS485_RXD = 1;
static constexpr int RS485_TXD = 2;
static constexpr int RS485_DIR1 = 3;

//Pins for handling communications between LoRa Module and ESP32
static constexpr int LORA_SCK = 11;
static constexpr int LORA_MISO = 10;
static constexpr int LORA_MOSI = 9;
static constexpr int LORA_SS = 12;
static constexpr int LORA_DIO0 = 17;
static constexpr int LORA_DIO1 = 16;
static constexpr int LORA_RST = 14;

//Pins for handling SSDMMC communication
static constexpr int SDMMC_CMD = 39;
static constexpr int SDMMC_CLK = 38;
static constexpr int SDMMC_DATA = 40;

//Used to communicate with the PMU
static constexpr int I2C_SDA_PMU = 15;
static constexpr int I2C_SCL_PMU = 7;

static constexpr int MODEM_RX = 4;
static constexpr int MODEM_TX = 5;
static constexpr int MODEM_PWR = 41;