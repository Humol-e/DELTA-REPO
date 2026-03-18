#include <QTRSensors.h>
#include "BluetoothSerial.h"

// ---------------- PINES ----------------
#define AIN1 22
#define AIN2 21
#define PWMA 23
#define BIN1 18
#define BIN2 5
#define PWMB 4
#define STBY 19

const int pinEntrada = 15;
const int ledPin = 2;

// ---------------- VELOCIDADES ----------------
int baseSpeed = 115;  //base pero la ajusto con bt a 163 (hasta ahora nuestro tope)
int turnSpeed = 40;  // velocidad del motor interior en curva
int maxSpeed = 250;
float reverseRatio = 0.25; // 25% del baseSpeed en reversa
uint16_t lastPosition = 3500;

// ---------------- QTR ----------------
QTRSensors qtr;
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

BluetoothSerial SerialBT;

int offsetA = 0;
int offsetB = 0;



void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  analogReadResolution(10);
  pinMode(pinEntrada, INPUT_PULLDOWN);

  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){ 35, 32, 33, 25, 26, 27, 14, 13 }, SensorCount);
  qtr.setTimeout(1000);
  Serial.println(">>> CALIBRANDO... <<<");
  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
    delay(10);
  }
  Serial.println(">>> LISTO <<<");
  digitalWrite(ledPin, HIGH);  // Enciende el LED

  SerialBT.begin("ESP32_Robot");
  delay(1000);
}

void loop() {
  //leerBluetooth();
  int revSpeed = -(baseSpeed * reverseRatio);
// En el loop, después de leer position:
uint16_t position = qtr.readLineBlack(sensorValues);
  int estado = digitalRead(pinEntrada);
  if (estado == LOW) {
    moverMotores(0, 0);
    return;
  }
// Variable global


int delta = abs((int)position - (int)lastPosition);

if (delta > 4000) {
  // salto enorme = probablemente falso, suaviza fuerte
  position = (lastPosition * 0.8) + (position * 0.2);
} else if (delta > 2000) {
  // cambio grande = curva cerrada, reacciona pero suaviza poco
  position = (lastPosition * 0.4) + (position * 0.6);
} else {
  // cambio normal = usa lectura directa sin filtro
  position = position;
}

lastPosition = position;
// ---- DECIDIR ACCIÓN SEGÚN POSICIÓN ----
  // 0-2999 → línea a la izquierda → girar izquierda (Motor A frena, B sigue)
  // 3000-4000 → centro → recto
  // 4001-7000 → línea a la derecha → girar derecha (Motor B frena, A sigue)

  int speedA, speedB;
    
  if (position < 500) {
    // extremo izquierdo → pivot
    speedA = constrain(baseSpeed * 1.35, 0, maxSpeed);
    speedB = revSpeed;

  } else if (position < 1000) {
    speedA = constrain(baseSpeed * 1.35, 0, maxSpeed);
    speedB = 0;

  } else if (position < 1700) {
    speedA = baseSpeed;
    speedB = 0;

  } else if (position < 2500) {
    speedA = baseSpeed * 0.75;
    speedB = baseSpeed * 0.20;

  } else if (position < 3275) {
    speedA = baseSpeed;
    speedB = turnSpeed;

  } else if (position > 6700) {
    // extremo derecho → pivot
    speedA = revSpeed;
    speedB = constrain(baseSpeed * 1.35, 0, maxSpeed);

  } else if (position > 6500) {
    speedA = 0;
    speedB = constrain(baseSpeed * 1.35, 0, maxSpeed);

  } else if (position > 5500) {
    speedA = 0;
    speedB = baseSpeed;

  } else if (position > 4500) {
    speedA = baseSpeed * 0.20;
    speedB = baseSpeed * 0.75;

  } else if (position > 3745) {
    speedA = turnSpeed;
    speedB = baseSpeed;

  } else {
    speedA = baseSpeed;
    speedB = baseSpeed;
  }
  moverMotores(speedA, speedB);
  return;
  // ---- DEBUG ----
  /*Serial.print("Pos: ");
  Serial.print(position);
  Serial.print(" | Estado: ");
  if (estadoActual == RECTO) Serial.print("RECTO     ");
  if (estadoActual == GIRANDO_IZQ) Serial.print("IZQ       ");
  if (estadoActual == GIRANDO_DER) Serial.print("DER       ");
  Serial.print(" | A: ");
  Serial.print(speedA);
  Serial.print(" | B: ");
  Serial.println(speedB);
  */
}
/*
void leerBluetooth() {
  if (SerialBT.available()) {
    char comando = SerialBT.read();
    delay(10);
    int valor = (int)SerialBT.parseFloat();
    while (SerialBT.available() > 0) { SerialBT.read(); }

    if (comando == 'V' || comando == 'v') {
      baseSpeed = valor;
      SerialBT.print("Base: ");
      SerialBT.println(baseSpeed);
    } else if (comando == 'T' || comando == 't') {
      turnSpeed = valor;
      SerialBT.print("Turn: ");
      SerialBT.println(turnSpeed);
    } else if (comando == 'A' || comando == 'a') {
      offsetA = valor;
      SerialBT.print("OffsetA: ");
      SerialBT.println(offsetA);
    } else if (comando == 'B' || comando == 'b') {
      offsetB = valor;
      SerialBT.print("OffsetB: ");
      SerialBT.println(offsetB);
    } else if (comando == 'R' || comando == 'r') {
      reverseRatio  =valor;  // siempre negativo sin importar lo que manden
      SerialBT.print("Reversa: ");
      SerialBT.println(reverseRatio );
    }
  }
  return;
}
*/
void moverMotores(int speedA, int speedB) {
  if (speedA == 0 && speedB == 0) {
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    return;
  }
  // Motor A
  if (speedA >= 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, constrain(speedA, 0, maxSpeed));
  } else {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, constrain(-speedA, 0, maxSpeed));
  }

  // Motor B
  if (speedB >= 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, constrain(speedB, 0, maxSpeed));
  } else {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, constrain(-speedB, 0, maxSpeed));
  }
}