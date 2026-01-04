#include "Modem.h"

String Modem::obtenerCampoGNSS(String respuesta, int numeroCampo) 
{
    int primerSalto = respuesta.indexOf('\n');
    int inicioSegundaLinea = primerSalto + 1;
    int segundoSalto = respuesta.indexOf('\n', inicioSegundaLinea);
    String segundaLinea = respuesta.substring(inicioSegundaLinea, segundoSalto);
    
    int coma = 0;
    int anterior = 0;
    
    for (int i = 0; i < numeroCampo && anterior < segundaLinea.length(); i++) {
        anterior = segundaLinea.indexOf(',', anterior) + 1;
    }
    
    int siguiente = segundaLinea.indexOf(',', anterior);
    if (siguiente == -1) siguiente = segundaLinea.length();
    
    return segundaLinea.substring(anterior, siguiente);
}


String Modem::sendATandRead(const char* cmd, unsigned long timeoutMs) 
{
    while (Serial1.available()) Serial1.read();
    Serial1.print(cmd);
    Serial1.print("\r\n");
    String s;
    unsigned long t = millis();
    while (millis() - t < timeoutMs) 
    {
        while (Serial1.available()) s += (char)Serial1.read();
        if (s.indexOf("OK") >= 0 || s.indexOf("ERROR") >= 0) break;
        delay(5);
    }
    return s;
}