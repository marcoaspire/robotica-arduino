import mqtt.*;

MQTTClient client;
int tiempoEspera = 1000;  // Tiempo de espera en milisegundos (1 segundo)
int ultimoTiempoPresionado = 0;
int botonSize = 50;  // Tamaño de los botones
boolean [] vuelta;
void setup() {
  size(400, 400);
  vuelta = new boolean[2];
  vuelta[0] = false;
  vuelta[1] = false; 
  
  client = new MQTTClient(this);
  client.connect("mqtt://localhost", "processing");
  //client.connect("mqtt://localhost:1883/1884", "processing");
  client.subscribe("comandos");
  
}

void draw() {
  background(255);
  
  // Dibuja los botones en forma de cruz
  dibujarBoton(width / 2 - botonSize / 2, height / 2 - botonSize * 2, botonSize, "△");
  dibujarBoton(width / 2 - botonSize / 2, height / 2 + botonSize, botonSize, "▽");
  dibujarBoton(width / 2 - botonSize * 2, height / 2 - botonSize / 2, botonSize, "izq");
  dibujarBoton(width / 2 + botonSize, height / 2 - botonSize / 2, botonSize, "der");
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
      vuelta[0] = true;
      // if (vuelta[0] && vuelta[1])
      // {
      //   println("Una vuelta");
      // }
    } 
    // if (keyCode == DOWN) {
    //   vuelta[0] = true;
    //   println("La tecla DOWN");
    //   if (vuelta[0] && vuelta[1])
    //   {
    //     println("La vuelta a la derecha");
    //   }
    // }
    if (keyCode == LEFT) {
      client.publish("comandos", "izq");
      vuelta[1] = true;
      // if (vuelta[0] && vuelta[1])
      // {
      //   println("La vuelta a la izq"); 
      // }
    } 
    if (keyCode == RIGHT) {
      client.publish("comandos", "der");
      vuelta[1] = true;
      // if (vuelta[0] && vuelta[1])
      // {
      //   println("La vuelta a la derecha");
      // }
    }
  }
    // Actualizar el tiempo de la última tecla presionada
    ultimoTiempoPresionado = millis();
  }
}
void keyReleased() {
  
  // Verifica si la tecla "A" ha sido presionada
  if (key == ' ' || key == 's' || key == 'S' ) {
    println("Solto La tecla s ha sido presionada.");
    // Puedes agregar aquí cualquier acción que desees realizar cuando se presiona la tecla "A"
  } else if (key == CODED) {
    if (keyCode == UP) {
      println("Solto La tecla UP");
      //client.publish("comandos", "detener");

      vuelta[0] = false;
    } else if (keyCode == LEFT) {
      vuelta[1] = false;
      println("Solto La tecla LEFT");
      //client.publish("comandos", "detener");

    } else if (keyCode == RIGHT) {
      vuelta[1] = false;
      println("Solto La tecla RIGHT");
      //client.publish("comandos", "detener");
    }
  }
}


void messageReceived(String topic, byte[] payload) {
  println( topic + ": " + new String(payload));
}
