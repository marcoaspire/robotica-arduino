## Motores 

- alterna, 220, frecuancua para el giro 

- continua, 
con escobilla, campo magnetico debe ir cambiando la posicion 
sin escobilla

- Motor paso a paso, continua, requiere electronica avanzada, 

- grado por paso: 1.8. 200 pulsos por vuelta, par de retencion 0.57Newton metro,

A mayor frecuencia mas rapido va.

pulso cada segundo - no se va a ver

360/1.8 grados = 200 para dar una vuelta, micropasos 3200 pulsos por vuelta. 1 paso = 16 micropasos
1.8 - 200
x - 3200 -> .112 

float tiempoTotal = 3200 * RETARDO_ENTRE_PASOS / 1000.0;
  
  // Calcula la velocidad de giro
  float velocidadDeGiro = 1 / tiempoTotal;

tecnica de micropasos, multiplo de 2 


- Generar movimiento en ambos motores. adelante, atras, distancia 2pi*r, 
- desplazamiento de los motores
- generar pulsos para mover el motor
