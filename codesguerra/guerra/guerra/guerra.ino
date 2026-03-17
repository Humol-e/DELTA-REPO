/* Marco Antonio Guereca Soto
Robot Sumo Mark 2 "Ashura"

Versión adaptada para utilizar la clase MegaSumo
y la librería Bluepad32 para comunicación con el mando.
*/

// Librerías
#include <Bluepad32.h>
#include "componentes.h"

// Pines del puente H (ajusta según tu cableado)
#define M1R 13
#define M1L 12
#define M2R 18
#define M2L 19  

MegaSumo motor(M1R, M1L, M2R, M2L);
GamepadPtr myGamepad = nullptr;

// ---------------- Variables ----------------
int velocidadBase = 120;
int velocidadMax = 160;
int velocidadMin = 40;
bool turboActivo = false;
unsigned long tiempoTurbo = 0;

// ---------------- Botones ----------------
#define BOTON_A (1 << 0)
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

// ---------------- Configuración ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Bluepad32...");
  BP32.setup(&onGamepadConnected, &onGamepadDisconnected);
  BP32.forgetBluetoothKeys();
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

    bool triggerLPressed = (botones & TRIGGER_L_DIGITAL) || triggerL > 50;
    bool triggerRPressed = (botones & TRIGGER_R_DIGITAL) || triggerR > 50;

    // ---- Activar Turbo con RB ----
    if ((botones & RB) && !turboActivo) {
      activarTurbo();
      Serial.println("Turbo activado");
    }

    // ---- Desactivar turbo tras 3 s ----
    if (turboActivo && millis() - tiempoTurbo >= 3000) {
      turboActivo = false;
      Serial.println("Turbo desactivado");
    }

    // ---- Ajustar velocidad base ----
    int velocidadActual = turboActivo ? 200 : velocidadBase;

    if (botones & BOTON_Y) {
      velocidadBase += 20;
      if (velocidadBase > velocidadMax) velocidadBase = velocidadMax;
      Serial.printf("Velocidad aumentada: %d\n", velocidadBase);
      delay(200);
    }

    if (botones & BOTON_A) {
      velocidadBase -= 20;
      if (velocidadBase < velocidadMin) velocidadBase = velocidadMin;
      Serial.printf("Velocidad disminuida: %d\n", velocidadBase);
      delay(200);
    }

    // ---- Movimiento ----
    if (triggerLPressed && triggerRPressed) {
      Serial.println("Frenado activo");
      motor.frenado(200);
    }
    else if (triggerRPressed) {  // Avanzar
      int vIzq = map(triggerR, 0, 255, velocidadMin, velocidadActual);
      int vDer = vIzq;

      if (ejeX < -50) vDer = map(abs(ejeX), 50, 512, vIzq, velocidadMin);  // izquierda
      else if (ejeX > 50) vIzq = map(abs(ejeX), 50, 512, vIzq, velocidadMin);  // derecha

      motor.avanzar(vIzq, vDer);
      Serial.println("Avanzando");
    }
    else if (triggerLPressed) {  // Retroceder
      int vIzq = map(triggerL, 0, 255, velocidadMin, velocidadActual);
      int vDer = vIzq;

      if (ejeX < -50) vDer = map(abs(ejeX), 50, 512, vIzq, velocidadMin);
      else if (ejeX > 50) vIzq = map(abs(ejeX), 50, 512, vIzq, velocidadMin);

      motor.retroceder(vIzq, vDer);
      Serial.println("Retrocediendo");
    }
    else if (abs(ejeX) > 50) {  // Giro en el lugar
      int vGiro = map(abs(ejeX), 50, 512, velocidadMin, velocidadActual);
      if (ejeX < -50) {
        motor.izquierda(vGiro, vGiro);
        Serial.println("Girando a la izquierda");
      } else {
        motor.derecha(vGiro, vGiro);
        Serial.println("Girando a la derecha");
      }
    }
    else {
      motor.detener();
      Serial.println("Detenido");
    }
  }

  delay(10);
}