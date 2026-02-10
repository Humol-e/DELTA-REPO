#include <Servo.h>

Servo esc;

const int escPin = 9;

// Ajusta estos valores si tu ESC lo requiere
const int ESC_MIN = 1000;
const int ESC_MAX = 2000;

int velocidad = ESC_MIN;

void setup() {
  Serial.begin(9600);
  esc.attach(escPin);

  Serial.println("===== CALIBRACION DEL ESC =====");
  Serial.println("1) Quita la helice");
  Serial.println("2) Desconecta la bateria del ESC");
  Serial.println("3) Presiona 'c' para comenzar calibracion");

  // Esperar comando
  while (!Serial.available());
  char cmd = Serial.read();

  if (cmd == 'c') {
    calibrarESC();
  }

  Serial.println("ESC listo para uso");
  Serial.println("Comandos:");
  Serial.println("a = arrancar");
  Serial.println("s = subir velocidad");
  Serial.println("d = bajar velocidad");
  Serial.println("p = paro total");
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

void calibrarESC() {
  Serial.println("Enviando MAX...");
  esc.writeMicroseconds(ESC_MAX);

  Serial.println(">>> Conecta la bateria del ESC AHORA <<<");
  delay(7000); 

  Serial.println("Enviando MIN...");
  esc.writeMicroseconds(ESC_MIN);
  delay(5000);

  Serial.println("Calibracion completada.");
}
