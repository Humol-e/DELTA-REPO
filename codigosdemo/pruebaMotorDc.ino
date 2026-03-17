//este codigo utiliza la librería esp32 de pablopeza: https://github.com/pablopeza/TB6612FNG_ESP32 , instalalo y  agrega e .zip en sketch :)
#include <TB6612_ESP32.h>

// Pines para el Motor A
#define AIN1 23
#define AIN2 22
#define PWMA 18  // Pin controlador de velocidad

#define STBY 4

// Creamos solo el objeto del Motor 1
// Usamos los 8 parámetros
Motor motor1 = Motor(AIN1, AIN2, PWMA, 1, STBY, 5000, 8, 0);

void setup() {
  Serial.begin(115200);

  // Configuración manual del pin de velocidad (PWMA)
  // Como lo conectaste al 18, necesitamos decirle al ESP32 que mande energía por ahí
  pinMode(PWMA, OUTPUT);
  analogWrite(PWMA, 50);  // Este valor de 0-255 dictamina la velocidad maxima del motor

  Serial.println("Probando SOLO Motor A...");
  Serial.println("PWMA conectado al pin 18");
  delay(2000);
}

void loop() {
  Serial.println("Motor A adelante...");
  motor1.drive(150);  // Velocidad media
  delay(2000);

  Serial.println("Motor A atrás...");
  motor1.drive(-150);  // Reversa
  delay(2000);

  Serial.println("Frenando...");
  motor1.brake();
  delay(2000);
}
