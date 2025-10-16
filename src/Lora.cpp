#include "Lora.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"

uint64_t devEUI  = 0x1122334455667788;
uint64_t joinEUI = 0x70B3D57ED0012345;
uint8_t  appKey[]  = { 0xB2, 0x45, 0xE8, 0x9F, 0xF8, 0x7E, 0xE8, 0x8D, 0xFB, 0xA6, 0x52, 0x8E, 0xD1, 0x52, 0xD9, 0x11 };

RTC_DATA_ATTR uint8_t lorawanSession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
RTC_DATA_ATTR uint8_t lorawanNonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
RTC_DATA_ATTR bool lorawanSessionValid = false;

static SX1276 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);
static LoRaWANNode node(&radio, &EU868);

static void printHexBuffer(const char* label, const uint8_t* buf, size_t len)
{
    DEBUG_PRINTF("%s", label);
    for (size_t i = 0; i < len; i++) {
        if (i > 0) {
            DEBUG_PRINT(" ");
        }
        DEBUG_PRINTF("%02X", buf[i]);
    }
    DEBUG_PRINTLN("");
}


bool Lora::turnOn()
{ 
    gpio_reset_pin((gpio_num_t)LORA_SCK);
    gpio_reset_pin((gpio_num_t)LORA_MISO);
    gpio_reset_pin((gpio_num_t)LORA_MOSI);
    gpio_reset_pin((gpio_num_t)LORA_SS);
    gpio_reset_pin((gpio_num_t)LORA_DIO0);
    gpio_reset_pin((gpio_num_t)LORA_DIO1);

    pinMode(LORA_RST, OUTPUT);
    digitalWrite(LORA_RST, LOW);

    Power::enableDC5();
    delay(10);

    pinMode(LORA_SS, OUTPUT);
    digitalWrite(LORA_SS, HIGH);
    digitalWrite(LORA_RST, HIGH);
    delay(5);

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);
    int state = radio.begin(868.1);
    
    if(state < RADIOLIB_ERR_NONE) 
    {
        DEBUG_PRINTLN("Radio init failed: ");
        DEBUG_PRINTLN(state);    
        return false; 
    }
    
    return true;
}


void Lora::turnOff()
{
    radio.sleep();
    delay(1);

    SPI.end();

    pinMode(LORA_RST, OUTPUT);
    digitalWrite(LORA_RST, LOW);

    gpio_reset_pin((gpio_num_t)LORA_SCK);
    gpio_reset_pin((gpio_num_t)LORA_MISO);
    gpio_reset_pin((gpio_num_t)LORA_MOSI);
    gpio_reset_pin((gpio_num_t)LORA_SS);
    gpio_reset_pin((gpio_num_t)LORA_DIO0);
    gpio_reset_pin((gpio_num_t)LORA_DIO1);

    Power::disableDC5();
}


bool Lora::joinOTAA()
{
    int state;
    
    state = node.beginOTAA(joinEUI, devEUI, appKey, appKey);
    if (state < RADIOLIB_ERR_NONE)
        return false;
    
    state = node.activateOTAA();

    if (state != RADIOLIB_LORAWAN_NEW_SESSION)
    {
        DEBUG_PRINTLN("There was an error when activating LoraWAN session, error code: ");
        DEBUG_PRINTLN(state);
        return false;
    }
        

    node.setADR(true);

    return true;
}


void Lora::saveLoRaSessionToRTC()
{
    memcpy(lorawanSession,
           node.getBufferSession(),
           RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

    memcpy(lorawanNonces,
           node.getBufferNonces(),
           RADIOLIB_LORAWAN_NONCES_BUF_SIZE);

    DEBUG_PRINTLN("Saved LoRaWAN session to RTC:");
    printHexBuffer("  Nonces:  ", lorawanNonces, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
    printHexBuffer("  Session: ", lorawanSession, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

    lorawanSessionValid = true;
}


bool Lora::restoreLoRaSessionFromRTC()
{
    if (!lorawanSessionValid)
        return false;

    if (node.beginOTAA(joinEUI, devEUI, appKey, appKey) < RADIOLIB_ERR_NONE)
        return false;

    DEBUG_PRINTLN("Restoring LoRaWAN session from RTC:");
    printHexBuffer("  Nonces:  ", lorawanNonces, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
    printHexBuffer("  Session: ", lorawanSession, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

    int16_t s1 = node.setBufferNonces(lorawanNonces);
    int16_t s2 = node.setBufferSession(lorawanSession);

    DEBUG_PRINTF("setBufferNonces = %d\n", s1);
    DEBUG_PRINTF("setBufferSession = %d\n", s2);

    int state = node.activateOTAA();
    if (state != RADIOLIB_LORAWAN_SESSION_RESTORED)
    {
        DEBUG_PRINTLN("There was an error when re-activating LoraWAN session");
        DEBUG_PRINTLN(state);
        lorawanSessionValid = false;
        return false;
    }
        

    return true;
}


uint8_t Lora::sendBatch(CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& buffer)
{
    if(buffer.empty())
        return 0;

    constexpr uint8_t LORAWAN_MAX_PAYLOAD = 242;
    alignas(Data) uint8_t payload[LORAWAN_MAX_PAYLOAD];
    
    uint8_t maxLen = node.getMaxPayloadLen();
    uint8_t max_items = maxLen / sizeof(Data);
    uint8_t len = 0;
    uint8_t items_to_send = 0;

    for (const auto& data : buffer)
    {
        if (items_to_send >= max_items) 
            break;
        
        *reinterpret_cast<Data*>(payload + len) = data;
        len += sizeof(Data);
        items_to_send++;
    }

    int state = node.sendReceive(payload, len);

    if (state < RADIOLIB_ERR_NONE) 
    {
        DEBUG_PRINTLN("Send failed:");
        DEBUG_PRINTLN(state);
        return 0;
    }

    if(items_to_send == 0)
        return 0;

    Data dummy;
    for (uint8_t i = 0; i < items_to_send; i++) 
        buffer.pop(dummy); 

    return items_to_send;
}


uint8_t Lora::getMaxItemsPerPacket()
{
    return node.getMaxPayloadLen() / sizeof(Data);
}