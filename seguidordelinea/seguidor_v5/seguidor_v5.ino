#include "esp_adc/adc_oneshot.h"
#include "BluetoothSerial.h"
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
int baseSpeed = 130;
int turnSpeed = 40;
int maxSpeed = 250;
float reverseRatio = 0.25;
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

BluetoothSerial SerialBT;

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

  Serial.println(">>> PON EN BLANCO, ESPERANDO 3s... <<<");
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
  Serial.println(">>> PON EN NEGRO, ESPERANDO 3s... <<<");

  for (int j = 0; j < 200; j++) {
    for (int i = 0; i < SensorCount; i++) {
      uint16_t val = leerPin(i);
      if (val > sensorMax[i]) sensorMax[i] = val;
    }
    delay(5);
  }
  Serial.println(">>> LISTO <<<");
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

// ---------------- BLUETOOTH ----------------
void leerBluetooth() {
  if (SerialBT.available()) {
    char comando = SerialBT.read();
    float valor = SerialBT.parseFloat();
    while (SerialBT.available() > 0) { SerialBT.read(); }

    if (comando == 'V' || comando == 'v') {
      baseSpeed = (int)valor;
      SerialBT.print("Base: ");
      SerialBT.println(baseSpeed);
    } else if (comando == 'T' || comando == 't') {
      turnSpeed = (int)valor;
      SerialBT.print("Turn: ");
      SerialBT.println(turnSpeed);
    } else if (comando == 'A' || comando == 'a') {
      offsetA = (int)valor;
      SerialBT.print("OffsetA: ");
      SerialBT.println(offsetA);
    } else if (comando == 'B' || comando == 'b') {
      offsetB = (int)valor;
      SerialBT.print("OffsetB: ");
      SerialBT.println(offsetB);
    } else if (comando == 'R' || comando == 'r') {
      reverseRatio = valor;
      SerialBT.print("ReverseRatio: ");
      SerialBT.println(reverseRatio);
    } else if (comando == 'C' || comando == 'c') {
      calibrar();
    } else if (comando == 'L' || comando == 'l') {
      SerialBT.print("Pos: ");
      SerialBT.print(position);
      SerialBT.print(" | Sensores: ");
      for (int i = 0; i < SensorCount; i++) {
        SerialBT.print(sensorValues[i]);
        SerialBT.print(" ");
      }
      SerialBT.println();
    }
  }
}

// ---------------- MOTORES ----------------
void moverMotores(int speedA, int speedB) {
  speedA += offsetA;
  speedB += offsetB;

  if (speedA == 0 && speedB == 0) {
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    return;
  }

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
  //-------- aumentamos velocidad desactivando cosas innecesarias
  esp_wifi_stop();
  esp_wifi_deinit();
  setCpuFrequencyMhz(240);
  //-----
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  pinMode(pinEntrada, INPUT_PULLDOWN);

  setupADC();
  calibrar();

  digitalWrite(ledPin, HIGH);
  delay(1000);
}

// ---------------- LOOP ----------------
void loop() {
  // leerBluetooth();
  leerPosicion();

  //Serial.print("Pos: "); Serial.print(position);

  int revSpeed = -(baseSpeed * reverseRatio);

  int estado = digitalRead(pinEntrada);
  if (estado == LOW) {
    moverMotores(0, 0);
    return;
  }

  int speedA, speedB;

  if (position < 500) {
    speedA = constrain(baseSpeed * 1.35, 0, maxSpeed);
    speedB = revSpeed;

  } else if (position <= 1000) {
    speedA = constrain(baseSpeed * 1.35, 0, maxSpeed);
    speedB = 0;

  } else if (position <= 1700) {
    speedA = baseSpeed;
    speedB = 0;

  } else if (position <= 2500) {
    speedA = baseSpeed * 0.75;
    speedB = baseSpeed * 0.20;

  } else if (position < 3275) {
    speedA = baseSpeed;
    speedB = turnSpeed;

  } else if (position > 6700) {
    speedA = revSpeed;
    speedB = constrain(baseSpeed * 1.35, 0, maxSpeed);

  } else if (position >= 6500) {
    speedA = 0;
    speedB = constrain(baseSpeed * 1.35, 0, maxSpeed);

  } else if (position >= 5500) {
    speedA = 0;
    speedB = baseSpeed;

  } else if (position >= 4500) {
    speedA = baseSpeed * 0.20;
    speedB = baseSpeed * 0.75;

  } else if (position >= 3745) {
    speedA = turnSpeed;
    speedB = baseSpeed;

  } else {
    speedA = baseSpeed;
    speedB = baseSpeed;
  }

  moverMotores(speedA, speedB);
}
