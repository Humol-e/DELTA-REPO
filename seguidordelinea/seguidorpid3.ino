#include "esp_adc/adc_oneshot.h"
#include "BluetoothSerial.h"
#include "esp_wifi.h"
#include <IRremote.h>

#define pwmi 23
#define izq1 22
#define izq2 21
#define pwmd 4
#define der1 5
#define der2 18
#define STBY 19

#define PIN_IR 34
const int ledPin = 2;


unsigned long ultimoIR = 0;
const unsigned long DEBOUNCE_IR = 300;  // 300ms sin rebotes
#define IR_ss 0XFF00FF00                // boton power
#define IR_CALIBRACION 0xFE01FF00       //boton mode
#define IR_vel 0xFD02FF00               //Boton mute

#define IR_P 0XFB04FF00   //seleccionar kp
#define IR_M1 0XFA05FF00  //aumentar parametro, es el de dos flechas derecha arriba de play
#define IR_I 0XF906FF00   //selccionar ki pa editar

#define IR_ANum 0XF708FF00  // es el de seleccion manual, izquierda de play
#define IR_Send 0XF609FF00  // boton play
#define IR_Pnt 0XF50AFF00   // boton derecha de play

#define IR_D 0XF30CFF00   //boton title
#define IR_m1 0XF20DFF00  // disminuye parametro, abajo de play
#define IR_0 0XF10EFF00

#define IR_1 0XEF10FF00
#define IR_2 0XEE11FF00
#define IR_3 0XED12FF00

#define IR_4 0XEB14FF00
#define IR_5 0XEA15FF00
#define IR_6 0XE916FF00

#define IR_7 0XE718FF00
#define IR_8 0XE619FF00
#define IR_9 0XE51AFF00
// ---------------- SENSOR ADC ----------------
const uint8_t pines[] = {13, 14, 27, 26, 25, 33, 32, 35};
const uint8_t SensorCount = 8;


uint16_t sensorValues[SensorCount];
uint16_t sensorMin[SensorCount];
uint16_t sensorMax[SensorCount];

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_handle_t adc2_handle;

struct PinADC {
  adc_unit_t unit;
  adc_channel_t channel;
};

const PinADC mapeoADC[] = {
  {ADC_UNIT_2, ADC_CHANNEL_4},  // GPIO 13
  {ADC_UNIT_2, ADC_CHANNEL_6},  // GPIO 14
  {ADC_UNIT_2, ADC_CHANNEL_7},  // GPIO 27
  {ADC_UNIT_2, ADC_CHANNEL_9},  // GPIO 26
  {ADC_UNIT_2, ADC_CHANNEL_8},  // GPIO 25
  {ADC_UNIT_1, ADC_CHANNEL_5},  // GPIO 33
  {ADC_UNIT_1, ADC_CHANNEL_4},  // GPIO 32
  {ADC_UNIT_1, ADC_CHANNEL_7},  // GPIO 35
};


int calibrando = 0;  // 0 = en proceso, 1 = listo
int Ecal = 0;        // 0 = blancos, 1 = negros
bool robotActivado = false;

bool botonPresionado = false;

int linea = 0;  //0 = linea negra, 1 = linea blanca

int pos = 0, ultimosensor = 0;  //Variables de posición de los sonsores y ultimo sensore detectado

//Velocidades
int vel = 80;  // 80  100
int veladelante = 110;
int velatras = -110;

//Pesos Variables PID

float kp = 0.6;    //(1.5) 2.1  3.0    0      0     6.0     0
float kd = 12;     //  0     0    0.1     0      0     0.2
float ki = 0.002;  //   0     0     0    0.003    0      0

// Se camniara dependiendo de lo seleccionado P, I, D
char selecPID = 'A';

// Modo manual
bool modoManual = false;
String Numero = "";

//Variables PID
int proporcional = 0;
int integral = 0;
int derivativa = 0;
int diferencial = 0;
int ultimo_prop;
int setpoint = 350;

//Errores

int error1 = 0;
int error2 = 0;
int error3 = 0;
int error4 = 0;
int error5 = 0;
int error6 = 0;


// ---------------- PWM CANALES ----------------
const int pwmFreq = 20000;  // 20kHz
const int pwmResolution = 8;
const int pwmChannelA = 0;
const int pwmChannelB = 1;


// ---------------- ADC SETUP ----------------
void setupADC() {
  adc_oneshot_unit_init_cfg_t cfg1 = {.unit_id = ADC_UNIT_1};
  adc_oneshot_unit_init_cfg_t cfg2 = {.unit_id = ADC_UNIT_2};
  adc_oneshot_new_unit(&cfg1, &adc1_handle);
  adc_oneshot_new_unit(&cfg2, &adc2_handle);

  adc_oneshot_chan_cfg_t chanCfg = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12  // 12 bits para mayor resolución
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
  Serial.println("CALIBRAA");
  for (int i = 0; i < SensorCount; i++) {
    sensorMin[i] = 4095;
    sensorMax[i] = 0;
  }
digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  Serial.println("PON EN BLANCO, ESPERANDO...");
  
  unsigned long inicio = millis();
  while (millis() - inicio < 3000) {
    for (int i = 0; i < SensorCount; i++) {
      uint16_t val = leerPin(i);
      if (val < sensorMin[i]) sensorMin[i] = val;
    }
  }
  
  digitalWrite(ledPin, LOW);
  delay(500);
  Serial.println("PON EN NEGRO, ESPERANDO...");
digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
      delay(500);

  inicio = millis();
  
  while (millis() - inicio < 3000) {
    for (int i = 0; i < SensorCount; i++) {
      uint16_t val = leerPin(i);
      if (val > sensorMax[i]) sensorMax[i] = val;
    }
  }
  
  digitalWrite(ledPin, LOW);
  Serial.println("CALIBRACION COMPLETA");
}


// ---------------- LECTURA NORMALIZADA ----------------
uint16_t normalizar(int i, uint16_t valor) {
  if (sensorMax[i] == sensorMin[i]) return 0;
  return constrain(map(valor, sensorMin[i], sensorMax[i], 0, 1000), 0, 1000);
}

int lectura() {
  long suma = 0;
  long total = 0;
  int activoCount = 0;

  for (int i = 0; i < SensorCount; i++) {
    uint16_t raw = leerPin(i);
    sensorValues[i] = normalizar(i, raw);

    if (sensorValues[i] > 500) {  // umbral ajustable
      suma += (long)sensorValues[i] * i * 100;
      total += sensorValues[i];
      activoCount++;
    }
  }

  if (activoCount > 0) {
    pos = suma / total;
    ultimosensor = pos;
  } else {
    // Si no detecta nada, usar última posición conocida
    pos = ultimosensor;
  }

  return pos;
}

void setup() {
  Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
  esp_wifi_stop();
  esp_wifi_deinit();
  setCpuFrequencyMhz(240);
  
  pinMode(izq1, OUTPUT);
  pinMode(izq2, OUTPUT);
  pinMode(der1, OUTPUT);
  pinMode(der2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STBY, OUTPUT);
  IrReceiver.begin(PIN_IR, DISABLE_LED_FEEDBACK);

  // Setup PWM 20kHz
  ledcAttach(pwmi, pwmFreq, pwmResolution);
  ledcAttach(pwmd, pwmFreq, pwmResolution);
  
  digitalWrite(STBY, HIGH);
  
  IrReceiver.begin(PIN_IR, DISABLE_LED_FEEDBACK);
  setupADC();
  
  Serial.println("=== CALIBRACION INICIAL ===");
  calibrar();
  
  Serial.println("=== LISTO PARA COMPETIR ===");
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
}

void loop() {
  unsigned long codigo;
  // Control IR
  if (IrReceiver.decode()) {
    unsigned long codigo = IrReceiver.decodedIRData.decodedRawData;
    procesarIR(codigo);
    IrReceiver.resume();
  }
  if (robotActivado) {
    digitalWrite(ledPin, HIGH);
    lectura();
    PID();
    frenos();
    Serial.println(pos);
  } else {
    digitalWrite(ledPin, LOW);
    motores(0, 0);
  }
}


// ---------------- MOTORES CON PWM 20kHz ----------------
void motores(int izq, int der) {
  // Motor izquierdo
  if (izq >= 0) {
    digitalWrite(izq1, HIGH);
    digitalWrite(izq2, LOW);
    ledcWrite(pwmd, constrain(izq, 0, 255));
  } else {
    digitalWrite(izq1, LOW);
    digitalWrite(izq2, HIGH);
    ledcWrite(pwmd, constrain(-izq, 0, 255));
  }

  // Motor derecho
  if (der >= 0) {
    digitalWrite(der1, HIGH);
    digitalWrite(der2, LOW);
    ledcWrite(pwmi, constrain(der, 0, 255));
  } else {
    digitalWrite(der1, LOW);
    digitalWrite(der2, HIGH);
    ledcWrite(pwmi, constrain(-der, 0, 255));
  }
}

// ---------------- FRENOS MEJORADOS ----------------
void frenos() {
  if (pos <= 100) {
    motores(velatras, veladelante);
  } else if (pos >= 600) {
    motores(veladelante, velatras);
  }
}

void PID() {
  proporcional = pos - setpoint;
  derivativa = proporcional - ultimo_prop;
  integral = error1 + error2 + error3 + error4 + error5 + error6;
  ultimo_prop = proporcional;
  error6 = error5;
  error5 = error4;
  error4 = error3;
  error3 = error2;
  error2 = error1;
  error1 = proporcional;
  diferencial = proporcional * kp + derivativa * kd + integral * ki;
  if (diferencial > vel) diferencial = vel;
  else if (diferencial < -vel) diferencial = -vel;
  (diferencial < 0) ? motores(vel, vel - diferencial) : motores(vel + diferencial, vel);
  Serial.println(diferencial);
}



// ---------------- MANEJO IR ----------------
void procesarIR(unsigned long codigo) {
  if (codigo == IR_ss && (millis() - ultimoIR) > DEBOUNCE_IR) {
    ultimoIR = millis();
    robotActivado = !robotActivado;
    Serial.print("Estado: ");
    Serial.println(robotActivado ? "ACTIVADO" : "DETENIDO");
    return;
  }
  if (codigo == IR_CALIBRACION && !botonPresionado) {
        botonPresionado = true;
        calibrar();
  }
  // Selección de parámetro
  if (codigo == IR_P) {
    selecPID = 'P';
    Serial.println("Seleccionado KP");
  } else if (codigo == IR_I) {
    selecPID = 'I';
    Serial.println("Seleccionado KI");
  } else if (codigo == IR_D) {
    selecPID = 'D';
    Serial.println("Seleccionado KD");
  } else if (codigo == IR_vel) {
    selecPID = 'V';
    Serial.println("Seleccionado VEL");
  }

  // Incremento automático
  if (codigo == IR_M1) {
    if (selecPID == 'P') kp += 0.1;
    if (selecPID == 'I') ki += 0.001;
    if (selecPID == 'D') kd += 1.0;
    if (selecPID == 'V') vel = constrain(vel + 5, 0, 255);
  }

  // Decremento automático
  if (codigo == IR_m1) {
    if (selecPID == 'P') kp = max(0.0f, kp - 0.1f);
    if (selecPID == 'I') ki = max(0.0f, ki - 0.001f);
    if (selecPID == 'D') kd = max(0.0f, kd - 1.0f);
    if (selecPID == 'V') vel = max(0, vel - 5);
  }

  // Modo manual
  if (codigo == IR_ANum) {
    modoManual = true;
    Numero = "";
    Serial.println("Modo manual activado");
  }

  if (modoManual) {
    if (codigo == IR_0) Numero += "0";
    if (codigo == IR_1) Numero += "1";
    if (codigo == IR_2) Numero += "2";
    if (codigo == IR_3) Numero += "3";
    if (codigo == IR_4) Numero += "4";
    if (codigo == IR_5) Numero += "5";
    if (codigo == IR_6) Numero += "6";
    if (codigo == IR_7) Numero += "7";
    if (codigo == IR_8) Numero += "8";
    if (codigo == IR_9) Numero += "9";
    if (codigo == IR_Pnt) Numero += ".";
  }

  // Confirmar valor manual
  if (codigo == IR_Send && modoManual) {
    float valor = Numero.toFloat();
    if (selecPID == 'P') kp = valor;
    if (selecPID == 'I') ki = valor;
    if (selecPID == 'D') kd = valor;
    if (selecPID == 'V') vel = constrain((int)valor, 0, 255);
    
    Serial.print("Valor asignado: ");
    Serial.println(valor);
    modoManual = false;
    Numero = "";
  }

  // Imprimir estado solo cuando cambia algo
  if (codigo == IR_P || codigo == IR_I || codigo == IR_D || 
      codigo == IR_M1 || codigo == IR_m1 || codigo == IR_Send) {
    Serial.print("KP:");
    Serial.print(kp, 2);
    Serial.print(" KI:");
    Serial.print(ki, 4);
    Serial.print(" KD:");
    Serial.print(kd, 1);
    Serial.print(" VEL:");
    Serial.println(vel);
  }
}

void imprimirBoton(unsigned long codigo) {
  Serial.println("\n========== BOTÓN PRESIONADO ==========");

  if (codigo == IR_ss) Serial.println("🔴 START/STOP → Enciende/apaga robot");
  else if (codigo == IR_CALIBRACION) Serial.println("⚙️  CALIBRACIÓN → Calibra sensores");
  else if (codigo == IR_vel) Serial.println("⚡ VELOCIDAD → Modo manual de velocidad");
  else if (codigo == IR_P) Serial.println("📊 KP SELECTED → Listo para ajustar proporcional");
  else if (codigo == IR_I) Serial.println("📊 KI SELECTED → Listo para ajustar integral");
  else if (codigo == IR_D) Serial.println("📊 KD SELECTED → Listo para ajustar derivativa");
  else if (codigo == IR_M1) Serial.println("➕ INCREASE → Aumenta parámetro actual");
  else if (codigo == IR_m1) Serial.println("➖ DECREASE → Disminuye parámetro actual");
  else if (codigo == IR_ANum) Serial.println("🔢 MODO MANUAL → Escribe número (0-9)");
  else if (codigo == IR_Send) Serial.println("✅ SEND → Confirma valor ingresado");
  else if (codigo == IR_Pnt) Serial.println("🔹 PUNTO → Agrega decimal (.）");
  else if (codigo >= IR_0 && codigo <= IR_9) {
    int numero = codigo == IR_0 ? 0 : codigo == IR_1 ? 1
                                    : codigo == IR_2 ? 2
                                    : codigo == IR_3 ? 3
                                    : codigo == IR_4 ? 4
                                    : codigo == IR_5 ? 5
                                    : codigo == IR_6 ? 6
                                    : codigo == IR_7 ? 7
                                    : codigo == IR_8 ? 8
                                                     : 9;
    Serial.print("🔢 NÚMERO → ");
    Serial.println(numero);
  } else Serial.println("❓ DESCONOCIDO → Código no mapeado");

  Serial.print("   Estado actual → KP: ");
  Serial.print(kp);
  Serial.print(" | KI: ");
  Serial.print(ki);
  Serial.print(" | KD: ");
  Serial.print(kd);
  Serial.print(" | VEL: ");
  Serial.println(vel);
  Serial.println("======================================\n");
}