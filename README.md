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
#include <SPI.h>             // Biblioteca para la comunicación SPI
#include <mcp2515_can.h>     // Biblioteca para controlar el MCP2515

#define CAN_CS_PIN 9  // Pin Chip Select (CS) para el MCP2515. Si usas SparkFun CAN Shield, cambia a 10.

mcp2515_can CAN(CAN_CS_PIN);  // Inicialización del objeto CAN con el pin CS configurado

void setup() {
    Serial.begin(115200);  // Iniciar comunicación serie a 115200 baudios
    while (!Serial);       // Esperar a que el monitor serie esté listo
    Serial.println("Iniciando CAN BUS...");

    // Inicializa el bus CAN a 500 Kbps con un cristal de 16 MHz
    if (CAN.begin(CAN_500KBPS, MCP_16MHz) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);  // Si falla la inicialización, detener el programa
    }
}

void loop() {
    // Genera un voltaje aleatorio entre 10 y 100
    int voltage = random(10, 101);

    // Separa el número en dos bytes para enviarlo
    byte data[2] = {(voltage >> 8) & 0xFF, voltage & 0xFF};

    // Enviar mensaje CAN con ID 0x100, no extendido (0), 2 bytes de datos
    if (CAN.sendMsgBuf(0x100, 0, 2, data) == CAN_OK) {
        Serial.print("Voltaje enviado: ");
        Serial.print(voltage);
        Serial.println(" V");
    } else {
        Serial.println("Error al enviar mensaje CAN");
    }

    delay(1000);  // Espera 1 segundo antes de enviar el siguiente mensaje
}
```

### B) **Receptor CAN**
Este programa **escucha los mensajes CAN y muestra el voltaje recibido**.

#### **Código del receptor por CAN.checkReceive()**
```cpp
#include <SPI.h>              // Biblioteca para comunicación SPI
#include <mcp2515_can.h>      // Biblioteca para el controlador MCP2515

#define CAN_CS_PIN 9  // Cambiar a 10 si usas SparkFun CAN Shield

// Se crea un objeto para manejar el bus CAN
mcp2515_can CAN(CAN_CS_PIN);

void setup() {
    Serial.begin(115200); // Inicializa la comunicación serie con una velocidad de 115200 baudios
    while (!Serial);      // Espera hasta que el puerto serie esté listo
    Serial.println("Esperando mensajes CAN...");

    // Inicializa el bus CAN a 500 kbps con un cristal de 16 MHz
    if (CAN.begin(CAN_500KBPS, MCP_16MHz) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1); // Si hay error, detiene el programa
    }
}

void loop() {
    // Verifica si hay un mensaje CAN disponible en el buffer de recepción
    if (CAN.checkReceive() == CAN_MSGAVAIL) {
        long unsigned int canId;  // Variable para almacenar el ID del mensaje recibido
        byte len;                 // Variable para almacenar la longitud del mensaje recibido
        byte data[8];             // Array para almacenar los datos del mensaje (máx. 8 bytes)

        // Lee el mensaje del bus CAN y extrae el ID, la longitud y los datos
        CAN.readMsgBufID(&canId, &len, data);

        // Verifica si el mensaje recibido tiene el ID esperado (0x100) y 2 bytes de datos
        if (canId == 0x100 && len == 2) {
            // Convierte los dos bytes recibidos en un número entero (voltaje)
            int voltage = (data[0] << 8) | data[1];

            // Muestra el voltaje en el monitor serie
            Serial.print("Voltaje recibido: ");
            Serial.print(voltage);
            Serial.println(" V");
        }
    }
}
```

#### Código del receptor por interrupción
```cpp
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
```

## **6. Opciones de configuración**
CAN.begin(velocidad, reloj)

| **Parámetro**       | **Valores posibles** |  **Descripción**  |
|---------------------|----------------------|-------------------|
| Velocidad           | `CAN_NOBPS`, `CAN_5KBPS`, `CAN_10KBPS`, `CAN_20KBPS`, `CAN_25KBPS`, `CAN_31K25BPS`, `CAN_33KBPS`, `CAN_40KBPS`, `CAN_50KBPS`, `CAN_80KBPS`, `CAN_83K3BPS`, `CAN_95KBPS`, `CAN_95K2BPS`, `CAN_100KBPS`, `CAN_125KBPS`, `CAN_200KBPS`, `CAN_250KBPS`, `CAN_500KBPS`, `CAN_666KBPS`, `CAN_800KBPS`, `CAN_1000KBPS` | Configura la velocidad del bus CAN |
| Reloj               |  `MCP_NO_MHz`, `MCP_8MHz`, `MCP_12MHz`, `MCP_16MHz` | Configura la frecuencia del cristal del MCP2515 |

## **Explicación detallada de las funciones CAN en Arduino**

### **CAN.sendMsgBuf(ID, ext, len, data)**
📤 Envía un mensaje CAN con un ID específico.

| **Parámetro** | **Tipo** | **Descripción** |
|---------------|----------|-----------------|
| ID	          | long unsigned int	| Identificador del mensaje (puede ser estándar o extendido) |
| ext	          | byte     | Si es 0, usa un ID estándar (11 bits). Si es 1, usa un ID extendido (29 bits) |
| len	          | byte	   | Longitud del mensaje en bytes (máximo 8 bytes) |
| data          | byte[]   | Array con los datos a enviar |

### **CAN.checkReceive()**
👀 Verifica si hay mensajes CAN disponibles para leer.

| **Valor** | **Significado** |
|-----------|-----------------|
| `CAN_MSGAVAIL` | Hay un mensaje CAN en el buffer, listo para leer |
| `CAN_NOMSG` | No hay mensajes pendientes |

### **CAN.readMsgBufID(&id, &len, data)**
📥 Lee un mensaje CAN recibido y obtiene su ID y datos.

| **Parámetro** | **Tipo** | **Descripción** |
|---------------|----------|-----------------|
| id            | long unsigned int* | Dirección donde se almacenará el ID del mensaje |
| len           | byte*    | Dirección donde se almacenará la longitud del mensaje |
| data          | byte[]	| Array donde se almacenarán los datos recibidos |

### **CAN_OK**
✅ Indica que una operación CAN se realizó correctamente.

### **CAN_FAIL**	
❌ Fallo en la operación (por ejemplo, fallo al enviar un mensaje).

## **7. Plantillas de código**

A continuación, se presentan las plantillas para **emisor** y **receptor** CAN.  
Solo necesitas **rellenar los valores** donde se indica `/* ... */`.

---

## **📤 Plantilla para Emisor CAN**
📌 **Esta plantilla permite enviar datos por CAN.**  
📌 **Debes configurar el ID del mensaje, la longitud y los datos a enviar.**

```cpp
#include <SPI.h>
#include <mcp2515_can.h>  // Biblioteca para el MCP2515

#define CAN_CS_PIN 9  // Cambiar a 10 si usas SparkFun CAN Shield

mcp2515_can CAN(CAN_CS_PIN);  // Inicialización del CAN BUS

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Iniciando CAN BUS...");

    // Configura el CAN a la velocidad y frecuencia del cristal deseadas
    if (CAN.begin(/* VELOCIDAD_CAN */, /* FRECUENCIA_CRISTAL */) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);
    }
}

void loop() {
    // Genera un dato a enviar (modifica según necesidad)
    int dato = /* VALOR_A_ENVIAR */;
    
    // Convierte el dato en bytes (modifica si el dato tiene otro formato)
    byte data[/* TAMAÑO_DATOS */] = { /* DATOS_EN_BYTES */ };

    // Enviar el mensaje CAN
    if (CAN.sendMsgBuf(/* ID_MENSAJE */, /* TIPO_ID */, /* TAMAÑO_DATOS */, data) == CAN_OK) {
        Serial.print("Mensaje enviado: ");
        Serial.println(dato);
    } else {
        Serial.println("Error al enviar mensaje CAN");
    }

    delay(/* INTERVALO_ENVÍO_MS */);  // Tiempo entre envíos
}
```

## **📥 Plantilla para Receptor CAN**

### **CAN.checkReceive()**
📌 **Esta plantilla permite recibir datos por CAN.**
📌 **Debes definir qué hacer con los datos recibidos.**

```cpp
#include <SPI.h>
#include <mcp2515_can.h>  // Biblioteca para el MCP2515

#define CAN_CS_PIN 9  // Cambiar a 10 si usas SparkFun CAN Shield

mcp2515_can CAN(CAN_CS_PIN);  // Inicialización del CAN BUS

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Esperando mensajes CAN...");

    // Configura el CAN a la velocidad y frecuencia del cristal deseadas
    if (CAN.begin(/* VELOCIDAD_CAN */, /* FRECUENCIA_CRISTAL */) == CAN_OK) {
        Serial.println("CAN BUS inicializado correctamente");
    } else {
        Serial.println("Error al inicializar CAN BUS");
        while (1);
    }
}

void loop() {
    // Verifica si hay un mensaje disponible
    if (CAN.checkReceive() == CAN_MSGAVAIL) {
        long unsigned int canId;
        byte len;
        byte data[8];  // Buffer para los datos

        // Leer el mensaje recibido
        CAN.readMsgBufID(&canId, &len, data);

        // Filtrar por ID específico (modificar según necesidad)
        if (canId == /* ID_ESPERADO */ && len == /* TAMAÑO_ESPERADO */) {
            // Reconstruir el dato recibido (modifica si el formato cambia)
            int datoRecibido = (data[0] << 8) | data[1];

            Serial.print("Dato recibido: ");
            Serial.println(datoRecibido);
        }
    }
}
```

### **Interrupción**
📌 **Esta plantilla permite recibir datos por CAN.**
📌 **Debes definir qué hacer con los datos recibidos.**

```cpp
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
    if (CAN.begin(/* VELOCIDAD_CAN */, /* FRECUENCIA_CRISTAL */) == CAN_OK) {
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
        if (canId == /* ID_ESPERADO */ && len == /* TAMAÑO_ESPERADO */) {  // Suponiendo que el emisor usa ID 0x100
            // Reconstruir el dato recibido (modifica si el formato cambia)
            int datoRecibido = (data[0] << 8) | data[1];

            Serial.print("Voltaje recibido: ");
            Serial.print(datoRecibido);
            Serial.println(" V");
        }
    }
}
```

| **Parámetro** | **Descripción** | **Posibles valores** |
|---------------|-----------------|----------------------|
| `VELOCIDAD_CAN` | Velocidad del bus CAN | `CAN_5KBPS`, `CAN_100KBPS`, `CAN_500KBPS`, `CAN_1000KBPS`, etc. |
| `FRECUENCIA_CRISTAL` | Frecuencia del cristal del MCP2515 | `MCP_8MHz`, `MCP_12MHz`, `MCP_16MHz`, etc. |
| `ID_MENSAJE`    | ID del mensaje enviado | Número hexadecimal o decimal (ej. 0x100, 256) |
| `TIPO_ID`       | Tipo de ID (estándar o extendido) | `0` (ID estándar), `1` (ID extendido) |
| `TAMAÑO_DATOS`	 | Cantidad de bytes enviados | Entre 1 y 8 |
| `DATOS_EN_BYTES` | Array con los datos | Ejemplo: {0x12, 0x34} |
| `INTERVALO_ENVÍO_MS` | Tiempo entre envíos | En milisegundos (1000 para 1s) |
| `ID_ESPERADO` | ID del mensaje que el receptor debe procesar | Mismo ID que el emisor (0x100) |
| `TAMAÑO_ESPERADO` | Número de bytes esperados | Igual al tamaño enviado por el emisor |
