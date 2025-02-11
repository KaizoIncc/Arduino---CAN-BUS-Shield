#include <SPI.h>
#include <mcp2515_can.h>  // Biblioteca correcta

#define CAN_CS_PIN 9

mcp2515_can CAN(CAN_CS_PIN);  // Usa la subclase correcta

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Iniciando CAN BUS...");

    if (CAN.begin(CAN_500KBPS, MCP_16MHz) == CAN_OK) {  // Configuración sin MCP_ANY
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);
    }
}

void loop() {
    int voltage = random(10, 101);  // Genera un voltaje aleatorio entre 10V y 100V
    byte data[2];
    data[0] = (voltage >> 8) & 0xFF; // Byte alto
    data[1] = voltage & 0xFF;        // Byte bajo

    if (CAN.sendMsgBuf(0x100, 0, 2, data) == CAN_OK) {
        Serial.print("Voltaje enviado: ");
        Serial.print(voltage);
        Serial.println(" V");
    } else {
        Serial.println("Error al enviar mensaje CAN");
    }

    delay(1000);
}
