// Definimos el pin que vamos a usar
const int pinEntrada = 15;

void setup() {
  // Iniciamos la comunicación serial para ver los resultados en el monitor
  Serial.begin(115200);

  // Configuramos el pin 15 como entrada.
  // INPUT_PULLDOWN fuerza al pin a leer LOW (0V) si no hay voltaje conectado.
  pinMode(pinEntrada, INPUT_PULLDOWN);
}

void loop() {
  // Leemos el estado del pin
  int estado = digitalRead(pinEntrada);

  if (estado == HIGH) {
    // Si hay voltaje (3.3V)
    Serial.println("¡Voltaje DETECTADO! (HIGH)");
  } else {
    // Si no hay voltaje (0V / GND)
    Serial.println("Sin voltaje (LOW)");
  }

  // Pequeña pausa para no saturar el monitor serial
  delay(500); 
}