// Motores
#define STP2 27
#define STP1 33
#define EN 25
#define DIR1 32
#define DIR2 26

// sensor 
const int EchoPin = 18;
const int TriggerPin = 17;
const int LedPin = 13;
//velocidades
#define VEL1 2100
#define VEL2 1500
#define VEL3 850

//Datos de la rueda
#define GIR 22.5  // Grados que se quiere que se rote al presionar tecla
#define RADIORUEDA 4
#define PI 3.1416
#define PULSOSRUEDACOMPLETA 3200

#include <WiFi.h>
#include <MQTT.h>

// const char ssid[] = "iot-ieya";
// const char pass[] = "C@IoT#148";

// const char ssid[] = "Piso-sagemcom2E90";
// const char pass[] = "WYMRW4EMMW2MMD";

const char ssid[] = "Galaxy";
const char pass[] = "123456789";

String comando;

int b = 16; // distancia entre ruedas
WiFiClient net;
MQTTClient client;
unsigned long tiempoInicio;
float velocidadIzquierda;
float velocidadDerecha;
float velocidadAngular; 
float velocidadLineal; 
float velocidadSegundos;
float velocidadMotores;
float perimetroRueda =  2 * PI * RADIORUEDA;
float matriz_transformacion[2][2]={{1.0/2.0,1.0/2.0},{-b/2.0,b/2.0}};

unsigned long tiempoTranscurrido;
unsigned long lastMillis = 0;
bool obstaculoDetectado = false;
int limiteDistancia = 40;


float x = 0;
float x2 ;
float y = 0;
float y2;
float theta = 0;

int pulsos = 0;
int q=0;

bool mensajeRecibido = false;

class Coche {
private:
	int velocidad;
public:
  bool vaReversa;
  bool vaIzquierda;  
  // Constructor
  /*
    Datos calculados
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
    velocidad angular = arco / tiempo 
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
    vaReversa = false;
    vaIzquierda = false;
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
    delayMicroseconds(velocidad); //2
  }

  void vueltaIzquierda(){
    reversa(1);
    avanzaVuelta();
    vaIzquierda = true;
    delante(1);
  }

  void vueltaIzquierdaAtras(){
    //ambos motores reversa 
    int vueltasNecesarias = 10;
    int vuetasRealizadas = 0;
    delante(1);
    avanzaVuelta();
    reversa(1);
    vaIzquierda = true;
  }

  void vueltaDerecha(){
    reversa(2);
    avanzaVuelta();
    delante(2);
    vaIzquierda = false;
  }

  void vueltaDerechaAtras(){
    delante(2);
    avanzaVuelta();
    reversa(2);
    vaIzquierda = false;
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

void actualizarPosicion(float velocidadM, float angulo){
  float vel_lineal = 0;
  float vel_angular = 0;

  if (angulo != 0)
    vel_angular = velocidadM;
  else
    vel_lineal = velocidadM;
  
  if (myCoche.vaIzquierda && angulo != 0)
  {
    theta = theta - angulo;
  }
  else
  {
    theta = theta + angulo;
  }

  if (vel_lineal != 0)
  {
    float matriz_conocer_posicion[3][2]={{cos(theta), 0},{sin(theta),0}, {0,1}};// o es theta
    float movX = matriz_conocer_posicion[0][0] * vel_lineal ;
    float movY = matriz_conocer_posicion[1][0] * vel_lineal ;
    //Serial.println("movX: " + String(movX) + " movY: " + String(movY));

    if (myCoche.vaReversa)
    {
      x = x - movX;
      y = y - movY;
      x2 = x - movX;
      y2 = y - movY;
    }
    else
    {
      x = x + movX;
      y = y + movY;
      x2 = x + movX;
      y2 = y + movY;
    }
  }
  //Mandan nuevas ubicaciones
  client.publish("/comandos", "x="+String(x)+",y="+String(y)+ ",theta="+String(theta) + ",x2="+String(x2)+",y2="+String(y2)+";");
}

void setup() {
  client.begin("192.168.244.81", net); // poner direccion ip uni 192.168.48.221, casa :192.168.0.14. 
  Serial.begin(115200);
  pinMode(LedPin, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT); 
  
  // start wifi and mqtt
  WiFi.begin(ssid, pass); 
  client.onMessage(messageReceived);

  connect();
  Serial.println("empieza...");
}

void delantePulso()
{
  tiempoInicio = micros();
  int pu = 0;
  int distanciaRecorrida = 0;
  // 12 cm - 1601 
  while (pu < 801) {
    myCoche.avanzaVuelta();
    pu++;
  } 
  distanciaRecorrida = pu * perimetroRueda / PULSOSRUEDACOMPLETA;
  Serial.print("Distancia: ");
  Serial.println(String(distanciaRecorrida));
  // Medir el tiempo transcurrido
  tiempoTranscurrido = micros() - tiempoInicio;

  // 1M micros = 1 s
  // x miros


  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos =  tiempoTranscurrido/ 1000000.0;
  velocidadMotores = distanciaRecorrida / velocidadSegundos;
  //Tiempo
  Serial.print("Tiempo: ");
  Serial.println(velocidadSegundos);

  // Mostrar la velocidad en el puerto serie
  Serial.print("Velocidad de los motores actual: ");
  Serial.println(velocidadMotores);

  actualizarPosicion(velocidadMotores, 0); // o es theta?

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
  // Diametro de circunferencia = pi * d
  // Medir el tiempo transcurrido
  tiempoTranscurrido = micros() - tiempoInicio;

  // arco = angulo * radio
  // 22.5 porque es lo que tengo configurado que rote cada vez que reciba la instruccion
  float anguloRadianes = GIR*3.1416/180;
  float distanciaRecorrida = RADIORUEDA*anguloRadianes;

  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos = tiempoTranscurrido/ 1000000.0;

  velocidadMotores = anguloRadianes/velocidadSegundos;
  actualizarPosicion(velocidadMotores,anguloRadianes); 
   // Reiniciar el tiempo para la siguiente medición
  tiempoInicio = micros();
}

void giroIzquierdoAtras()
{
  int q = 0;
  tiempoInicio = micros();
  while (q < 400) {
    myCoche.vueltaIzquierdaAtras();
    q++;
  }
  tiempoTranscurrido = micros() - tiempoInicio;
  float anguloRadianes = GIR*PI/180;
  float distanciaRecorrida = RADIORUEDA*anguloRadianes;
  velocidadSegundos = tiempoTranscurrido/ 1000000.0;
  velocidadMotores = anguloRadianes/velocidadSegundos;
  actualizarPosicion(velocidadMotores,anguloRadianes);
}

void giroDerecho()
{
  tiempoInicio = micros();
  int q = 0;
  while (q < 400) {
    myCoche.vueltaDerecha();
    q++;
  }
  tiempoTranscurrido = micros() - tiempoInicio;
  float anguloRadianes = GIR*PI/180;
  float distanciaRecorrida = RADIORUEDA*anguloRadianes;
  velocidadSegundos = tiempoTranscurrido/ 1000000.0;
  velocidadMotores = anguloRadianes/velocidadSegundos;
  actualizarPosicion(velocidadMotores,anguloRadianes);
}

void giroDerechoAtras()
{
  tiempoInicio = micros();
  int q = 0;
  while (q < 400) {
    myCoche.vueltaDerechaAtras();
    q++;
  }
  tiempoTranscurrido = micros() - tiempoInicio;
  float anguloRadianes = GIR*PI/180;
  float distanciaRecorrida = RADIORUEDA*anguloRadianes;
  velocidadSegundos = tiempoTranscurrido/ 1000000.0;
  velocidadMotores = anguloRadianes/velocidadSegundos;
  actualizarPosicion(velocidadMotores,anguloRadianes);
}

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
    int cm = ping(TriggerPin, EchoPin,500);
    delay(1000);
    // publish a message roughly every second.
    if (millis() - lastMillis > 5000) {
      lastMillis = millis();
      client.publish("/comandos", "D=" + String(cm) + ";");
    }
  
}

void accionARealizar(String comando){
    if (mensajeRecibido)
    {
      if (comando.equals("avanzar")) {
        Serial.println("avanzando!!!!");
        delantePulso();  
      } else if (comando.equals("der")) {
        if (digitalRead(DIR1) == 0 )
          giroDerecho();
        else 
          giroDerechoAtras();
      } else if (comando.equals("izq")) {
        if (digitalRead(DIR1) == 0 )
          giroIzquierdo();
        else 
          giroIzquierdoAtras();
      } else if (comando.equals("delante")) {
        myCoche.delante(1);
        myCoche.delante(2);
        myCoche.vaReversa = false;
      } else if (comando.equals("atras")) {
        myCoche.reversa(1);
        myCoche.reversa(2);
        myCoche.vaReversa = true;
        Serial.println("interrupcion despues ");

      } else if (comando.equals("subirVelocidad")) {
        myCoche.setVelocidad(myCoche.aumentoVelocidad());
        Serial.println("Nueva velocidad " + String(myCoche.getVelocidad()));
      } else if (comando.equals("bajarVelocidad")) {
        myCoche.setVelocidad(myCoche.reduccionVelocidad());
        Serial.println("Nueva velocidad " + String(myCoche.getVelocidad()));
      } else if (comando.equals("detener")) {
        int velocidadActual = myCoche.getVelocidad();
        myCoche.detenerMotores();  
      } else {
        // Manejar el caso por defecto (comando no reconocido)
        //Serial.println("Comando no reconocido");
      } 
      mensajeRecibido = false; 
    }
}

void messageReceived(String &topic, String &payload) {
  //Serial.println(topic + ": " + payload);
  mensajeRecibido = true;
  //Serial.println("qwerty: " + String(mensajeRecibido));
  comando = payload;
  accionARealizar(payload);
}
