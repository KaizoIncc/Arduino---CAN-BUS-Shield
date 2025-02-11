#include <SPI.h>
#include <mcp2515_can.h>  // Biblioteca para el MCP2515

#define CAN_CS_PIN 9  // Pin CS del MCP2515

mcp2515_can CAN(CAN_CS_PIN);  // Objeto CAN

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Esperando mensajes CAN...");

    if (CAN.begin(CAN_500KBPS, MCP_16MHz) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);
    }
}

void loop() {
    if (CAN.checkReceive() == CAN_MSGAVAIL) {  // Comprueba si hay un mensaje disponible
        long unsigned int canId;
        byte len;
        byte data[8];

        CAN.readMsgBufID(&canId, &len, data);  // Lee el mensaje recibido

        if (canId == 0x100 && len == 2) {  // Comprobar si el mensaje es del emisor
            int voltage = (data[0] << 8) | data[1];  // Reconstruir el voltaje
            Serial.print("Voltaje recibido: ");
            Serial.print(voltage);
            Serial.println(" V");
        }
    }
}
