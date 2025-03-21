#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "credentials.h"

// Configuración WiFi
const char* ssid = SSID;
const char* password = PASSWORD;

// Configuración MQTT
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = 1883;
const char* mqtt_topic = "ideam/river_level";

Adafruit_SSD1306 display = Adafruit_SSD1306();

// Instancia de WiFi, MQTT y pantalla OLED
WiFiClient espClient;
PubSubClient client(espClient);

// Variables para almacenar los datos recibidos
String municipio = "Cargando...";
String max_nivel = "Cargando...";
String estado = "Cargando...";
String nivel = "Cargando...";
String fecha = "Cargando...";

void setup() {
  Serial.begin(115200);
  // Inicializar pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error al inicializar SSD1306"));
    for(;;); // No continuar si falla
  }
  
  // Limpiar pantalla
  display.clearDisplay();
  display.setTextSize(0);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Iniciando..."));
  display.display();
  
  // Conectar a WiFi
  setup_wifi();
  
  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.println("] ");
  
  // Convertir el payload a String
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  Serial.println(mensaje);
  
  // Extraer los datos del mensaje
  if (String(topic) == mqtt_topic ) {
    int pos1 = mensaje.indexOf("Municipio: ");
    int pos2 = mensaje.indexOf("\nMaxNivel: ");
    int pos3 = mensaje.indexOf("\nEstado: ");
    int pos4 = mensaje.indexOf("\nNivel Sensor: ");
    int pos5 = mensaje.indexOf("\nFecha: ");
    
    if (pos1 >= 0 && pos2 >= 0 && pos3 >= 0) {
      municipio = mensaje.substring(pos1 + 10, pos2);
      max_nivel = mensaje.substring(pos2 + 11, pos3);
      estado = mensaje.substring(pos3 + 9, pos4);
      nivel = mensaje.substring(pos4 + 15, pos5);
      fecha = mensaje.substring(pos5 + 8);
      
      // Actualizar pantalla
      actualizarPantalla();
    }
  }
}

void actualizarPantalla() {
  display.clearDisplay();
  display.setTextSize(0);
  display.setCursor(0, 0);

  display.println(municipio);

  // display.print("MaxNivel: ");
  // display.println(nivel);
  
  display.print("Estado: ");
  display.println(estado);

  display.print("Nivel: ");
  display.println(nivel);

  display.println(fecha);

  
  display.display();
}

void reconnect() {
  // Reconectar hasta que se establezca la conexión
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    
    // Intentar conectar
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
      
      // Suscribirse al tema
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      
      // Mostrar mensaje de error en pantalla
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Error MQTT");
      display.setCursor(0, 10);
      display.print("Estado: ");
      display.println(client.state());
      display.setCursor(0, 20);
      display.println("Reintentando...");
      display.display();
      
      // Esperar antes de reintentar
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Comprobar si WiFi sigue conectado
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
  delay(100);
}
