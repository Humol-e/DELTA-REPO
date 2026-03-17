//componentes.h
class MegaSumo {
  private:
    int PWMA, AIN1, AIN2;
    int PWMB, BIN1, BIN2;

  public:
    MegaSumo(int pa, int a1, int a2, int pb, int b1, int b2) {
      PWMA = pa; AIN1 = a1; AIN2 = a2;
      PWMB = pb; BIN1 = b1; BIN2 = b2;
      
      pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
      pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
      detener();
    }

    void avanzar(int vIzq, int vDer) {
      // Motor A (Izquierdo)
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      analogWrite(PWMA, vIzq);
      // Motor B (Derecho)
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
      analogWrite(PWMB, vDer);
    }

    void retroceder(int vIzq, int vDer) {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, vIzq);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
      analogWrite(PWMB, vDer);
    }

    void izquierda(int vIzq, int vDer) { // Giro sobre eje
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, vIzq);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
      analogWrite(PWMB, vDer);
    }

    void derecha(int vIzq, int vDer) { // Giro sobre eje
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      analogWrite(PWMA, vIzq);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
      analogWrite(PWMB, vDer);
    }

    void detener() {
      analogWrite(PWMA, 0);
      analogWrite(PWMB, 0);
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
    }

    void frenado(int v = 255) {
      // El TB6612 frena poniendo IN1 e IN2 en HIGH
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, HIGH);
      analogWrite(PWMA, v);
      analogWrite(PWMB, v);
    }
};
