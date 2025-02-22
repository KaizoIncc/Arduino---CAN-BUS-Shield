#include <SPI.h>
#include <mcp2515_can.h>  // Biblioteca para el MCP2515

#define CAN_CS_PIN 9   // Cambiar a 10 si usas SparkFun CAN Shield
#define CAN_INT_PIN 2  // Pin de interrupción del MCP2515 (debe ser 2 o 3 en Arduino UNO)

mcp2515_can CAN(CAN_CS_PIN);  // Inicialización del CAN BUS
volatile bool mensajeDisponible = false;  // Flag de interrupción

// Función de interrupción: se ejecuta cuando llega un mensaje CAN
void mensajeCAN_ISR() {
    mensajeDisponible = true;  // Activa el flag cuando hay un mensaje disponible
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Esperando mensajes CAN...");

    // Configurar el CAN a la velocidad y frecuencia del cristal deseadas
    if (CAN.begin(CAN_500KBPS, MCP_16MHz) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);
    }

    // Configurar el pin de interrupción
    pinMode(CAN_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), mensajeCAN_ISR, FALLING);
}

void loop() {
    // Si se activó la interrupción (mensaje CAN recibido)
    if (mensajeDisponible) {
        mensajeDisponible = false;  // Reiniciar el flag

        long unsigned int canId;
        byte len;
        byte data[8];  // Buffer para los datos

        // Leer el mensaje recibido
        CAN.readMsgBufID(&canId, &len, data);

        // Filtrar por ID específico (modificar según necesidad)
        if (canId == 0x100 && len == 2) {  // Suponiendo que el emisor usa ID 0x100
            // Reconstruir el dato recibido (modifica si el formato cambia)
            int datoRecibido = (data[0] << 8) | data[1];

            Serial.print("Voltaje recibido: ");
            Serial.print(datoRecibido);
            Serial.println(" V");
        }
    }
}