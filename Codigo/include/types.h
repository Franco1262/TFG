#pragma once
#include <Arduino.h>

struct __attribute__((packed)) Data {
    uint32_t timestamp;
    int16_t temperature;
    uint16_t humidity;
    uint16_t conductivity;
    uint16_t ph;
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
    uint8_t battery;
};

struct LoRaSessionState {
    bool is_joined;
    uint32_t netid;
    uint32_t devaddr;
    uint8_t nwkKey[16];
    uint8_t artKey[16];
    uint32_t seqnoUp;
    uint32_t seqnoDn;
};

struct PendingBatch {
    uint32_t first_sample;
    uint8_t count;
    uint8_t attempts;

    constexpr bool active() const { return count > 0; }
    constexpr bool exhausted() const { return attempts >= 2; }

    constexpr PendingBatch forCapacity(uint8_t capacity) const {
        return {first_sample, static_cast<uint8_t>(capacity > 0 && capacity < count ? capacity : count), attempts};
    }

    constexpr PendingBatch afterFailure() const {
        return {first_sample, count, static_cast<uint8_t>(attempts + 1)};
    }
};