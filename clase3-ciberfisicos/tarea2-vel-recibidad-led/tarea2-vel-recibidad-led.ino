/*
Hacer parpader un LED, definiendo los tiempos con los sig. comandos L=xxxx;, H=xxxxx;
dar confirmacion
*/
#include <string.h>
#include <stdio.h>

#define LED 2
string opcion;
char []o2;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  Serial.begin(115200);  // Es la velocidad que ha visto con la que funciona correctamente
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    opcion = Serial.readString();
    opcion.trim();
    o2 = strtok(opcion, ";");

    Serial.println(opcion);

    for (int i=0;i< sizeof(o2); i++)
      Serial.println(o2[i]);

    Serial.println("FIN");
    /*
    switch (opcion) {
      case 'a':
        digitalWrite(LED,HIGH);
        Serial.println("LED -> ON");
        break;
      case 'b':
        digitalWrite(LED, LOW);
        Serial.println("LED -> OFF");
        break;
      default:
        Serial.println("Comando no reconocido");
        break;
    }
    */
  }
}
