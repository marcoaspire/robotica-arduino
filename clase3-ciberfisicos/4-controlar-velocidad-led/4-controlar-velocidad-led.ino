#define LED 2
#define PUL1 35
#define PUL2 34

int tiempo = 1000;
int tiempoAnt = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(PUL2, INPUT);
  pinMode(PUL1, INPUT);
  tiempoAnt = millis();
}


void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(PUL1)) tiempo += 100;
  if (digitalRead(PUL2)) tiempo -= 100;

  if (tiempo>2000) tiempo=2000;
  if (tiempo<0) tiempo=100;

  if ( (millis()-tiempoAnt) > tiempo)
  {
    digitalWrite(LED,!digitalRead(LED));
    tiempoAnt= millis();
  }
  
  delay(100);
}

