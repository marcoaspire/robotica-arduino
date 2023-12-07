import mqtt.*;

MQTTClient client;

void setup() {
  client = new MQTTClient(this);
  client.connect("mqtt://localhost", "processing");
  //client.connect("mqtt://localhost:1883/1884", "processing");
  client.subscribe("hello");
}

void draw() {}

void keyPressed() {
  client.publish("hello", "world");
}

void messageReceived(String topic, byte[] payload) {
  println( topic + ": " + new String(payload));
}
