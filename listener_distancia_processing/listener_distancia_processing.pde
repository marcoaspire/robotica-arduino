import mqtt.*;

// Recibe los mensajes del canal hello que son enteror y los grafica en tiempo real
MQTTClient client;
float[] distancias = {0} ;

void setup() {
  client = new MQTTClient(this);
  client.connect("mqtt://localhost", "processing1230090");
  //client.connect("mqtt://localhost:1883/1884", "processing");
  size(400, 210); //width and height
  background(255);
  client.subscribe("hello");
}

void draw() {
  background(255);
  graficar(distancias);
}

void keyPressed() {
  //float newDataPoint = random(0, 50);
  //agregarDistancia(newDataPoint);
  client.publish("hello", newDataPoint+"");
}

void messageReceived(String topic, byte[] payload) {
  println( topic + "v: " + new String(payload));
  float distancia = Float.parseFloat(new String(payload));
  if (distancia < 51)
    agregarDistancia(distancia);
  println( topic + ": " + distancia+ "");
}

void dispose() {
  println("Adios ");
  // Disconnect from the MQTT broker when the sketch is closed
  if (client != null) {
    client.unsubscribe("hello");
    client.disconnect();
  }
  super.dispose();
}

void connectionLost() {
  println("connection lost");
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
  if (distancias.length > 1)
  {
    float xSpacio = width / (distancias.length - 1);
  
    // Plot the points
    for (int i = 0; i < distancias.length; i++) {
      float x = i * xSpacio;
      float y = height - distancias[i]*4;
      ellipse(x, y, 8, 8);
    }
    // Connect the points with lines
    for (int i = 0; i < distancias.length - 1; i++) {
      float x1 = i * xSpacio;
      float y1 = height - distancias[i]*4;
      float x2 = (i + 1) * xSpacio;
      float y2 = height - distancias[i + 1]*4;
      line(x1, y1, x2, y2);
    }
  }
}
