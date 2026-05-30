// ===========================================
// 🤖 ROBOT BEEBOT - SOLO BLUETOOTH
// Control por App Android
// ===========================================

// ---------------------------
// BUZZER
#define PIN_BUZZER A1

// ---------------------------
// MOTORES
// Motor Izquierdo
const int L[4] = {2, 3, 4, 5};  // Azul, Rosa, Amarillo, Naranja

// Motor Derecho
const int R[4] = {6, 7, 8, 9};  // Azul, Rosa, Amarillo, Naranja

// Secuencia half-step
int halfstep[8][4] = {
  {0,0,0,1},
  {0,0,1,1},
  {0,0,1,0},
  {0,1,1,0},
  {0,1,0,0},
  {1,1,0,0},
  {1,0,0,0},
  {1,0,0,1}
};

// ---------------------------
// PARÁMETROS
int delayPaso = 2;
int pasosAvance = 2673;
int pasosRetroceso = 2673;
int pasosGiro90 = 1024;

// ---------------------------
// VARIABLES
char secuencia[50];
int cantidadPasos = 0;

String comandoBT = "";
bool stopFlag = false;

// ---------------------------
// SONIDO
void beep() {
  tone(PIN_BUZZER, 1000);
  delay(100);
  noTone(PIN_BUZZER);
}

// ---------------------------
// MOTOR
void stepMotor(int *pins, int paso) {
  digitalWrite(pins[0], halfstep[paso][0]);
  digitalWrite(pins[1], halfstep[paso][1]);
  digitalWrite(pins[2], halfstep[paso][2]);
  digitalWrite(pins[3], halfstep[paso][3]);
}

void apagarMotores() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(L[i], LOW);
    digitalWrite(R[i], LOW);
  }
}

// ---------------------------
// MOVIMIENTO
void moverSimultaneo(long pasosIzq, long pasosDer) {
  long maxPasos = max(abs(pasosIzq), abs(pasosDer));

  int dirI = (pasosIzq > 0) ? 1 : -1;
  int dirD = (pasosDer > 0) ? 1 : -1;

  long contadorI = 0, contadorD = 0;
  int pasoI = 0, pasoD = 0;

  for (long i = 0; i < maxPasos; i++) {

    if (stopFlag) return; // 🔴 STOP inmediato

    if (contadorI < abs(pasosIzq)) {
      pasoI = (pasoI + dirI + 8) % 8;
      stepMotor((int*)L, pasoI);
      contadorI++;
    }

    if (contadorD < abs(pasosDer)) {
      pasoD = (pasoD + dirD + 8) % 8;
      stepMotor((int*)R, pasoD);
      contadorD++;
    }

    delay(delayPaso);
  }

  apagarMotores();
  delay(100);
}

// ---------------------------
// ACCIONES
void avanzar() {
  Serial.println("A");
  moverSimultaneo(pasosAvance, -pasosAvance);
}

void retroceder() {
  Serial.println("B");
  moverSimultaneo(-pasosRetroceso, pasosRetroceso);
}

void girarDerecha() {
  Serial.println("D");
  moverSimultaneo(pasosGiro90, pasosGiro90);
}

void girarIzquierda() {
  Serial.println("I");
  moverSimultaneo(-pasosGiro90, -pasosGiro90);
}

// ---------------------------
// EJECUTAR SECUENCIA
void ejecutarSecuencia() {

  if (cantidadPasos == 0) {
    Serial.println("VACIO");
    beep();
    return;
  }

  Serial.println("EJECUTANDO...");
  stopFlag = false;

  for (int i = 0; i < cantidadPasos; i++) {

    if (stopFlag) break;

    switch(secuencia[i]) {
      case 'A': avanzar(); break;
      case 'B': retroceder(); break;
      case 'D': girarDerecha(); break;
      case 'I': girarIzquierda(); break;
    }

    delay(300);
  }

  apagarMotores();
  Serial.println("FIN");
  beep();
}

// ---------------------------
// BLUETOOTH
void procesarBluetooth() {
  while (Serial.available()) {

    char c = Serial.read();

    if (c == '\n') {
      interpretarComandoBT(comandoBT);
      comandoBT = "";
    } else {
      comandoBT += c;
    }
  }
}

void interpretarComandoBT(String cmd) {
  cmd.toUpperCase();

  Serial.print("RECIBIDO: ");
  Serial.println(cmd);

  for (int i = 0; i < cmd.length(); i++) {

    char c = cmd[i];

    switch(c) {

      case 'A':
      case 'B':
      case 'D':
      case 'I':
        if (cantidadPasos < 50) {
          secuencia[cantidadPasos++] = c;
          Serial.print("ADD: ");
          Serial.println(c);
        }
        break;

      case 'V': // GO
        ejecutarSecuencia();
        break;

      case 'F': // RESET
        cantidadPasos = 0;
        Serial.println("RESET");
        beep();
        break;

      case 'S': // STOP
        stopFlag = true;
        apagarMotores();
        Serial.println("STOP");
        break;
    }
  }
}

// ---------------------------
// SETUP
void setup() {
  Serial.begin(9600); // 🔵 Bluetooth

  pinMode(PIN_BUZZER, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(L[i], OUTPUT);
    pinMode(R[i], OUTPUT);
  }

  apagarMotores();

  Serial.println("ROBOT LISTO - BLUETOOTH");
  beep();
}

// ---------------------------
// LOOP
void loop() {
  procesarBluetooth();
}
