#include "esp_adc/adc_oneshot.h"
#include "esp_wifi.h"

// ---------------- PINES MOTORES ----------------
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
int baseSpeed = 190;   // recta a tope
int curvaSpeed = 160;  // velocidad en curvas suaves
int turnSpeed = 50;    // motor interior curva suave
int maxSpeed = 255;
float reverseRatio = 0.35;
int offsetA = 0;
int offsetB = 0;

// ---------------- SENSOR ADC ----------------
const uint8_t pines[] = { 35, 32, 33, 25, 26, 27, 14, 13 };
const uint8_t SensorCount = 8;

uint16_t sensorValues[SensorCount];
uint16_t sensorMin[SensorCount];
uint16_t sensorMax[SensorCount];
uint16_t lastPosition = 3500;
uint16_t position = 3500;

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_handle_t adc2_handle;

struct PinADC {
  adc_unit_t unit;
  adc_channel_t channel;
};

const PinADC mapeoADC[] = {
  { ADC_UNIT_1, ADC_CHANNEL_7 },  // GPIO 35
  { ADC_UNIT_1, ADC_CHANNEL_4 },  // GPIO 32
  { ADC_UNIT_1, ADC_CHANNEL_5 },  // GPIO 33
  { ADC_UNIT_2, ADC_CHANNEL_8 },  // GPIO 25
  { ADC_UNIT_2, ADC_CHANNEL_9 },  // GPIO 26
  { ADC_UNIT_2, ADC_CHANNEL_7 },  // GPIO 27
  { ADC_UNIT_2, ADC_CHANNEL_6 },  // GPIO 14
  { ADC_UNIT_2, ADC_CHANNEL_4 },  // GPIO 13
};

// ---------------- ADC SETUP ----------------
void setupADC() {
  adc_oneshot_unit_init_cfg_t cfg1 = { .unit_id = ADC_UNIT_1 };
  adc_oneshot_unit_init_cfg_t cfg2 = { .unit_id = ADC_UNIT_2 };
  adc_oneshot_new_unit(&cfg1, &adc1_handle);
  adc_oneshot_new_unit(&cfg2, &adc2_handle);

  adc_oneshot_chan_cfg_t chanCfg = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_10
  };

  for (int i = 0; i < SensorCount; i++) {
    adc_oneshot_unit_handle_t h = mapeoADC[i].unit == ADC_UNIT_1 ? adc1_handle : adc2_handle;
    adc_oneshot_config_channel(h, mapeoADC[i].channel, &chanCfg);
  }
}

int calcularVelocidadBase() {
  int error = abs((int)position - 3500);
  // En recta (error < 500) va a tope
  // En curva reduce proporcionalmente
  return map(constrain(error, 0, 3500), 0, 3500, baseSpeed, 60);
}
uint16_t leerPin(int i) {
  int raw = 0;
  adc_oneshot_unit_handle_t h = mapeoADC[i].unit == ADC_UNIT_1 ? adc1_handle : adc2_handle;
  adc_oneshot_read(h, mapeoADC[i].channel, &raw);
  return (uint16_t)raw;
}

// ---------------- CALIBRACIÓN ----------------
void calibrar() {
  for (int i = 0; i < SensorCount; i++) {
    sensorMin[i] = 1023;
    sensorMax[i] = 0;
  }

  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(2000);
  for (int j = 0; j < 200; j++) {
    for (int i = 0; i < SensorCount; i++) {
      uint16_t val = leerPin(i);
      if (val < sensorMin[i]) sensorMin[i] = val;
    }
    delay(5);
  }
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(2000);

  for (int j = 0; j < 200; j++) {
    for (int i = 0; i < SensorCount; i++) {
      uint16_t val = leerPin(i);
      if (val > sensorMax[i]) sensorMax[i] = val;
    }
    delay(5);
  }
}

// ---------------- LECTURA ----------------
uint16_t normalizar(int i, uint16_t valor) {
  if (sensorMax[i] == sensorMin[i]) return 0;
  return constrain(map(valor, sensorMin[i], sensorMax[i], 0, 1000), 0, 1000);
}

void leerPosicion() {
  long suma = 0;
  long total = 0;
  int activoCount = 0;

  for (int i = 0; i < SensorCount; i++) {
    uint16_t raw = leerPin(i);
    sensorValues[i] = normalizar(i, raw);

    if (sensorValues[i] > 600) {
      suma += (long)sensorValues[i] * i * 1000;
      total += sensorValues[i];
      activoCount++;
    }
  }

  if (activoCount > 0) {
    position = suma / total;
    lastPosition = position;
  } else {
    position = lastPosition;
  }
}


// ---------------- MOTORES ----------------

void moverMotores(int speedA, int speedB) {
  if (speedA == 0 && speedB == 0) {
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    return;
  }

  speedA += offsetA;
  speedB += offsetB;

  if (speedA >= 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, constrain(speedA, 0, maxSpeed));
  } else {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, constrain(-speedA, 0, maxSpeed));
  }

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
// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  esp_wifi_stop();
  esp_wifi_deinit();
  setCpuFrequencyMhz(240);

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, LOW); // motores apagados durante setup

  pinMode(pinEntrada, INPUT_PULLDOWN);

  setupADC();
  calibrar();

  // Estabilizar lecturas antes de arrancar
  for (int i = 0; i < 100; i++) leerPosicion();

  digitalWrite(ledPin, HIGH);
  digitalWrite(STBY, HIGH); // encender motores solo cuando todo está listo
  delay(500);
}
// ---------------- LOOP ----------------
void loop() {
  leerPosicion();

  int revSpeed = -(baseSpeed * reverseRatio);
  int velBase = calcularVelocidadBase();

  int estado = digitalRead(pinEntrada);
  if (estado == LOW) {
    moverMotores(0, 0);
    return;
  }

  int speedA, speedB;
  int error = (int)position - 3500;

  // Detección de cruce (todos ven negro = ignorar)
  int sensoresActivos = 0;
  for (int i = 0; i < SensorCount; i++) {
    if (sensorValues[i] > 600) sensoresActivos++;
  }

  if (sensoresActivos >= 7) {
    // Cruce de líneas → seguir recto
    speedA = velBase;
    speedB = velBase;

  } else if (position < 400) {
    speedA = constrain(baseSpeed * 1.4, 0, maxSpeed);
    speedB = revSpeed;

  } else if (position < 900) {
    speedA = constrain(baseSpeed * 1.3, 0, maxSpeed);
    speedB = -(revSpeed * 0.5);

  } else if (position < 1700) {
    speedA = curvaSpeed;
    speedB = 0;

  } else if (position < 2500) {
    speedA = velBase * 0.9;
    speedB = velBase * 0.15;

  } else if (position < 3275) {
    speedA = velBase;
    speedB = velBase * 0.35;

  } else if (position > 6600) {
    speedA = revSpeed;
    speedB = constrain(baseSpeed * 1.4, 0, maxSpeed);

  } else if (position > 6100) {
    speedA = -(revSpeed * 0.5);
    speedB = constrain(baseSpeed * 1.3, 0, maxSpeed);

  } else if (position > 5500) {
    speedA = 0;
    speedB = curvaSpeed;

  } else if (position > 4500) {
    speedA = velBase * 0.15;
    speedB = velBase * 0.9;

  } else if (position > 3745) {
    speedA = velBase * 0.35;
    speedB = velBase;

  } else {
    // RECTA → tope absoluto
    speedA = baseSpeed;
    speedB = baseSpeed;
  }

  moverMotores(speedA, speedB);
}