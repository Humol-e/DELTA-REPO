// test_motores.ino
#include "componentes.h"

#define PWMB_PIN 0
#define BIN2_PIN 1
#define BIN1_PIN 2
#define AIN1_PIN 5
#define AIN2_PIN 6
#define PWMA_PIN 7

MegaSumo motor(PWMA_PIN, AIN1_PIN, AIN2_PIN, PWMB_PIN, BIN1_PIN, BIN2_PIN);

void setup() {
  Serial.begin(115200);
  Serial.println("=== TEST MOTORES ===");
  Serial.println("Comandos: [a] avanzar  [r] retroceder  [s] stop");
}

void loop() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  switch (cmd) {
    case 'a':
      motor.avanzar(90, 90);
      Serial.println(">> Avanzando (PWM=180)");
      break;
    case 'r':
      motor.retroceder(180, 180);
      Serial.println(">> Retrocediendo (PWM=180)");
      break;
    case 's':
      motor.detener();
      Serial.println(">> Stop");
      break;
    default:
      break;
  }
}