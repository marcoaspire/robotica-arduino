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
    22 cm - necesita 3200 pulsos aka 360 
    i.e 8.888 pulsos equivale a un grado  
  */ 
  Coche(int velocidad) {
    pinMode(STP1, OUTPUT);
    pinMode(STP2, OUTPUT);
    pinMode(EN, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);
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
    while (vuetasRealizadas < vueltasNecesarias)
    {
      avanzaVuelta();
      vuetasRealizadas++;
    }
    delante(1);
  }

  void detenerMotores() {
    // Detiene ambos motores
    digitalWrite(STP1, LOW);
    digitalWrite(STP2, LOW);
  }

};
int pulsos = 0;
Coche myCoche(2);

void setup() {
  // Nothing to do in setup for this example
  Serial.begin(115200);
  Serial.println("\empieza...");

}
int q=0;
void loop() {
  // 180 grados
  while (q < 340) {
    myCoche.vueltaIzquierda();
    delay(10); // 1
    q++;
  } 
  // Necesito que distancia avanza
  while (pulsos < 3200) {
    //myCoche.avanzaVuelta();
    //myCoche.vueltaIzquierda();
    delay(10); // 1
    // delayMicroseconds(1000); //2
    // delayMicroseconds(500); //3
    
    pulsos++;
  } 
  if (pulsos == 3200)
  Serial.println("terminaando...");
  pulsos++;

}
