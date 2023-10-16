#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Define suas credenciais WiFi
const char* SSID = "HPMP-2G";
const char* PASSWORD = "97531HeMpe";

// Define as configurações MQTT
const char* BROKER_MQTT = "46.17.108.113"; // Endereço do broker MQTT
int BROKER_PORT = 1883; // Porta do broker MQTT
const char* TOPICO_SUBSCRIBE = "/TEF/lamp118/cmd";
const char* TOPICO_PUBLISH = "/TEF/lamp118/attrs";
const char* TOPICO_PUBLISH_2 = "/TEF/lamp118/attrs/color"; // Tópico para a cor do LED
const char* ID_MQTT = "fiware_108";

// Define os pinos para os LEDs
const int RED_LED_PIN = D1;
const int GREEN_LED_PIN = D2;
const int BLUE_LED_PIN = D3;
const int OUTPUT_PIN = D4; // Pino de saída para o estado do LED

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';
float polution = 0.0;

void initSerial() {
    Serial.begin(115200);
}


void initWiFi() {
    delay(10);
    Serial.println("------Conexão WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;

    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }

    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    if (msg.equals("lamp118@on|")) {
        digitalWrite(OUTPUT_PIN, HIGH);
        EstadoSaida = '1';
    }

    if (msg.equals("lamp118@off|")) {
        digitalWrite(OUTPUT_PIN, LOW);
        EstadoSaida = '0';
    }
}

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);

        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFIEMQTT(void) {
    if (!MQTT.connected())
        reconnectMQTT();

    reconectWiFi();
}

void EnviaEstadoOutputMQTT(void) {
    String estadoLED;
    
    if (polution <= 33.0) {
        estadoLED = "Red";
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(BLUE_LED_PIN, LOW);
        Serial.print("Red");
        EstadoSaida == '1';
    } else if (polution > 33.0 && polution <= 66.0) {
        estadoLED = "Yellow";
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(BLUE_LED_PIN, LOW);
       Serial.print("Yellow");
        EstadoSaida == '0';
    } else {
        estadoLED = "Green";
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(BLUE_LED_PIN, HIGH);
        Serial.print("Green");
         EstadoSaida == '0';
    }

    MQTT.publish(TOPICO_PUBLISH_2, estadoLED.c_str()); // Publish color to "/TEF/lamp118/attrs/color"



    //verifica o estadoSaida
    if (EstadoSaida == '1') {
        MQTT.publish(TOPICO_PUBLISH, "s|on");
        Serial.println("- Humidity Ligado");
    }

    if (EstadoSaida == '0') {
        MQTT.publish(TOPICO_PUBLISH, "s|off");
        Serial.println("- Humidity Desligado");
    }

    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}

void InitOutput(void) {
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(OUTPUT_PIN, OUTPUT);
    digitalWrite(OUTPUT_PIN, HIGH);

    boolean toggle = false;

    for (int i = 0; i <= 10; i++) {
        toggle = !toggle;
        digitalWrite(OUTPUT_PIN, toggle);
        delay(200);
    }

    digitalWrite(OUTPUT_PIN, LOW);
}

void setup() {
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH, "s|off");
}

void loop() {
    const int potPin = A0; // Use the correct analog pin for your LDR.

    char msgBuffer[4];
    VerificaConexoesWiFIEMQTT();

    int sensorValue = analogRead(potPin);
    float voltage = sensorValue * (3.3 / 1024.0);
    polution = map(sensorValue, 0, 1023, 0, 100);
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.print("V - ");
    Serial.print("Polution: ");
    Serial.print(polution);
    Serial.println("%");
    dtostrf(polution, 4, 2, msgBuffer);

    EnviaEstadoOutputMQTT(); // Envie a cor do LED ao MQTT

    MQTT.loop();
}
