import mqtt.*;

MQTTClient client;
float[] distancias ;
// Para ejecutar, correr shiftr 
//NO ESTA FUNCIONANDO
// Recibe los mensajes del canal hello que son enteror y los grafica en tiempo real

void setup() {
  client = new MQTTClient(this);
  client.connect("mqtt://localhost:1883/1884", "processing");
  client.subscribe("hello");

  size(400, 200);
  background(255);
  graficar(distancias);

}

void draw() {}

void keyPressed() {
  client.publish("hello", "world");
}

void messageReceived(String topic, byte[] payload) {
  println( topic + ": " + new String(payload));
  // Recibir distancias, pasarlo a entero y meterlo al array
  
  // agregarDistancia(distancia);
}

void agregarDistancia(float nuevaDistancia){
  float[] nuevasDistancia = new float[distancias.length + 1];
  System.arraycopy(distancias, 0, nuevasDistancia, 0, distancias.length);
  nuevasDistancia[distancias.length] = nuevaDistancia;
  distancias = nuevasDistancia;  

  if (distancias.length > width / 10) {
    distancias = subset(distancias, 1);
  }
}

void graficar(float[] distancias){
  float xSpacio = width / (distancias.length - 1);

  // Plot the points
  for (int i = 0; i < distancias.length; i++) {
    float x = i * xSpacio;
    float y = height - distancias[i];
    ellipse(x, y, 8, 8);
  }
}
