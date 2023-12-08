#define STP2 27
#define STP1 33
#define EN 25
#define DIR1 32
#define DIR2 26
// Custom class definition
class Coche {
public:
  // Constructor
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
    digitalWrite( STP2, LOW );
    digitalWrite( STP1, HIGH );
    digitalWrite( STP1, LOW );
    delay(10);
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
  
Coche myCoche(2);
int pulsos = 0;

void setup() {
  // Nothing to do in setup for this example
}

void loop() {
  // Necesito que distancia avanza
  while (pulsos < 3200) {
    myCoche.avanzaVuelta();
    delay(10);
    pulsos++;
  } 
}
