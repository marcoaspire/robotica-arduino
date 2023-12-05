/*
Hacer parpader un LED, definiendo los tiempos con los sig. comandos L=xxxx;, H=xxxxx;
Si se recibe L=xxxx; indicará cuanto tiempo durara apagado
Si se recibe H=xxxx; indicará cuanto tiempo durara encendido
MAximo tiempo 9 segundos
*/
#define LED 2

String opcion;
String parteVelocidad;
int value =0;
int tiempoEncendido=1000;
int tiempoApagado=1000;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  Serial.begin(115200);  // Es la velocidad que ha visto con la que funciona correctamente
}

void loop() {
  digitalWrite(LED,HIGH); 
  delay(tiempoEncendido);
  digitalWrite(LED,LOW);
  delay(tiempoApagado);
  if (Serial.available()) {
    opcion = Serial.readString();
    opcion.trim();
    Serial.println("Comando recibido:" + opcion);
    if (opcion.endsWith(";"))
    {
      if (opcion.startsWith("L=") || opcion.startsWith("H=") )
      {
        parteVelocidad = opcion.substring(2, opcion.length()-1);        
        if (parteVelocidad.length() <=4) 
        {
          value = parteVelocidad.toInt();
          
          Serial.println("Velocidad nueva");
          Serial.println("zaqw="+String(value));
        }
        else 
          Serial.println("Error7"); 
        if (opcion.startsWith("L=")){
          Serial.println("nuevo apagado " + String(value));
          tiempoApagado = value;
        }
        else if (opcion.startsWith("H=")){
          tiempoEncendido = value;
          Serial.println("nuevo encendido " + String(value));
        }
        else{
          Serial.println("Error"); 
        }
      }
      else{
        Serial.println("Error2"); 
      }
    }
    else
      Serial.println("Error"); 
    Serial.println("FIN");    
  }
}