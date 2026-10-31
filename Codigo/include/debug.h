#define DEBUG_MODE 1 

#if DEBUG_MODE
  #define DEBUG_PRINT(x)       Serial.print(x)
  #define DEBUG_PRINTLN(x)     Serial.println(x)
  #define DEBUG_PRINTF(...)    Serial.printf(__VA_ARGS__) // For ESP32/modern boards
#else
  #define DEBUG_PRINT(x)       // Compiles to nothing
  #define DEBUG_PRINTLN(x)     // Compiles to nothing
  #define DEBUG_PRINTF(...)    // Compiles to nothing
#endif