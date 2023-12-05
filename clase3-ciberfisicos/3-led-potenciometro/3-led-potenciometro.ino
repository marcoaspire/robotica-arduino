#define LED 2
#define POT 36

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
}

unsigned int tiempo;

void loop() {
  // put your main code here, to run repeatedly:
  
  tiempo = analogRead(POT);
  digitalWrite(LED,HIGH);
  delay(tiempo);
  digitalWrite(LED,LOW);
  delay(tiempo);
}

