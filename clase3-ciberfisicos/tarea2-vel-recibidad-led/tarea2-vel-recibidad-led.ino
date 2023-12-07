/*
Hacer parpader un LED, definiendo los tiempos con los sig. comandos L=xxxx;, H=xxxxx;
dar confirmacion, Terminada
*/
#include <string.h>
#include <stdio.h>

#define LED 2
String opcion;
String tiempoFuncionando;
int tiempoEncendido = 1000;
int tiempoApagado = 1000;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  Serial.begin(115200);  // Es la velocidad que ha visto con la que funciona correctamente
}

void loop() {
  digitalWrite(LED,HIGH);
  delay(tiempoEncendido);
  digitalWrite(LED, LOW);
  delay(tiempoApagado);

  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    opcion = Serial.readString();
    opcion.trim();
    if ((opcion.startsWith("L=") || opcion.startsWith("H=")) && opcion.endsWith(";")) {
      tiempoFuncionando = opcion.substring(2, opcion.length() - 1);
      if (tiempoFuncionando.length() <= 4) 
      {
        if (opcion.startsWith("L=")) {
          tiempoApagado = tiempoFuncionando.toInt();
          Serial.println("Nuevo tiempo apagado:" + (String)tiempoApagado);

        } else if (opcion.startsWith("H=")) {
          tiempoEncendido = tiempoFuncionando.toInt();  
          Serial.println("Nuevo tiempo encendido:" + (String)tiempoEncendido);
        }
      }
      else{
          Serial.println("Tiempo maximo 9999 milesegundos");
      }
    }
    else{
      Serial.println("Comando invalido, el comando debe seguir esta estructura H=4000; o L=2000;");
    }
  }
}
