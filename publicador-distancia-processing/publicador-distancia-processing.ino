#include <WiFi.h>
#include <MQTT.h>
/* Sensor mide la distancia, esta se debe mandar por el canal /hello
  En processing se lee, y grafica en tiempo real
*/
const int EchoPin = 18;
const int TriggerPin = 17;
const int LedPin = 13;

const String IPAddress = "192.168.0.18"; 
// const char ssid[] = "iot";
// const char pass[] = "";

const char ssid[] = "Piso-";
const char pass[] = "";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println(topic + ": " + payload);
}

int ping(int TriggerPin, int EchoPin) {
  long duration, distanceCm;
  digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  duration = pulseIn(EchoPin, HIGH);     //medimos el tiempo entre pulsos, en microsegundos
  distanceCm = duration * 10 / 292 / 2;  //convertimos a distancia, en cm
  return distanceCm;
}

void setup() {
  Serial.begin(115200);
  pinMode(LedPin, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  // start wifi and mqtt
  WiFi.begin(ssid, pass);
  // client.begin("192.168.48.221", net);
  client.begin("192.168.0.18", net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(10);

  // check if connected
  if (!client.connected()) {
    connect();
  }
  int cm = ping(TriggerPin, EchoPin);
  //Serial.print("Distancia: ");
  //Serial.println(cm);
  delay(1000);

  // publish a message roughly every second.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    client.publish("/hello", String (cm));
  }
}