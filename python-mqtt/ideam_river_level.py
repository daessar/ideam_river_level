import requests
import paho.mqtt.client as mqtt
import logging
import urllib3
import os
from dotenv import load_dotenv


load_dotenv()
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

logging.basicConfig(
    filename='ideam_river_level.log',
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    filemode='w',
)

MQTT_BROKER = os.getenv("MQTT_BROKER")
MQTT_PORT = 1883
MQTT_TOPIC = "ideam/river_level"

URL_IDEAM = "https://fews.ideam.gov.co/visorfews/data/ReporteTablaEstacionesHsim.json"
ID_ESTACION = "0023037010" # ID puerto salgar
URL_IDEAM_SENSOR = "https://fews.ideam.gov.co/visorfews/data/series/jsonH/0023037010.json"

def get_data_sensor():
    try:
        
        response = requests.get(URL_IDEAM_SENSOR, timeout=10, verify=False)
        if response.status_code == 200:
            data = response.json()
            sensor_data = data.get('Hsen', {}).get('data', [])
            last_five_values = [item for item in sensor_data[-5:] if item.get('Hsen', '') is not None]
            if last_five_values:
                last_value = last_five_values[-2]
                return last_value
            else:
                logging.error("No se encontraron datos válidos en el sensor")
                return None
            
        else:    
            logging.error(f"Error al consultar la API: Código {response.status_code}")
            return None
    except Exception as e:
        logging.error(f"Error en la consulta del sensor: {str(e)}")
        return None

def obtener_datos_estacion():
    """Función para consultar la API del IDEAM y extraer los datos de la estación deseada"""
    try:
        # Realizar la petición HTTP
        response = requests.get(URL_IDEAM, timeout=10, verify=False)
        
        # Verificar si la respuesta fue exitosa
        if response.status_code == 200:
            # Parsear los datos JSON
            data = response.json()
            
            # Buscar la estación con el ID específico
            for feature in data.get('features', []):
                properties = feature.get('properties', {})
                if properties.get('id') == ID_ESTACION:
                    # Extraer los datos requeridos
                    maxnivel = properties.get('maxnivel', 'N/A')
                    municipio = properties.get('municipio', 'N/A')
                    estado = properties.get('Estado', 'N/A')
                    
                    return {
                        'municipio': municipio,
                        'maxnivel': maxnivel,
                        'estado': estado
                    }
            
            logging.error(f"No se encontró la estación con ID {ID_ESTACION}")
            return None
        else:
            logging.error(f"Error al consultar la API: Código {response.status_code}")
            return None
    except Exception as e:
        logging.error(f"Error en la consulta: {str(e)}")
        return None

def enviar_datos_mqtt(datos, nivel_sensor):
    """Función para enviar los datos al broker MQTT"""
    try:
        # Crear cliente MQTT
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        
        # Conectar al broker
        client.connect(MQTT_BROKER, MQTT_PORT, 60)

        logging.info(f"Datos obtenidos - Municipio: {datos['municipio']}, MaxNivel: {datos['maxnivel']}mts, Estado: {datos['estado']}, Nivel Sensor: {nivel_sensor['Hsen']}mts, Fecha: {nivel_sensor['Fecha']}")

        
        mensaje = f"Municipio: {datos['municipio']}\nMaxNivel: {datos['maxnivel']}mt\nEstado: {datos['estado']} \nNivel Sensor: {nivel_sensor['Hsen']} mts \nFecha: {nivel_sensor['Fecha']}"
        
        # Publicar el mensaje
        result = client.publish(MQTT_TOPIC, mensaje)
        
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            logging.info("Mensaje publicado correctamente en MQTT")
        else:
            logging.error(f"Error al publicar mensaje: {result.rc}")
        
        client.disconnect()
        return True
    except Exception as e:
        logging.error(f"Error al enviar datos por MQTT: {str(e)}")
        return False

def main():
    """Función principal"""
    logging.info("Iniciando consulta de datos IDEAM")
    
    datos = obtener_datos_estacion()
    nivel_sensor = get_data_sensor()
    if datos and nivel_sensor:
        enviar_datos_mqtt(datos, nivel_sensor)
    else:
        logging.error("No se pudieron obtener datos para enviar")

if __name__ == "__main__":
    main()