#define STP2 27
#define STP1 33
#define EN 25
#define DIR1 32
#define DIR2 26

#define VEL1 2100
#define VEL2 1500
#define VEL3 850

#include <WiFi.h>
#include <MQTT.h>

WiFiClient net;
MQTTClient client;
unsigned long tiempoInicio;
float velocidadIzquierda;
float velocidadDerecha;
float velocidadAngular; 
float velocidadLineal; 
float velocidadSegundos;

unsigned long tiempoTranscurrido;

int b = 16;
  int x = 0;
  int y = 0;
  int theta = 0;

    AccelStepper motorIzquierdo(AccelStepper::DRIVER, STP1);
    AccelStepper motorDerecho(AccelStepper::DRIVER, STP2);
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
    25 cm - necesita 3200 pulsos aka 360 
    i.e 8.888 pulsos equivale a un grado  
    distincia | grados | pulsos
    25.13  |360 | 3200 | 8 s
    12.065 |180 | 1600 | 4 seg
    6.03   |90  | 800  | 2
    3.01   |45  | 400  | 1
    1.505  |22.5| 200  | .5
    .0335  |1   | 8.888| 
           |.05  |  1
    velocidad angular = angulo rotado/ tiempo 
    velocidad lineal = distancia / tiempo

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
    //velocidadIzquierda = motorIzquierdo.speed();
    //velocidadDerecha = motorDerecho.speed();
    //Serial.println("velocidadDer: " + String(velocidadDerecha));
    //Serial.println("velocidadIzq: " + String(velocidadIzquierda));
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
    delante(1);
    avanzaVuelta();
    reversa(1);
  }

  void vueltaDerecha(){
    reversa(2);
    avanzaVuelta();
    delante(2);
  }

  void vueltaDerechaAtras(){
    delante(2);
    avanzaVuelta();
    reversa(2);
  }

  void detenerMotores() {
    // Detiene ambos motores
    digitalWrite(STP1, LOW);
    digitalWrite(STP2, LOW);
  }

  int aumentoVelocidad(){
    if (velocidad == VEL1) {
      return VEL2;
    } 
    return VEL3;   
  }

  int reduccionVelocidad(){
    if (velocidad == VEL3) {
      return VEL2;
    } 
    return VEL1;
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

// void multiplicarMatriz(float matriz [][2]){
//   int res [][2];
//   for (int i=0; i<matriz.length(); i++)
//     for (int j=0; j<matriz[0].length();j++)
//       res[i][j]= matriz[i][j]  ; 

// }

void actualizar_posicion(float matriz_transformacion [][2]){
  int vel_lineal = matriz_transformacion[0][0] *velocidadIzquierda + matriz_transformacion[0][1] * velocidadDerecha;
  int vel_angular = matriz_transformacion[1][0]*velocidadIzquierda + matriz_transformacion[1][1] * velocidadDerecha;
  // actualiza 
  x += vel_lineal * cos(theta);
  y += vel_lineal * sin(theta);
  theta += vel_angular;

}

void setup() {

  float matriz_transformacion[2][2]={{1/2,1/2},{-1/2*b,1/2*b}};

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
  if (velocidadActual == VEL3 )
  {
    myCoche.setVelocidad(VEL2);
    velocidadActual = VEL2;
    while (pu < 201) {
      myCoche.avanzaVuelta();
      pu++;
    }
  }
  pu=0;
  if (velocidadActual == VEL2 )
  {
    myCoche.setVelocidad(VEL1);
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
  tiempoInicio = micros();
  int pu = 0;
  // 12 cm
  while (pu < 1601) {
    myCoche.avanzaVuelta();

    // no
    // velocidadIzquierda = motorIzquierdo.speed();
    // if (velocidadIzquierda != 0)
    //   Serial.println("2velocidadIzq: " + String(velocidadIzquierda));
    // else
    //   Serial.print("@");

    //myCoche.vueltaIzquierda();
    //delay(10); // 10 000 //1
    //delayMicroseconds(velocidad); //2

    // delayMicroseconds(1000); //2
    // delayMicroseconds(500); //3
    pu++;
  } 

  // Medir el tiempo transcurrido
  tiempoTranscurrido = micros() - tiempoInicio;

  // 1M micros = 1 s
  // x miros


  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos =  tiempoTranscurrido/ 1000000.0;
  velocidadAngular = pu / velocidadSegundos;
  //Tiempo
  Serial.print("Tiempo: ");
  Serial.println(velocidadSegundos);

  // Mostrar la velocidad en el puerto serie
  Serial.print("Velocidad angular actual: ");
  Serial.println(velocidadAngular);

  velocidadLineal = 12.065/ velocidadSegundos;  // cm por seg, esta es velocidad lineal de los motores
  Serial.print("Velocidad lineal actual: ");
  Serial.println(velocidadLineal);


  // Reiniciar el tiempo para la siguiente medición
  tiempoInicio = micros();
}

void giroIzquierdo()
{
  tiempoInicio = micros();
  int q = 0;
  /* 
    grados | pulsos
      180  | 3200 vuelta
       90  | 1600
       45  | 800
       22.5| 400  
  */
  while (q < 400) {
    myCoche.vueltaIzquierda();
    q++;
  }
  // Medir el tiempo transcurrido
  tiempoTranscurrido = micros() - tiempoInicio;

  //Conociendo el angulo, la trayectoria = radio por angulo en radianes
  // 180 = pi radianes
  float anguloRadianes = 22.5*3.1416/180;
  float distancidaRecorrida = 4*anguloRadianes;
  Serial.print("Distancia recorrida vuelta izq cm: ");
  Serial.println(distanciaRecorrida);

  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos =  tiempoTranscurrido/ 1000000.0;
  velocidadLineal = distanciaRecorrida/velocidadSegundos;

  Serial.print("Velocidad lineal actual: ");
  Serial.println(velocidadLineal);

   // Reiniciar el tiempo para la siguiente medición
  tiempoInicio = micros();
}

void giroIzquierdoAtras()
{
  int q = 0;
  //180 - 3200
  while (q < 400) {
    myCoche.vueltaIzquierdaAtras();

    //delay(10); // 1
    q++;
  }
}

void giroDerecho()
{
  int q = 0;
  while (q < 400) {
    myCoche.vueltaDerecha();
    q++;
  }
}

void giroDerechoAtras()
{
  int q = 0;
  while (q < 400) {
    myCoche.vueltaDerechaAtras();
    q++;
  }
}


int qwerty = 0;
void loop() {
    //Serial.println("1velocidadDer: " + String(velocidadDerecha));

    // no funciono
    // velocidadIzquierda = motorIzquierdo.speed();
    // if (velocidadIzquierda != 0)
    //   Serial.println("2velocidadIzq: " + String(velocidadIzquierda));
    // else
    //   Serial.print("@");

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
    if (digitalRead(DIR1) == 0 )
      giroDerecho();
    else 
      giroDerechoAtras();
  } else if (payload.equals("izq")) {
    if (digitalRead(DIR1) == 0 )
      giroIzquierdo();
    else 
      giroIzquierdoAtras();
  } else if (payload.equals("delante")) {
    myCoche.delante(1);
    myCoche.delante(2);
  } else if (payload.equals("atras")) {
    myCoche.reversa(1);
    myCoche.reversa(2);
  } else if (payload.equals("subirVelocidad")) {
    myCoche.setVelocidad(myCoche.aumentoVelocidad());
    Serial.println("Nueva velocidad " + String(myCoche.getVelocidad()));
  } else if (payload.equals("bajarVelocidad")) {
    myCoche.setVelocidad(myCoche.reduccionVelocidad());
    Serial.println("Nueva velocidad " + String(myCoche.getVelocidad()));
  } else if (payload.equals("detener")) {
    int velocidadActual = myCoche.getVelocidad();
    if (velocidadActual < 2500){
      frenado(velocidadActual);
    }
  } else {
    // Manejar el caso por defecto (comando no reconocido)
    Serial.println("Comando no reconocido");
  }  
}
