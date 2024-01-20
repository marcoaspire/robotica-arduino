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

#include <WiFi.h>
#include <MQTT.h>
#include <AccelStepper.h>

#include "Clase.h"
Clase miObjeto; 


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
float matriz_transformacion[2][2]={{1.0/2.0,1.0/2.0},{-b/2.0,b/2.0}};

unsigned long tiempoTranscurrido;
unsigned long lastMillis = 0;



int x = 0;
int y = 0;
int theta = 0;

AccelStepper motorIzquierdo(AccelStepper::DRIVER, STP1);
AccelStepper motorDerecho(AccelStepper::DRIVER, STP2);
// int velocidad = 2500;

int pulsos = 0;
int q=0;

// Custom class definition
class Coche {
private:
	int velocidad;
public:
  bool vaReversa;
  bool vaIzquierda;  
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
    vaIzquierda = true;

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

// void multiplicarMatriz(float matriz [][2]){
//   int res [][2];
//   for (int i=0; i<matriz.length(); i++)
//     for (int j=0; j<matriz[0].length();j++)
//       res[i][j]= matriz[i][j]  ; 

// }

void actualizarPosicion(float velocidadM, float angulo, bool estaGirando){
  int vel_angular = 0;
  float matriz_conocer_posicion[3][2]={{cos(angulo), 0},{sin(angulo),0}, {0,1}};// o es theta

  Serial.println("Velocidad motor: " + String(velocidadM));
  Serial.println("M{0}{0}: " + String(matriz_transformacion[0][0]));

  Serial.println("Matriz_transformacion: " + String(matriz_transformacion[0][0]) + "," + String(matriz_transformacion[0][1]) +  "," +  String(matriz_transformacion[1][0])+ ","+ String(matriz_transformacion[1][1]));

  // Si la velocidad de ambos motores siempre es la misma, optimizar codigo
  int vel_lineal = matriz_transformacion[0][0] *velocidadM + matriz_transformacion[0][1] * velocidadM;
  Serial.println("Velocidad vel_lineal: " + String(vel_lineal));

  if (estaGirando)
  {
    vel_angular = matriz_transformacion[1][0]*velocidadM + matriz_transformacion[1][1] * velocidadM * (-1);
  }
  else
  {
    vel_angular = matriz_transformacion[1][0]*velocidadM + matriz_transformacion[1][1] * velocidadM;
  } 
  Serial.println("Velocidad vel_angular: " + String(vel_angular));


  Serial.print("Velocidad lineal actual: ");
  Serial.println(vel_lineal);
  Serial.print("Velocidad angular actual: ");
  Serial.println(vel_angular);
  // TODO: Actualiza x y , multiplicando por la matriz que falta M*{{vlineal}{w velocidad angular}}
  if (myCoche.vaIzquierda)
  {
    theta = theta - (matriz_conocer_posicion[2][0] * vel_lineal + matriz_conocer_posicion[2][1] * vel_angular);
  }
  else
  {
    theta = theta + (matriz_conocer_posicion[2][0] * vel_lineal + matriz_conocer_posicion[2][1] * vel_angular);
  }

  if (myCoche.vaReversa)
  {
    x = x - (matriz_conocer_posicion[0][0] * vel_lineal + matriz_conocer_posicion[0][1] * vel_angular);
    y = y - (matriz_conocer_posicion[1][0] * vel_lineal + matriz_conocer_posicion[1][1] * vel_angular);
  }
  else
  {
    x = x + (matriz_conocer_posicion[0][0] * vel_lineal + matriz_conocer_posicion[0][1] * vel_angular);
    y = y + (matriz_conocer_posicion[1][0] * vel_lineal + matriz_conocer_posicion[1][1] * vel_angular);
  }

  // Las sig 2 lineas no tienen sentido 
  // x += vel_lineal * cos(theta);
  // y += vel_lineal * sin(theta);

  // Dibujar lineas con la trayectoria,, limitar, si se sale del espacio de la pantalla que se detenga, si encuentra algo lo esquiva. 

  //Actualizar bien theta
  //theta += vel_angular;
  Serial.print("theta ");
  Serial.print(theta);
  Serial.print("x: " + String(x) + " y: " + String(y));
  //Mandan nuevas ubicaciones
  client.publish("/comandos", "x="+String(x)+",y="+String(y)+ ",theta="+String(theta) + ";");
}

void setup() {
  Serial.begin(115200);
  pinMode(LedPin, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);  
  // start wifi and mqtt
  WiFi.begin(ssid, pass); 
  client.begin("192.168.0.14", net); // poner direccion ip uni 192.168.48.221, casa :192.168.0.14. 
  client.onMessage(messageReceived);

  connect();
  Serial.println("empieza...");
  
  // Serial.println(miObjeto.miMetodo()); 
  // miObjeto.ping(1,1); 
  // Serial.println("M{0}{0}: " + String(matriz_transformacion[0][0]));
  Serial.println(matriz_transformacion[1][1]);
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
  velocidadMotores = pu / velocidadSegundos;
  //Tiempo
  Serial.print("Tiempo: ");
  Serial.println(velocidadSegundos);

  // Mostrar la velocidad en el puerto serie
  Serial.print("Velocidad de los motores actual: ");
  Serial.println(velocidadMotores);

  actualizarPosicion(velocidadMotores, 0,false); // o es theta?

  // velocidadLineal = 12.065/ velocidadSegundos;  // cm por seg, esta es velocidad lineal de los motores
  // Serial.print("Velocidad lineal actual: ");
  // Serial.println(velocidadLineal);


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

  //Conociendo el angulo, la trayectoria = radio por angulo en radianes
  // 180 = pi radianes
  // arco = angulo * radio
  // 22.5 porque es lo que tengo configurado que rote cada vez que reciba la instruccion
  float anguloRadianes = 22.5*3.1416/180;
  // radio es 4
  float distanciaRecorrida = 4*anguloRadianes;
  Serial.print("Distancia recorrida vuelta izq cm: ");
  Serial.println(distanciaRecorrida);

  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos =  tiempoTranscurrido/ 1000000.0;
  velocidadMotores = distanciaRecorrida/velocidadSegundos;

  Serial.print("Angulo ");
  Serial.println(String(anguloRadianes));


  actualizarPosicion(velocidadMotores,anguloRadianes,true);
  // Serial.print("Velocidad lineal actual: ");
  // Serial.println(velocidadMotores);

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
  tiempoInicio = micros();
  int q = 0;
  while (q < 400) {
    myCoche.vueltaDerecha();
    q++;
  }


  // Diametro de circunferencia = pi * d
  // Medir el tiempo transcurrido
  tiempoTranscurrido = micros() - tiempoInicio;

  //Conociendo el angulo, la trayectoria = radio por angulo en radianes
  // 180 = pi radianes
  // arco = angulo * radio
  // 22.5 porque es lo que tengo configurado que rote cada vez que reciba la instruccion
  float anguloRadianes = 22.5*3.1416/180;
  // radio es 4
  float distanciaRecorrida = 4*anguloRadianes;
  Serial.print("Distancia recorrida vuelta izq cm: ");
  Serial.println(distanciaRecorrida);

  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos =  tiempoTranscurrido/ 1000000.0;
  velocidadMotores = distanciaRecorrida/velocidadSegundos;

  Serial.print("Angulo ");
  Serial.println(String(anguloRadianes));


  actualizarPosicion(velocidadMotores,anguloRadianes,true);
  // Serial.print("Velocidad lineal actual: ");
  // Serial.println(velocidadMotores);

   // Reiniciar el tiempo para la siguiente medición
  tiempoInicio = micros();

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
  int cm = ping(TriggerPin, EchoPin);
  delay(1000);
  // publish a message roughly every second.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    client.publish("/comandos", "D=" + String(cm) + ";");
  }
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
    myCoche.vaReversa = false;
  } else if (payload.equals("atras")) {
    myCoche.reversa(1);
    myCoche.reversa(2);
    myCoche.vaReversa = true;
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
