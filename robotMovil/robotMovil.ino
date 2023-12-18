#define STP2 27
#define STP1 33
#define EN 25
#define DIR1 32
#define DIR2 26
// Custom class definition
class Coche {
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
  Coche(int velocidad) {
    pinMode(STP1, OUTPUT);
    pinMode(STP2, OUTPUT);
    pinMode(EN, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);
    digitalWrite(DIR1, LOW);
    digitalWrite(DIR2, LOW); 
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
    delayMicroseconds(500);
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

};

int pulsos = 0;
int q=0;
Coche myCoche(2);

void setup() {
  // Nothing to do in setup for this example
  Serial.begin(115200);
  Serial.println("empieza...");
}

void giroIzquierdo()
{
  int q = 0;
  //180
  while (q < 3300) {
    myCoche.vueltaIzquierdaAtras();
    delay(10); // 1
    q++;
  }
}

void delantePulso()
{
  int pu = 0;
  // 12 cm
  while (pu < 1601) {
    myCoche.avanzaVuelta();
    //myCoche.vueltaIzquierda();
    delay(10); // 1
    // delayMicroseconds(1000); //2
    // delayMicroseconds(500); //3
    pulsos++;
  } 
}

void loop() {
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
  //   delay(10); // 1
  //   // delayMicroseconds(1000); //2
  //   // delayMicroseconds(500); //3
    
  //   pulsos++;
  // } 
  if (pulsos == 3200)
  Serial.println("terminaando...");
  pulsos++;

}
