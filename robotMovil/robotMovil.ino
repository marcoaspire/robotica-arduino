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
#define GIR 22.5
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


int x = 0;
int x2 ;
int y = 0;
int y2;
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

// void multiplicarMatriz(float matriz [][2]){
//   int res [][2];
//   for (int i=0; i<matriz.length(); i++)
//     for (int j=0; j<matriz[0].length();j++)
//       res[i][j]= matriz[i][j]  ; 

// }

void actualizarPosicion(float velocidadM, float angulo){
  Serial.println("Angulo r: " + String(angulo));

  int vel_angular = 0;

  Serial.println("Velocidad motor: " + String(velocidadM));
  Serial.println("M{0}{0}: " + String(matriz_transformacion[0][0]));

  Serial.println("Matriz_transformacion: " + String(matriz_transformacion[0][0]) + "," + String(matriz_transformacion[0][1]) +  "," +  String(matriz_transformacion[1][0])+ ","+ String(matriz_transformacion[1][1]));

  // Si la velocidad de ambos motores siempre es la misma, optimizar codigo
  int vel_lineal = matriz_transformacion[0][0] *velocidadM + matriz_transformacion[0][1] * velocidadM;
  Serial.println("Velocidad vel_lineal: " + String(vel_lineal));

  /* 
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
  */
  // TODO: Actualiza x y , multiplicando por la matriz que falta M*{{vlineal}{w velocidad angular}}
  
  if (myCoche.vaIzquierda)
  {
    // theta = theta - (matriz_conocer_posicion[2][0] * vel_lineal + matriz_conocer_posicion[2][1] * vel_angular);
     theta = theta - angulo;
  }
  else
  {
    // theta = theta + (matriz_conocer_posicion[2][0] * vel_lineal + matriz_conocer_posicion[2][1] * vel_angular);
    theta = theta + angulo;
  }
  float matriz_conocer_posicion[3][2]={{cos(theta), 0},{sin(theta),0}, {0,1}};// o es theta
  int movX = matriz_conocer_posicion[0][0] * vel_lineal + matriz_conocer_posicion[0][1] * vel_angular;
  int movY = matriz_conocer_posicion[1][0] * vel_lineal + matriz_conocer_posicion[1][1] * vel_angular;

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

  // Las sig 2 lineas no tienen sentido 
  // x += vel_lineal * cos(theta);
  // y += vel_lineal * sin(theta);

  // Dibujar lineas con la trayectoria,, limitar, si se sale del espacio de la pantalla que se detenga, si encuentra algo lo esquiva. 

  //Actualizar bien theta
  //theta += vel_angular;
  Serial.print("theta ");
  Serial.print(theta);
  Serial.println("x: " + String(x) + " y: " + String(y));
  Serial.println("PosiblesX2: " + String(x2) + " y2: " + String(y2));

  //Mandan nuevas ubicaciones
  client.publish("/comandos", "x="+String(x)+",y="+String(y)+ ",theta="+String(theta) + ",x2="+String(x2)+",y2="+String(y2)+";");
}

void setup() {
  client.begin("192.168.244.81", net); // poner direccion ip uni 192.168.48.221, casa :192.168.0.14. 
  Serial.begin(115200);
  pinMode(LedPin, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT); 
  /*
  thread1->onRun(loop2);
	thread1->setInterval(500);

  thread2.onRun(accionARealizar);
  */
  // start wifi and mqtt
  WiFi.begin(ssid, pass); 
  client.onMessage(messageReceived);

  connect();
  Serial.println("empieza...");
  
  // Serial.println(miObjeto.miMetodo()); 
  // miObjeto.ping(1,1); 
  // Serial.println("M{0}{0}: " + String(matriz_transformacion[0][0]));
  Serial.println(matriz_transformacion[1][1]);
  // controll.add(thread1);
  // controll.add(&thread2);

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
  int distanciaRecorrida = 0;
  // 12 cm - 1601 
  while (pu < 801) {
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
  float anguloRadianes = GIR*3.1416/180;
  // radio es 4
  float distanciaRecorrida = RADIORUEDA*anguloRadianes;
  Serial.print("Distancia recorrida vuelta izq cm: ");
  Serial.println(distanciaRecorrida);

  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos = tiempoTranscurrido/ 1000000.0;
  Serial.print("velocidad en seg: ");
  Serial.println(String(velocidadSegundos));

  velocidadMotores = distanciaRecorrida/velocidadSegundos;
  
  Serial.print("Velocidad angular: ");

  Serial.println(String(anguloRadianes/velocidadSegundos));

  Serial.print("Angulo ");
  Serial.println(String(anguloRadianes));


  actualizarPosicion(velocidadMotores,anguloRadianes); 
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
  float anguloRadianes = GIR*PI/180;
  float distanciaRecorrida = RADIORUEDA*anguloRadianes;
  Serial.print("Distancia recorrida vuelta izq cm: ");
  Serial.println(distanciaRecorrida);

  // Calcular la velocidad en PPS, 1,000,000 (microsegundos en un segundo)
  velocidadSegundos =  tiempoTranscurrido/ 1000000.0;
  velocidadMotores = distanciaRecorrida/velocidadSegundos;

  Serial.print("Angulo ");
  Serial.println(String(anguloRadianes));


  actualizarPosicion(velocidadMotores,anguloRadianes);
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
void loo2() {
  // El bucle principal no hace nada aquí
  // if(thread1.shouldRun())
	// 	thread1.run();
  // if(thread2.shouldRun())
	// 	thread2.run();
  // run ThreadController
	// this will check every thread inside ThreadController,
	// if it should run. If yes, he will run it;
	//controll.run();
}

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
    //Serial.println("Hilo 1 ");

    // if(!obstaculoDetectado)
    // {
    //   delantePulso();
    // }

    int cm = ping(TriggerPin, EchoPin);
    // if (cm <= limiteDistancia)
    // {
    //  obstaculoDetectado = true; 
    // }
    delay(1000);
    // publish a message roughly every second.
    if (millis() - lastMillis > 5000) {
      lastMillis = millis();
      client.publish("/comandos", "D=" + String(cm) + ";");
    }
  
}

void accionARealizar(String comando){
  
    //Serial.println("qq " + String(mensajeRecibido));
    if (mensajeRecibido)
    {
      //Serial.println("Entramos " + comando);
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
        if (velocidadActual < 2500){
          frenado(velocidadActual);
        }
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
