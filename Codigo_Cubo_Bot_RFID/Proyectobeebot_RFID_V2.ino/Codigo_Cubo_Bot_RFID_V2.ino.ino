 // -------------------------------------------
//  ROBOT BEEBOT - VERSIÓN FINAL CALIBRADA
//  VELOCIDAD: 800us | RESET AUTOMÁTICO RFID
//  BUZZER MEJORADO | DOBLE PITIDO AL FINALIZAR
//  CORRECCIÓN ANGULAR DE GIRO INDEPENDIENTE
//  REINICIO COMPLETO CON TARJETA DE BORRADO
// -------------------------------------------

#include <SPI.h>
#include <MFRC522.h>

// ==================== PINES ====================
#define PIN_SS     10
#define PIN_RST    A0
#define PIN_BUZZER A1

MFRC522 lectorRFID(PIN_SS, PIN_RST);

// ==================== MOTORES ====================
const byte L[4] = {2, 3, 4, 5};
const byte R[4] = {6, 7, 8, 9};

// SECUENCIA DE PASOS (8 FASES - HALF STEP)
const byte halfstep[8][4] PROGMEM = {
  {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
  {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
};

// ==================== PARÁMETROS DE MOVIMIENTO ====================
const int pasosAvance    = 5858;   // pasos para avanzar 1 cuadrado (16 cm)
const int pasosGiro90    = 3604;   // pasos para girar 90°
const int ajustePostGiro = 1000;   // pasos de centrado lateral pre/post giro

// ==================== VELOCIDAD ====================
unsigned int delayMicroPaso = 800; // microsegundos por paso (menor = más rápido)

// ─────────────────────────────────────────────────────────────────────
//  CORRECCIÓN ANGULAR DE GIRO
//
//  Ajusta la orientación final del robot después del giro de 90°.
//  Se aplica como una micro-rotación entre el giro principal y el
//  recentrado lateral, actuando solo sobre el ángulo sin mover
//  la posición del robot en el cuadrado.
//
//  CÓMO CALIBRAR:
//  1. Pon el robot centrado y alineado en un cuadrado.
//  2. Envía solo una tarjeta D (o I) + tarjeta V y observa.
//  3. Si la nariz quedó desviada a la DERECHA → sube el valor (+200).
//  4. Si la nariz quedó desviada a la IZQUIERDA → baja el valor (-200).
//  5. Afina con pasos de ±50 hasta que quede paralelo a la cuadrícula.
//  6. Repite el mismo proceso para girarIzquierda de forma independiente.
//
//  REFERENCIA: ~100 pasos ≈ 2-3° de corrección (con delay 800us)
//  Rango típico necesario: entre -400 y +400 pasos
// ─────────────────────────────────────────────────────────────────────
int ajusteAngularDerecha   = 0;   // ← modifica para calibrar girarDerecha()
int ajusteAngularIzquierda = 0;   // ← modifica para calibrar girarIzquierda()

// ==================== VARIABLES DE SECUENCIA ====================
char secuencia[30];
byte cantidadPasos = 0;
unsigned long lastReadTime = 0;
const unsigned long timeoutRFID = 5000;

// ==================== SONIDOS ====================
void beep() {
  tone(PIN_BUZZER, 2500);
  delay(100);
  noTone(PIN_BUZZER);
}

void beepOK() {
  tone(PIN_BUZZER, 3000);
  delay(80);
  noTone(PIN_BUZZER);
  delay(40);
  tone(PIN_BUZZER, 3000);
  delay(80);
  noTone(PIN_BUZZER);
}

void beepError() {
  for (byte i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 800);
    delay(100);
    noTone(PIN_BUZZER);
    delay(50);
  }
}

void beepSecuenciaCompleta() {
  tone(PIN_BUZZER, 2000);
  delay(300);
  noTone(PIN_BUZZER);
  delay(150);
  tone(PIN_BUZZER, 2000);
  delay(300);
  noTone(PIN_BUZZER);
}

void beepReinicio() {
  for (byte i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 3000);
    delay(50);
    noTone(PIN_BUZZER);
    delay(50);
  }
}

// ==================== CONTROL DE MOTORES ====================
void stepMotor(const byte* pins, byte paso) {
  byte stepVals[4];
  memcpy_P(stepVals, halfstep[paso], 4);
  for (byte i = 0; i < 4; i++) digitalWrite(pins[i], stepVals[i]);
}

void apagarMotores() {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(L[i], LOW);
    digitalWrite(R[i], LOW);
  }
}

// Mueve ambos motores simultáneamente.
// Positivo = sentido horario, Negativo = antihorario, 0 = quieto.
void moverSimultaneo(int pasosIzq, int pasosDer) {
  if (pasosIzq == 0 && pasosDer == 0) return;

  int maxPasos = max(abs(pasosIzq), abs(pasosDer));
  byte dirI = (pasosIzq > 0) ? 1 : (pasosIzq < 0) ? 255 : 0;
  byte dirD = (pasosDer > 0) ? 1 : (pasosDer < 0) ? 255 : 0;
  int contI = 0, contD = 0;
  byte pasoi = 0, pasod = 0;

  for (int i = 0; i < maxPasos; i++) {
    if (contI < abs(pasosIzq)) {
      pasoi = (pasoi + dirI) & 7;
      stepMotor(L, pasoi);
      contI++;
    }
    if (contD < abs(pasosDer)) {
      pasod = (pasod + dirD) & 7;
      stepMotor(R, pasod);
      contD++;
    }
    delayMicroseconds(delayMicroPaso);
  }
  apagarMotores();
  delay(30);
}

// ==================== MOVIMIENTOS BÁSICOS ====================
void avanzar() {
  moverSimultaneo(-pasosAvance, pasosAvance);
}

void retroceder() {
  moverSimultaneo(pasosAvance, -pasosAvance);
}

// ==================== GIROS CON CORRECCIÓN ANGULAR ====================
void girarDerecha() {
  // 1. Ajuste previo: centra el robot lateralmente en el cuadrado
  moverSimultaneo(-ajustePostGiro, ajustePostGiro);

  // 2. Giro principal de 90° (ambas ruedas en mismo sentido)
  moverSimultaneo(-pasosGiro90, -pasosGiro90);
  delay(50);

  // 3. Corrección angular fina de orientación
  //    Positivo → corrige si la nariz quedó girada de más hacia la derecha
  //    Negativo → corrige si la nariz quedó girada de más hacia la izquierda
  if (ajusteAngularDerecha != 0) {
    moverSimultaneo(-ajusteAngularDerecha, ajusteAngularDerecha);
    delay(30);
  }

  // 4. Recentrado lateral para el siguiente movimiento
  moverSimultaneo(ajustePostGiro, -ajustePostGiro);
}

void girarIzquierda() {
  // 1. Ajuste previo: centra el robot lateralmente en el cuadrado
  moverSimultaneo(-ajustePostGiro, ajustePostGiro);

  // 2. Giro principal de 90° (ambas ruedas en mismo sentido, dirección opuesta)
  moverSimultaneo(pasosGiro90, pasosGiro90);
  delay(50);

  // 3. Corrección angular fina de orientación
  //    Mismo criterio que girarDerecha pero de forma independiente
  if (ajusteAngularIzquierda != 0) {
    moverSimultaneo(-ajusteAngularIzquierda, ajusteAngularIzquierda);
    delay(30);
  }

  // 4. Recentrado lateral para el siguiente movimiento
  moverSimultaneo(ajustePostGiro, -ajustePostGiro);
}

// ==================== EJECUCIÓN DE SECUENCIA ====================
void ejecutarSecuencia() {
  if (cantidadPasos == 0) {
    beepError();
    return;
  }

  for (byte i = 0; i < cantidadPasos; i++) {
    switch (secuencia[i]) {
      case 'A': avanzar();       break;
      case 'B': retroceder();    break;
      case 'D': girarDerecha();  break;
      case 'I': girarIzquierda(); break;
    }
    delay(100);
  }

  beepSecuenciaCompleta();
}

// ==================== REINICIO COMPLETO DEL SISTEMA ====================
void reiniciarSistema() {
  beepReinicio();
  delay(500);

  cantidadPasos = 0;
  for (byte i = 0; i < 30; i++) secuencia[i] = '\0';

  apagarMotores();
  lectorRFID.PCD_Init();
  lastReadTime = millis();

  delay(200);
  asm volatile ("jmp 0"); // Reinicio completo (equivale a pulsar Reset)
}

// ==================== LECTOR RFID ====================
void reiniciarRFID() {
  lectorRFID.PCD_Init();
  delay(100);
}

void procesarRFID() {
  // Reinicio periódico del lector para evitar bloqueos
  if (millis() - lastReadTime > timeoutRFID) {
    reiniciarRFID();
    lastReadTime = millis();
    return;
  }

  if (!lectorRFID.PICC_IsNewCardPresent()) return;
  if (!lectorRFID.PICC_ReadCardSerial())   return;

  lastReadTime = millis();

  MFRC522::MIFARE_Key clave;
  for (byte i = 0; i < 6; i++) clave.keyByte[i] = 0xFF;

  if (lectorRFID.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &clave, &lectorRFID.uid) != MFRC522::STATUS_OK) return;

  byte buffer[18];
  byte tam = sizeof(buffer);
  if (lectorRFID.MIFARE_Read(4, buffer, &tam) != MFRC522::STATUS_OK) return;

  char comando = toupper(buffer[0]);
  beep();

  switch (comando) {
    case 'A':
    case 'B':
    case 'D':
    case 'I':
      if (cantidadPasos < 30) {
        secuencia[cantidadPasos++] = comando;
        beepOK();
      } else {
        beepError();
      }
      break;

    case 'V':  // Ejecutar secuencia cargada
      ejecutarSecuencia();
      break;

    case 'F':  // Tarjeta de borrado — reinicia completamente el sistema
      reiniciarSistema();
      break;

    default:
      beepError();
  }

  lectorRFID.PICC_HaltA();
  lectorRFID.PCD_StopCrypto1();
  delay(150);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  SPI.begin();
  lectorRFID.PCD_Init();

  pinMode(PIN_BUZZER, OUTPUT);

  for (byte i = 0; i < 4; i++) {
    pinMode(L[i], OUTPUT);
    pinMode(R[i], OUTPUT);
  }

  apagarMotores();
  lastReadTime = millis();

  // Pitido doble de inicio
  tone(PIN_BUZZER, 2500); delay(100); noTone(PIN_BUZZER);
  delay(50);
  tone(PIN_BUZZER, 2500); delay(100); noTone(PIN_BUZZER);
}

// ==================== LOOP ====================
void loop() {
  procesarRFID();
}
