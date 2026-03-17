/*

Versión adaptada para utilizar la clase guerramot
y la librería Bluepad32 para comunicación con el mando.
*/

// Librerías
#include <Bluepad32.h>
#include "componentes.h"
#include <ESP32Servo.h>

Servo esc;

const int escPin = 4;
// Pines del puente H (ajusta según tu cableado)
#define PWMB_PIN 0
#define BIN2_PIN 1
#define BIN1_PIN 2
#define AIN1_PIN 5
#define AIN2_PIN 6
#define PWMA_PIN 7

// DPAD — valores correctos de Bluepad32
#define DPAD_UP 0x01
#define DPAD_DOWN 0x02
#define DPAD_RIGHT 0x08
#define DPAD_LEFT 0x04

const int ESC_MIN = 1000;
const int ESC_MAX = 2000;

int velocidad = ESC_MIN;

MegaSumo motor(PWMA_PIN, AIN1_PIN, AIN2_PIN, PWMB_PIN, BIN1_PIN, BIN2_PIN);
GamepadPtr myGamepad = nullptr;

// ---------------- Variables ----------------
int velocidadBase = 120;
int velocidadMax = 160;
int velocidadMin = 40;
bool turboActivo = false;
unsigned long tiempoTurbo = 0;

// ---------------- Botones ----------------
#define BOTON_A (1 << 0)
#define BOTON_X (1 << 1)
#define BOTON_B (1 << 2)
#define BOTON_Y (1 << 3)
#define LB (1 << 4)
#define RB (1 << 5)
#define TRIGGER_L_DIGITAL (1 << 10)
#define TRIGGER_R_DIGITAL (1 << 11)

// ---------------- Callbacks ----------------
void onGamepadConnected(GamepadPtr gp) {
  Serial.println("Control conectado.");
  myGamepad = gp;
}

void onGamepadDisconnected(GamepadPtr gp) {
  Serial.println("Control desconectado.");
  myGamepad = nullptr;
}

void setup() {
  setCpuFrequencyMhz(80); 

  Serial.begin(115200);
  delay(2000);
  Serial.println("PASO 1 - Serial OK");

  ESP32PWM::allocateTimer(0);
  esc.setPeriodHertz(50);
  esc.attach(escPin, ESC_MIN, ESC_MAX);
  esc.writeMicroseconds(ESC_MIN);
  Serial.println("PASO 2 - ESC OK");

  BP32.setup(&onGamepadConnected, &onGamepadDisconnected);
}

// ---------------- Modo Turbo ----------------
void activarTurbo() {
  turboActivo = true;
  tiempoTurbo = millis();
  Serial.println("TURBO ACTIVADO (3s)");
}

// ---------------- Bucle principal ----------------
void loop() {
BP32.update();

  if (myGamepad && myGamepad->isConnected()) {
    int ejeX = myGamepad->axisX();
    uint16_t botones = myGamepad->buttons();
    int triggerL = myGamepad->brake();
    int triggerR = myGamepad->throttle();
    uint8_t dpad = myGamepad->dpad();

    bool triggerLPressed = (botones & TRIGGER_L_DIGITAL) || triggerL > 50;
    bool triggerRPressed = (botones & TRIGGER_R_DIGITAL) || triggerR > 50;

    // ---- Turbo con RB ----
    if ((botones & RB) && !turboActivo) activarTurbo();
    if (botones & LB) {
      velocidad = ESC_MIN;
      esc.writeMicroseconds(velocidad);
      Serial.println("Brushless OFF");
    };

    if (turboActivo && millis() - tiempoTurbo >= 3000) {
      turboActivo = false;
      Serial.println("Turbo desactivado");
    }

    int velocidadActual = turboActivo ? 200 : velocidadBase;

    // ---- Velocidad base con DPAD ----
    if (dpad & DPAD_UP) {
      velocidadBase += 20;
      if (velocidadBase > velocidadMax) velocidadBase = velocidadMax;
      Serial.printf("Velocidad aumentada: %d\n", velocidadBase);
      delay(75);
    }
    if (dpad & DPAD_DOWN) {
      velocidadBase -= 20;
      if (velocidadBase < velocidadMin) velocidadBase = velocidadMin;
      Serial.printf("Velocidad disminuida: %d\n", velocidadBase);
      delay(75);
    }

    // ---- Movimiento ----
    if (triggerLPressed && triggerRPressed) {
      motor.frenado(200);
    } else if (triggerRPressed) {
      int vIzq = map(triggerR, 0, 255, velocidadMin, velocidadActual);
      int vDer = vIzq;
      if (ejeX < -50) vDer = map(abs(ejeX), 50, 512, vIzq, velocidadMin);
      else if (ejeX > 50) vIzq = map(abs(ejeX), 50, 512, vIzq, velocidadMin);
      motor.avanzar(vIzq, vDer);
    } else if (triggerLPressed) {
      int vIzq = map(triggerL, 0, 255, velocidadMin, velocidadActual);
      int vDer = vIzq;
      if (ejeX < -50) vDer = map(abs(ejeX), 50, 512, vIzq, velocidadMin);
      else if (ejeX > 50) vIzq = map(abs(ejeX), 50, 512, vIzq, velocidadMin);
      motor.retroceder(vIzq, vDer);
    } else if (abs(ejeX) > 50) {
      int vGiro = map(abs(ejeX), 50, 512, velocidadMin, velocidadActual);
      if (ejeX < -50) motor.izquierda(vGiro, vGiro);
      else motor.derecha(vGiro, vGiro);
    } else {
      motor.detener();
    }

    // ---- Brushless con Y/B/A/X ----
    if (botones & BOTON_Y) {
      velocidad = 1500;
      esc.writeMicroseconds(velocidad);
      Serial.println("Brushless 50%");
    } else if (botones & BOTON_B) {
      velocidad = 1750;
      esc.writeMicroseconds(velocidad);
      Serial.println("Brushless 75%");
    } else if (botones & BOTON_A) {
      velocidad = ESC_MAX;
      esc.writeMicroseconds(velocidad);
      Serial.println("Brushless 100%");
    } else if (botones & BOTON_X) {
      velocidad = ESC_MIN;
      esc.writeMicroseconds(velocidad);
      Serial.println("Brushless OFF");
    }
  }
if (!myGamepad || !myGamepad->isConnected()) {
  velocidad = ESC_MIN;
  esc.writeMicroseconds(velocidad);
  Serial.println("Brushless OFF - control desconectado");
}
  delay(10);
}