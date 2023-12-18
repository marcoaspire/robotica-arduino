#define STP2 27
#define STP1 33
#define EN 25
#define DIR1 32
#define DIR2 26

#include <WiFi.h>
#include <MQTT.h>



WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;
// int velocidad = 2500;

int pulsos = 0;
int q=0;

// Custom class definition
class Coche {
private:
	int velocidad;  
public:
  // Constructor
  /*
    Rueda mide 7 cm, perimetro pi * diametro = 21.99 cm avanza cada vuelta
    Rueda mide 8 cm, perimetro pi * diametro = 25.13 cm avanza cada vuelta es esta
    22 cm - necesita 3200 pulsos aka 360 
    i.e 8.888 pulsos equivale a un grado  
    grados | pulsos
      180  | 3200 vuelta
       90 | 1600
       45  | 800
       22.5  | 400  
    velocidad angular = angulo rotado/ tiempo 
  */ 
  Coche() {
    pinMode(STP1, OUTPUT);
    pinMode(STP2, OUTPUT);
    pinMode(EN, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);
    digitalWrite(DIR1, LOW);
    digitalWrite(DIR2, LOW); 
    velocidad = 2500;
  }

  void reversa(int motor) {
    if (motor == 1 )
      digitalWrite(DIR1, HIGH); 
    if (motor == 2 )
      digitalWrite(DIR2, HIGH); 
  }

  void delante(int motor) {
    if (motor == 1 )
      digitalWrite(DIR1, LOW); 
    if (motor == 2 )
      digitalWrite(DIR2, LOW); 
  }

  // void avanzaVuelta(){
  //   int pulsos = 0;
  //   // Necesita 3200 pulsos para dar una vuelta completa
  //   while (pulsos < 3200) {
  //     digitalWrite( STP2, HIGH );
  //     digitalWrite( STP2, LOW );
  //     delay(10);
  //     pulsos++ ;
  //   } 
  // }

  void avanzaVuelta(){
    digitalWrite( STP2, HIGH );
    digitalWrite( STP1, HIGH );
    digitalWrite( STP2, LOW );
    digitalWrite( STP1, LOW );
    //delayMicroseconds(500);
    delayMicroseconds(velocidad); //2
  }

  void vueltaIzquierda(){
    //TODO: Calcula la cantidad de pasos necesarios para girar 45 grados, diametro de rueda?
    int vueltasNecesarias = 10;
    int vuetasRealizadas = 0;
    reversa(1);
    avanzaVuelta();

    /*
    while (vuetasRealizadas < vueltasNecesarias)
    {
      avanzaVuelta();
      vuetasRealizadas++;
    }
    */
    delante(1);
  }

  void vueltaIzquierdaAtras(){
    //ambos motores reversa 
    //TODO: Calcula la cantidad de pasos necesarios para girar 45 grados, diametro de rueda?
    int vueltasNecesarias = 10;
    int vuetasRealizadas = 0;
    reversa(2);
    avanzaVuelta();
    delante(1);
  }

  void detenerMotores() {
    // Detiene ambos motores
    digitalWrite(STP1, LOW);
    digitalWrite(STP2, LOW);
  }

  int aumentoVelocidad(){
    if (velocidad == 2500) {
      return 1500;
    } 
    return 750;   
  }

  int reduccionVelocidad(){
    if (velocidad == 750) {
      return 1500;
    } 
    return 2500;
  }

  void setVelocidad(int v){
    velocidad = v;
  }

  int getVelocidad(){
    return velocidad;
  }

};


Coche myCoche;

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

  client.subscribe("comandos");
}




void setup() {
  Serial.begin(115200);

  // start wifi and mqtt
  WiFi.begin(ssid, pass); 
  client.begin("192.168.0.18", net); // poner direccion ip uni 192.168.48.221, casa :192.168.0.18
  client.onMessage(messageReceived);

  connect();
  Serial.println("empieza...");
}

void frenado(int velocidadRecibida)
{
  int velocidadActual = velocidadRecibida;
  int pu = 0;
  if (velocidadActual == 750 )
  {
    myCoche.setVelocidad(1500);
    velocidadActual = 1500;
    while (pu < 201) {
      myCoche.avanzaVuelta();
      pu++;
    }
  }
  pu=0;
  if (velocidadActual == 1500 )
  {
    myCoche.setVelocidad(2500);
    while (pu < 201) {
      myCoche.avanzaVuelta();
      //myCoche.vueltaIzquierda();
      //delay(10); // 10 000 //1
      //delayMicroseconds(velocidad); //2

      // delayMicroseconds(1000); //2
      // delayMicroseconds(500); //3
      pu++;
    }
  }
  myCoche.setVelocidad(velocidadRecibida);
}  

void delantePulso()
{
  int pu = 0;
  // 12 cm
  while (pu < 1601) {
    myCoche.avanzaVuelta();
    //myCoche.vueltaIzquierda();
    //delay(10); // 10 000 //1
    //delayMicroseconds(velocidad); //2

    // delayMicroseconds(1000); //2
    // delayMicroseconds(500); //3
    pu++;
  } 
}

void giroIzquierdo()
{
  int q = 0;
  //180 - 3200
  while (q < 3300) {
    myCoche.vueltaIzquierda();

    //delay(10); // 1
    q++;
  }
}

void giroIzquierdoAtras()
{
  int q = 0;
  //180 - 3200
  while (q < 3300) {
    myCoche.vueltaIzquierdaAtras();

    //delay(10); // 1
    q++;
  }
}


int qwerty = 0;
void loop() {
  client.loop();
  delay(10);

  // check if connected
  if (!client.connected()) {
    connect();
  }
  /*
    Mensages posibles
    avanzar
    der
    izq
    delante
    atras
    subirVelocidad
    bajarVelocidad
    detener

    - para el frenado debe ser mucho mas corto despues del cambio de velocidad,
    - regresar a la velocidad anterior
    - 
  */

  // publish a message roughly every second.
  // if (millis() - lastMillis > 1000) {
  //   lastMillis = millis();
  //   if (qwerty < 1)
  //   { 
  //     Serial.println("diresccion: " + String(digitalRead(DIR1)));
  //     client.publish("/hello", "avanzar");
  //   }
  //   qwerty ++;
  // }
  
/*
  // 180 grados
  while (q < 3300) {
    myCoche.vueltaIzquierdaAtras();
    delay(10); // 1
    q++;
  } 
  // Necesito que distancia avanza
  // while (pulsos < 1601) {
  //   myCoche.avanzaVuelta();
  //   //myCoche.vueltaIzquierda();
  //   delay(10); // 10 milisegundos aka 10 000 micros
  //   // delayMicroseconds(1000); //2
  //   // delayMicroseconds(500); //3
    
  //   pulsos++;
  // } 
  if (pulsos == 3200)
  Serial.println("terminaando...");
  pulsos++;
*/  

}

void messageReceived(String &topic, String &payload) {
  Serial.println(topic + ": " + payload);
  if (payload.equals("avanzar")) {
    Serial.println("Entramos ");
    delantePulso();  
  } else if (payload.equals("der")) {
    //derecha();
  } else if (payload.equals("izq")) {
    if (digitalRead(DIR1) == 0 )
      giroIzquierdo();
    else 
      giroIzquierdoAtras();
  } else if (payload.equals("delante")) {
    myCoche.delante(1);
    myCoche.delante(2);
  } else if (payload.equals("atras")) {
    Serial.println("dd andtes " + String(digitalRead(DIR1)) );
    myCoche.reversa(1);
    myCoche.reversa(2);
    Serial.println("dd " + String(digitalRead(DIR1)) );
    Serial.println("dd2 " + String(digitalRead(DIR2)) );

    
  } else if (payload.equals("subirVelocidad")) {
    myCoche.setVelocidad(myCoche.aumentoVelocidad());
    Serial.println("Nueva velocidad " + String(myCoche.getVelocidad()));
    // subirVelocidad();
  } else if (payload.equals("bajarVelocidad")) {
    myCoche.setVelocidad(myCoche.reduccionVelocidad());
    Serial.println("Nueva velocidad " + String(myCoche.getVelocidad()));
    // bajarVelocidad();
  } else if (payload.equals("detener")) {
    int velocidadActual = myCoche.getVelocidad();
    if (velocidadActual < 2500){
      frenado(velocidadActual);
    }
    //detener();
  } else {
    // Manejar el caso por defecto (comando no reconocido)
    Serial.println("Comando no reconocido");
  }  
}
