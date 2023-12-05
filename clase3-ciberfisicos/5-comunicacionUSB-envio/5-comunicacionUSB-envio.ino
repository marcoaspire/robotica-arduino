int contador= 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Es la velocidad que ha visto con la que funciona correctamente
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Valor del contador:");
  Serial.println(contador);
  contador++;
  delay(200);
}
