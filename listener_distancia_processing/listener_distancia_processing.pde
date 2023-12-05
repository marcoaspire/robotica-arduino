import mqtt.*;

MQTTClient client;

// Recibe los mensajes del canal hello que son enteror y los grafica en tiempo real

void setup() {
  client = new MQTTClient(this);
  client.connect("mqtt://localhost", "processing");
  client.subscribe("hello");
}

void draw() {}

void keyPressed() {
  client.publish("hello", "world");
}

void messageReceived(String topic, byte[] payload) {
  println( topic + ": " + new String(payload));
}
