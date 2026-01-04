#pragma once
#include <Arduino.h>

namespace Modem
{
    String obtenerCampoGNSS(String respuesta, int numeroCampo);
    String sendATandRead(const char* cmd, unsigned long timeoutMs = 2000);
}