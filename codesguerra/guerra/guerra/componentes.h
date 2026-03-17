class MegaSumo {
  private:
    int M1R;
    int M1L;
    int M2R;
    int M2L;

  public:
    MegaSumo(int m1r, int m1l, int m2r, int m2l) {
      M1R = m1r;
      M1L = m1l;
      M2R = m2r;
      M2L = m2l;
      pinMode(M1R, OUTPUT);
      pinMode(M1L, OUTPUT);
      pinMode(M2R, OUTPUT);
      pinMode(M2L, OUTPUT);
      detener();  // detener motores al iniciar
    }

    // --- Movimiento básico --- //
    void avanzar(int v) {
      analogWrite(M1R, 0);
      analogWrite(M1L, v);
      analogWrite(M2R, 0);
      analogWrite(M2L, v);
    }

    void retroceder(int v) {
      analogWrite(M1R, v);
      analogWrite(M1L, 0);
      analogWrite(M2R, v);
      analogWrite(M2L, 0);
    }

    void girarD(int v) { // derecha
      analogWrite(M1R, v);
      analogWrite(M1L, 0);
      analogWrite(M2R, 0);
      analogWrite(M2L, v);
    }

    void girarI(int v) { // izquierda
      analogWrite(M1R, 0);
      analogWrite(M1L, v);
      analogWrite(M2R, v);
      analogWrite(M2L, 0);
    }

    // --- Compatibilidad con código Bluepad32 --- //
    void avanzar(int vIzq, int vDer) {
      analogWrite(M1R, 0);
      analogWrite(M1L, vIzq);
      analogWrite(M2R, 0);
      analogWrite(M2L, vDer);
    }

    void retroceder(int vIzq, int vDer) {
      analogWrite(M1R, vIzq);
      analogWrite(M1L, 0);
      analogWrite(M2R, vDer);
      analogWrite(M2L, 0);
    }

    void izquierda(int vIzq, int vDer) {
      analogWrite(M1R, 0);
      analogWrite(M1L, vIzq);
      analogWrite(M2R, vDer);
      analogWrite(M2L, 0);
    }

    void derecha(int vIzq, int vDer) {
      analogWrite(M1R, vIzq);
      analogWrite(M1L, 0);
      analogWrite(M2R, 0);
      analogWrite(M2L, vDer);
    }

    // --- Detener motores --- //
    void detener() {
      analogWrite(M1R, 0);
      analogWrite(M1L, 0);
      analogWrite(M2R, 0);
      analogWrite(M2L, 0);
    }

    // --- Frenado activo --- //
    void frenado(int v = 255, int t = 0) {
      analogWrite(M1R, v);
      analogWrite(M1L, v);
      analogWrite(M2R, v);
      analogWrite(M2L, v);
      if (t > 0) delay(t);
      detener();
    }
};