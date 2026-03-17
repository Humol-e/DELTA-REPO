// Control ESC con ESP32-C3 Supermini
// Librería requerida: ESP32Servo (instalar desde Library Manager)
#include <ESP32Servo.h>

Servo esc;

const int escPin = 4; 

const int ESC_MIN = 1000;
const int ESC_MAX = 2000;

int velocidad = ESC_MIN;

void setup() {
  Serial.begin(115200);
  delay(2000); // Da tiempo a que abras el Serial Monitor

  ESP32PWM::allocateTimer(0);
  esc.setPeriodHertz(50);
  esc.attach(escPin, ESC_MIN, ESC_MAX);
esc.writeMicroseconds(ESC_MIN); // <-- esta línea faltaba
delay(2000); // deja que el ESC inicialice viendo MIN
  Serial.println("===== CALIBRACION DEL ESC =====");
  Serial.println("Presiona 'c' para calibrar o cualquier tecla para saltar");

  // Esperar con timeout de 10 segundos
  unsigned long t = millis();
  while (!Serial.available() && millis() - t < 10000);


  Serial.println("ESC listo para uso");
  Serial.println("a=arrancar | s=subir | d=bajar | p=parar");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'a') {
      velocidad = 1200;
      esc.writeMicroseconds(velocidad);
      Serial.println("Motor arrancado");
    }

    if (cmd == 's') {
      velocidad += 50;
      if (velocidad > ESC_MAX) velocidad = ESC_MAX;
      esc.writeMicroseconds(velocidad);
      Serial.print("Velocidad: ");
      Serial.println(velocidad);
    }

    if (cmd == 'd') {
      velocidad -= 50;
      if (velocidad < ESC_MIN) velocidad = ESC_MIN;
      esc.writeMicroseconds(velocidad);
      Serial.print("Velocidad: ");
      Serial.println(velocidad);
    }

    if (cmd == 'p') {
      velocidad = ESC_MIN;
      esc.writeMicroseconds(velocidad);
      Serial.println("Motor detenido");
    }
  }
}

