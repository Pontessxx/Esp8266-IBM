#include <WiFi.h>
#include <PubSubClient.h>

 

const char* SSID = "FIAP-IBM";
const char* PASSWORD = "Challenge@23!";
const char* BROKER_MQTT = "46.17.108.113";
const int BROKER_PORT = 1883;
const char* TOPICO_PUBLISH = "/TEF/lamp118/attrs";
const char* ID_MQTT = "fiware_118";

 

const int ledPin = 26  ; // Pino G14 para o LED

 

WiFiClient espClient;
PubSubClient client(espClient);

 

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

 

  // Conecta-se à rede Wi-Fi
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

 

  // Conecta-se ao broker MQTT
  client.setServer(BROKER_MQTT, BROKER_PORT);
  client.setCallback(callback);
}

 

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Conteúdo da mensagem: ");
  Serial.println(message);

 

  if (String(topic) == TOPICO_PUBLISH) {
    if (message == "s|on") {
      digitalWrite(ledPin, HIGH); // Liga o LED
      Serial.println("LED ligado");
    } else if (message == "s|off") {
      digitalWrite(ledPin, LOW); // Desliga o LED
      Serial.println("LED desligado");
    }
  }
}

 

void reconnect() {
  while (!client.connected()) {
    Serial.println("Conectando ao MQTT Broker...");
    if (client.connect(ID_MQTT)) {
      Serial.println("Conectado ao MQTT Broker");
      client.subscribe(TOPICO_PUBLISH);
    } else {
      Serial.println("Falha na conexão ao MQTT Broker. Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

 

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
