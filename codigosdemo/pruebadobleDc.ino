#define AIN1 21
#define AIN2 22
#define PWMA 23 
#define BIN1 18
#define BIN2 5
#define PWMB 4 
#define STBY 19

void setup() {
  Serial.begin(115200);
  pinMode(pinEntrada, INPUT_PULLDOWN);

  // 1. Configurar pines como salida
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);

  // 2. FORZAR TODO A LOW (Apagado total)
  // Esto limpia cualquier señal "HIGH" que haya quedado del arranque
  digitalWrite(STBY, LOW); 
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  
  // 3. Esperar a que los voltajes se estabilicen
  delay(1000); 

  // 4. Ahora sí, despertamos el driver
  digitalWrite(STBY, HIGH); 
  Serial.println("Driver reiniciado y pines limpios.");
}
void loop() {
  // --- ADELANTE ---
  Serial.println("Adelante 50");
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
  analogWrite(PWMA, 75);
  analogWrite(PWMB, 75);
  delay(1000);

  // --- PARADA TOTAL (MUY IMPORTANTE) ---
  Serial.println("Frenado Seco...");
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  delay(1500); // Dale tiempo a los motores de detenerse físicamente

  // --- ATRÁS (CON ARRANQUE SUAVE) ---
  Serial.println("Atrás 75");
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
  
  for(int i=0; i<=75; i+=5){
    analogWrite(PWMA, i);
    analogWrite(PWMB, i);
    delay(20);
  }
  delay(2000);

  // Parada antes de repetir
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  delay(1000);
}
