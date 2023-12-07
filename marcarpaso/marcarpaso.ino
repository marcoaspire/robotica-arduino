#define STP2 27
#define STP1 33
#define EN 25
#define DIR1 32
#define DIR2 26



// Processing andar velocidad, delante y atras, y la velocidad, negativa para atras
// processing manda velocidad, mientras se pulse avanza, pensar como hacerlo
//
int i= 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(STP1, OUTPUT);
  pinMode(STP2, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  digitalWrite(EN, LOW);
  digitalWrite(DIR1, HIGH);  // atras
  Serial.begin(115200); 
}
void loop() {
  while (i < 3200) {
    digitalWrite( STP2, HIGH );
    digitalWrite( STP2, LOW );
    delay(10);
    i++ ;
  } 
}