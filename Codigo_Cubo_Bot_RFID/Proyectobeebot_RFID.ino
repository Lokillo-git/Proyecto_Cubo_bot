// -------------------------------------------
//  ROBOT BEEBOT - VERSIÓN FINAL MEJORADA
//  VELOCIDAD: 800us | RESET AUTOMÁTICO RFID
//  BUZZER MÁS FUERTE | DOBLE PITIDO AL FINALIZAR
//  REINICIO COMPLETO CON TARJETA DE BORRADO
// -------------------------------------------

#include <SPI.h>
#include <MFRC522.h>

// PINES
#define PIN_SS 10
#define PIN_RST A0
#define PIN_BUZZER A1

MFRC522 lectorRFID(PIN_SS, PIN_RST);

// MOTORES
const byte L[4] = {2, 3, 4, 5};
const byte R[4] = {6, 7, 8, 9};

// SECUENCIA DE PASOS (8 FASES)
const byte halfstep[8][4] PROGMEM = {
  {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
  {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
};

// PARÁMETROS CALIBRADOS
//const int pasosAvance = 5858; //Para cuadros de 15x15cm
const int pasosAvance = 6250; //Para cuadros de 16x16cm
const int pasosGiro90 = 3554;
const int ajustePostGiro = 1000;

// VELOCIDAD
unsigned int delayMicroPaso = 800;

// VARIABLES
char secuencia[30];
byte cantidadPasos = 0;
unsigned long lastReadTime = 0;
const unsigned long timeoutRFID = 5000;

// ==================== SONIDOS MEJORADOS ====================
// BUZZER MÁS FUERTE: usando frecuencia más aguda y mayor duración
void beep() { 
  tone(PIN_BUZZER, 2500);  // 2500Hz más agudo = más fuerte
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
  for(byte i=0;i<3;i++){ 
    tone(PIN_BUZZER, 800); 
    delay(100); 
    noTone(PIN_BUZZER); 
    delay(50);
  } 
}

void beepSecuenciaCompleta() {
  // DOS PITIDOS LARGOS PARA INDICAR SECUENCIA CONCLUIDA
  tone(PIN_BUZZER, 2000);
  delay(300);
  noTone(PIN_BUZZER);
  delay(150);
  tone(PIN_BUZZER, 2000);
  delay(300);
  noTone(PIN_BUZZER);
}

void beepReinicio() {
  // PITIDO DE REINICIO
  for(byte i=0;i<3;i++) {
    tone(PIN_BUZZER, 3000);
    delay(50);
    noTone(PIN_BUZZER);
    delay(50);
  }
}

// ==================== MOTORES ====================
void stepMotor(const byte* pins, byte paso) {
  byte stepVals[4];
  memcpy_P(stepVals, halfstep[paso], 4);
  for(byte i=0;i<4;i++) digitalWrite(pins[i], stepVals[i]);
}

void apagarMotores() {
  for(byte i=0;i<4;i++) { digitalWrite(L[i], LOW); digitalWrite(R[i], LOW); }
}

void moverSimultaneo(int pasosIzq, int pasosDer) {
  if(pasosIzq == 0 && pasosDer == 0) return;
  
  int maxPasos = max(abs(pasosIzq), abs(pasosDer));
  byte dirI = (pasosIzq > 0) ? 1 : (pasosIzq < 0) ? 255 : 0;
  byte dirD = (pasosDer > 0) ? 1 : (pasosDer < 0) ? 255 : 0;
  int contI=0, contD=0;
  byte pasoi=0, pasod=0;
  
  for(int i=0; i<maxPasos; i++) {
    if(contI < abs(pasosIzq)) {
      pasoi = (pasoi + dirI) & 7;
      stepMotor(L, pasoi);
      contI++;
    }
    if(contD < abs(pasosDer)) {
      pasod = (pasod + dirD) & 7;
      stepMotor(R, pasod);
      contD++;
    }
    delayMicroseconds(delayMicroPaso);
  }
  apagarMotores();
  delay(30);
}

void avanzar() { moverSimultaneo(-pasosAvance, pasosAvance); }
void retroceder() { moverSimultaneo(pasosAvance, -pasosAvance); }

void girarDerecha() { 
  moverSimultaneo(-ajustePostGiro, ajustePostGiro);
  moverSimultaneo(-pasosGiro90, -pasosGiro90);
  delay(50);
  moverSimultaneo(ajustePostGiro, -ajustePostGiro);
}

void girarIzquierda() { 
  moverSimultaneo(-ajustePostGiro, ajustePostGiro);
  moverSimultaneo(pasosGiro90, pasosGiro90);
  delay(50);
  moverSimultaneo(ajustePostGiro, -ajustePostGiro);
}

void ejecutarSecuencia() {
  if(cantidadPasos==0) { 
    beepError(); 
    return; 
  }
  
  for(byte i=0;i<cantidadPasos;i++) {
    switch(secuencia[i]) {
      case 'A': avanzar(); break;
      case 'B': retroceder(); break;
      case 'D': girarDerecha(); break;
      case 'I': girarIzquierda(); break;
    }
    delay(100);
  }
  
  // DOBLE PITIDO AL FINALIZAR LA SECUENCIA
  beepSecuenciaCompleta();
}

// ==================== REINICIO COMPLETO DEL SISTEMA ====================
void reiniciarSistema() {
  beepReinicio();           // Pitido de confirmación
  delay(500);
  
  // Limpiar variables
  cantidadPasos = 0;
  for(byte i=0;i<30;i++) secuencia[i] = '\0';
  
  // Apagar motores
  apagarMotores();
  
  // Reiniciar RFID
  lectorRFID.PCD_Init();
  lastReadTime = millis();
  
  // Pequeña pausa antes de reiniciar el Arduino
  delay(200);
  
  // REINICIO COMPLETO (como si presionaras el botón de reset)
  asm volatile ("jmp 0");   // Salta a la dirección 0 (reinicia el programa)
}

// ==================== RFID ====================
void reiniciarRFID() {
  lectorRFID.PCD_Init();
  delay(100);
}

void procesarRFID() {
  if (millis() - lastReadTime > timeoutRFID) {
    reiniciarRFID();
    lastReadTime = millis();
    return;
  }
  
  if (!lectorRFID.PICC_IsNewCardPresent()) return;
  if (!lectorRFID.PICC_ReadCardSerial()) return;
  
  lastReadTime = millis();
  
  MFRC522::MIFARE_Key clave;
  for(byte i=0;i<6;i++) clave.keyByte[i]=0xFF;
  if(lectorRFID.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &clave, &lectorRFID.uid) != MFRC522::STATUS_OK) return;
  
  byte buffer[18];
  byte tam = sizeof(buffer);
  if(lectorRFID.MIFARE_Read(4, buffer, &tam) != MFRC522::STATUS_OK) return;
  
  char comando = toupper(buffer[0]);
  beep();
  
  switch(comando) {
    case 'A': case 'B': case 'D': case 'I':
      if(cantidadPasos<30) {
        secuencia[cantidadPasos++] = comando;
        beepOK();
      } else beepError();
      break;
    case 'V': 
      ejecutarSecuencia(); 
      break;
    case 'F':  // TARJETA DE BORRADO - REINICIA COMPLETAMENTE EL SISTEMA
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
  
  // BUZZER CONFIGURADO COMO SALIDA
  pinMode(PIN_BUZZER, OUTPUT);
  
  for(byte i=0;i<4;i++) { 
    pinMode(L[i], OUTPUT); 
    pinMode(R[i], OUTPUT); 
  }
  apagarMotores();
  lastReadTime = millis();
  
  // PITIDO DE INICIO MÁS FUERTE
  tone(PIN_BUZZER, 2500); 
  delay(100); 
  noTone(PIN_BUZZER);
  delay(50);
  tone(PIN_BUZZER, 2500); 
  delay(100); 
  noTone(PIN_BUZZER);
}

void loop() { 
  procesarRFID(); 
}
