# IDEAM River Level Monitoring
Este proyecto es un monitor personalizado para los niveles del río Magdalena que consta de dos componentes principales:

### Estructura del Proyecto
```
ideam_river_level/
├── esp32/                    # Código para el dispositivo ESP32
├── python-mqtt/              # Scripts de Python para la recolección de datos
└── python-bot/               # Componente de bot (sin detalles)
```
### Esquema
![Texto alternativo](/assets/arquitectura.png "Título alternativo")

### Componentes
### 1. Script Python (Recolector de Datos)
ideam_river_level.py es un script que:
- Obtiene datos del nivel del río desde la API del IDEAM
- Procesa los datos para extraer información relevante
- Publica los datos a través de MQTT para que sean consumidos por el ESP32
- Registra eventos en ideam_river_level.log
### 2. Dispositivo ESP32
ideam_river_level.ino es un firmware para ESP32 que:

- Se conecta a una red WiFi utilizando credenciales almacenadas en credentials.h
- Se suscribe a un tema MQTT para recibir datos del nivel del río
- Muestra la información recibida en una pantalla OLED (SSD1306)
- Visualiza datos como: municipio, nivel máximo, estado actual, nivel actual y fecha
### Instalación y Configuración
#### Python
- Crea un entorno virtual (incluido en ideam_river_level)
- Instala las dependencias: `pip install -r requirements.txt`
- Configura las variables de entorno en el archivo `.env:`
- `MQTT_BROKER` - Dirección del broker MQTT
#### ESP32
- Instala las bibliotecas necesarias en Arduino IDE:
```
- WiFi
- PubSubClient (MQTT)
- Adafruit_GFX
- Adafruit_SSD1306
```
- Crea un archivo `credentials.h` con las siguientes constantes:
- `SSID` - Nombre de la red WiFi
- `PASSWORD` - Contraseña WiFi
- `MQTT_SERVER` - Dirección del broker MQTT
### Funcionamiento
1. El script Python consulta periódicamente la API del IDEAM para obtener datos del nivel del río
Procesa estos datos y los publica en el tema MQTT `ideam/river_level`
2. El dispositivo ESP32 recibe estos datos y los muestra en la pantalla OLED
Personalización
3. Si necesitas monitorear una estación diferente, modifica la constante ID_ESTACION en el archivo Python.
