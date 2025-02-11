# **Comunicación CAN BUS con Arduino y CAN BUS Shield V2.0**

## **Índice**
1. [Introducción](#introducción)
2. [Materiales necesarios](#materiales-necesarios)
3. [Instalación de la biblioteca](#instalación-de-la-biblioteca)
4. [Conexión del hardware](#conexión-del-hardware)
5. [Explicación detallada del código](#explicación-detallada-del-código)
   - [Emisor CAN](#emisor-can)
   - [Receptor CAN](#receptor-can)
6. [Opciones de configuración y funciones clave](#opciones-de-configuración-y-funciones-clave)
7. [Plantillas de código](#plantillas-de-código)
8. [Notas importantes](#notas-importantes)

---

## **1. Introducción**
Este proyecto permite la comunicación entre dispositivos a través del **bus CAN (Controller Area Network)** utilizando un **Arduino** con el **CAN BUS Shield V2.0 de Seeed Studio**.

Se implementan dos programas:
- **Emisor**: genera y envía valores de voltaje aleatorios.
- **Receptor**: recibe y muestra los valores en el monitor serie de Arduino.

Este documento explica **paso a paso** cómo instalar, configurar y entender el código.

---

## **2. Materiales necesarios**
- **1 Arduino UNO, Mega o similar**
- **1 CAN BUS Shield V2.0 de Seeed Studio**
- **Cables para conectar los módulos CAN entre sí**
- **Un segundo Arduino si se quiere probar comunicación emisor-receptor**
- **Resistencias de terminación de 120Ω (opcional, si la red CAN lo requiere)**

---

## **3. Instalación de la biblioteca**
El CAN BUS Shield usa la biblioteca **CAN_BUS_Shield**, que se puede instalar desde el **Gestor de Bibliotecas de Arduino**:

### **Pasos para instalar la librería:**
1. Abre **Arduino IDE**.
2. Ve a **Herramientas → Administrar Bibliotecas**.
3. Busca `"CAN_BUS_Shield"` en la barra de búsqueda.
4. Instala la versión más reciente.

También puedes descargarla desde [GitHub](https://github.com/Seeed-Studio/CAN_BUS_Shield) y colocarla en `Documents/Arduino/libraries/`.

---

## **4. Conexión del hardware**

### **Pines de conexión SPI**
El CAN BUS Shield usa el protocolo **SPI** para comunicarse con Arduino.

| **Placa**       | **Pin CS del Shield** |
|-----------------|----------------------|
| Seeed Studio   | **9** (por defecto)    |
| SparkFun CAN Shield | **10**             |

> 🔴 **IMPORTANTE**: En este proyecto, se usa el **pin 9** para el **CS** del módulo. Si usas el **CAN Shield de SparkFun**, debes cambiarlo a **10** en el código.

### **Conexión entre nodos CAN**
Para conectar dos Arduinos con CAN BUS Shield:
1. **Conecta CANH de un módulo con CANH del otro.**
2. **Conecta CANL de un módulo con CANL del otro.**
3. Si no usarás los módulos, deberás usar **2 conectores DB9 y soldarlos con cableado** con tal de comunicarlos de esa forma.

---

## **5. Explicación detallada del código**

### **A) Emisor CAN**
Este programa **genera un voltaje aleatorio (entre 10V y 100V) y lo envía cada segundo** por el bus CAN.

#### **Código del emisor**
```cpp
#include <SPI.h>
#include <mcp2515_can.h>  // Biblioteca para el MCP2515

#define CAN_CS_PIN 9  // Cambiar a 10 si usas SparkFun CAN Shield

mcp2515_can CAN(CAN_CS_PIN);  // Inicialización del CAN BUS

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Iniciando CAN BUS...");

    if (CAN.begin(CAN_500KBPS, MCP_16MHz) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);
    }
}

void loop() {
    int voltage = random(10, 101);
    byte data[2] = {(voltage >> 8) & 0xFF, voltage & 0xFF};

    if (CAN.sendMsgBuf(0x100, 0, 2, data) == CAN_OK) {
        Serial.print("Voltaje enviado: ");
        Serial.print(voltage);
        Serial.println(" V");
    } else {
        Serial.println("Error al enviar mensaje CAN");
    }

    delay(1000);
}
```

## B) **Receptor CAN**
Este programa **escucha los mensajes CAN y muestra el voltaje recibido**.

### **Código del receptor**
```cpp
#include <SPI.h>
#include <mcp2515_can.h>

#define CAN_CS_PIN 9  // Cambiar a 10 si usas SparkFun CAN Shield

mcp2515_can CAN(CAN_CS_PIN);

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
    if (CAN.checkReceive() == CAN_MSGAVAIL) {
        long unsigned int canId;
        byte len;
        byte data[8];

        CAN.readMsgBufID(&canId, &len, data);

        if (canId == 0x100 && len == 2) {
            int voltage = (data[0] << 8) | data[1];
            Serial.print("Voltaje recibido: ");
            Serial.print(voltage);
            Serial.println(" V");
        }
    }
}
```


