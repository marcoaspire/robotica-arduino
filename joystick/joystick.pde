import mqtt.*;

MQTTClient client;
int tiempoEspera = 1300;  // Tiempo de espera en milisegundos (1 segundo)
int ultimoTiempoPresionado = 0;
int botonSize = 50;  // Tamaño de los botones
float[] distancias = {0} ;
float dis= 80;

float robotX, robotY;
float robotAngle;
ArrayList<PVector> path;


String opcion;
String distanciaRecibida;

void setup() {
  size(700, 600);
  
  client = new MQTTClient(this);
  //client.connect("mqtt://localhost", "processing");
  client.connect("mqtt://localhost:1883/1884", "processing");
  client.subscribe("comandos");
  path = new ArrayList<PVector>();
  path.add(new PVector(0, 0));
}

void draw() {
  
  background(255);
  // Muestra la trayectoria
  stroke(#009EFF); // azul
  noFill();
  translate(600/2, height/2);
  textSize(64);
  text(dis, 280, 0);
  beginShape();
   vertex(-280, -280);
   vertex(-280, 280);
   vertex(280, 280);
   vertex(280, -280);
  endShape(CLOSE);
  beginShape();

    for (PVector point : path) {
      vertex(point.x, point.y);
    }
  
  endShape();

  // Muestra el robot en la posición actual
  drawRobot(robotX, robotY, robotAngle);

}

void drawRobot(float x, float y, float angle) {
  //println("drawX:" + robotX + " y:"+y + " angle:"+angle);
  
  if (x < 270 && x > -270 && y < 270 && y > -270 )
  {
    pushMatrix();
    translate(x, y);
    rotate(angle);
    fill(150); // cambiar color si detecta uno cercano
    rect(-5, -5, 20, 10); // robot
    stroke(#55FF00); 
      if (dis<40 || x>250 || x < -250 || y> 250 || y < -250 )
      {
        stroke(#FF2D00); //red
        forzarVuelta(x,y,true, 0,0);
      }
    line(0, 0, dis, 0); // AQUI PONER la distancia que recibe , x,y,w,h
    popMatrix();
  }
  else{
    stroke(#55FF00); 
    line(0, 0, dis, 0);
    forzarVuelta(x,y,true,0,0);
    // Se sale del recuadro
    // hacerlo girar y que avance a otra posicion
  }
}

void dibujarBoton(float x, float y, float size, String tag) {
  // Dibuja un botón en la posición (x, y)
  fill(200);
  rect(x, y, size, size);
  fill(0);
  textAlign(CENTER, CENTER);
  textSize(12);
  text(tag, x + size / 2, y + size / 2);
}

void mousePressed() {
  // Verifica si se ha hecho clic en alguno de los botones
  checkButton(width / 2 - botonSize / 2, height / 2 - botonSize * 2, botonSize, 1); //arriba
  checkButton(width / 2 - botonSize / 2, height / 2 + botonSize, botonSize, 2); // abajo
  checkButton(width / 2 - botonSize * 2, height / 2 - botonSize / 2, botonSize, 3);//izq
  checkButton(width / 2 + botonSize, height / 2 - botonSize / 2, botonSize, 4); //der
}

void checkButton(float x, float y, float size, int boton) {
  // Verifica si el clic del mouse está dentro del botón y muestra un mensaje en la consola
  if (mouseX >= x && mouseX <= x + size && mouseY >= y && mouseY <= y + size) {
    println("Botón presionado: " + boton);
    //client.publish("comandos", boton);
  }
}

void keyPressed() {

  if (key == 'A' && keyPressed == true && keyEvent.isShiftDown() && keyEvent.getKey() == 'B') {
    println("Las teclas A y B están siendo presionadas al mismo tiempo.");
    // Puedes agregar aquí cualquier acción que desees realizar cuando ambas teclas estén presionadas.
  }

  if (millis() - ultimoTiempoPresionado > tiempoEspera) {
    // Verifica si la tecla "A" ha sido presionada
    if (key == ' ' || key == 'f' || key == 'F' ) {
      println("La tecla f freno.");
      client.publish("comandos", "detener");
    } else if (key == 'd' || key == 'D') {
      println("La tecla d hacia delate.");
      client.publish("comandos", "delante");
    } else if (key == 'a' || key == 'A' ) {
      println("La tecla hacia atras.");
      client.publish("comandos", "atras");
    } else if (key == 'w' || key == 'W' ) {
      println("La tecla subir velocidad");
      client.publish("comandos", "subirVelocidad");
    } else if (key == 's' || key == 'S' ) {
      println("La tecla bajar velocidad");
      client.publish("comandos", "bajarVelocidad");
    } else if (key == CODED) {
      if (keyCode == UP) {
        println("La tecla UP");
        client.publish("comandos", "avanzar");
      } 
     if (keyCode == LEFT) {
        client.publish("comandos", "izq");
      } 
      if (keyCode == RIGHT) {
        client.publish("comandos", "der");
      }
    }
    // Actualizar el tiempo de la última tecla presionada
    ultimoTiempoPresionado = millis();
  }
}


void messageReceived(String topic, byte[] payload) {
  opcion = new String(payload);
  opcion.trim();
  if ((opcion.startsWith("D=") || opcion.startsWith("d="))) {
    distanciaRecibida = opcion.substring(2, opcion.length() - 1);
    dis = Float.parseFloat(distanciaRecibida);
    // println("distancia recibida");

    agregarDistancia(Float.parseFloat(distanciaRecibida));
  }
  else if (opcion.startsWith("X=") || opcion.startsWith("x=")) {

    String[] posiciones = opcion.split("[=,;]");
    println(   "x: " +  posiciones[1] + " Y:"+  posiciones[3]);

    float nuevoX = Float.parseFloat(posiciones[1])*10;
    float nuevoY =  Float.parseFloat(posiciones[3])*10;
    float nuevoAngulo =  Float.parseFloat(posiciones[5]);
    float posibleX2 = Float.parseFloat(posiciones[7])*10;
    float posibleY2 =  Float.parseFloat(posiciones[9])*10;

    robotX = nuevoX;
    robotY = nuevoY;
    robotAngle = nuevoAngulo;
    println(   "robotX: " +  nuevoX + " robotY:"+  nuevoY + ",px:" + posibleX2+ ",pY"+ posibleY2);
    forzarVuelta(nuevoX,nuevoY, false,posibleX2,posibleY2);
    path.add(new PVector(robotX, robotY));  
  }
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

void forzarVuelta(float x,float y, boolean movimientoPorDistancia, float x2, float y2)
{
  if (x2 > 250 || movimientoPorDistancia)
  {
      println("aqui debe x2: " + x2 + " y2" + y2);

    if (y2 > 0)
    {
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "der");
        delay(1000);
      }
    }
    else
    {
      // a la izquierda
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "izq");
        delay(1000);
      }
    }
    movimientoPorDistancia = false;
  }
  if (x2 < -250)
  {
    if (y2 > 0)
    {
      // a la izquierda
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "izq");
        delay(1000);
      }
    }
    else{
      // a la derecha
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "der");
        delay(1000);
      }
    }
  }
  if (y2 >= 250)
  {
    if (x2 > 0)
    {
      // a la derecha
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "der");
        delay(1000);
      }
    }
    else{
      // a la izquierda
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "izq");
        delay(1000);
      }
    }
  }
  if (y2 < -230)
  {
    if (x2 < 0)
    {
      // a la izquierda
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "izq");
        delay(1000);
      }
    }
    else{
      // a la derecha
      for (int i = 0; i < 4; i++) {
        client.publish("comandos", "der");
        delay(1000);
      }
    }
  }
}
