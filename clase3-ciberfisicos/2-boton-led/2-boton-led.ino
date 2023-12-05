#define LED 2
#define PUL1 35

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(PUL1, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(digitalRead(PUL1))
  {
    digitalWrite(LED,LOW); 
  }
  else{
    digitalWrite(LED,HIGH); 
  }


}

