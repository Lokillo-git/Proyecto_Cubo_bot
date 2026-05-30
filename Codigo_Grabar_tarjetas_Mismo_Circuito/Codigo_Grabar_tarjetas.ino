#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN A0
#define BUZZER_PIN A1

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

void beepOK() {
  tone(BUZZER_PIN, 3000);
  delay(80);
  noTone(BUZZER_PIN);
  delay(40);
  tone(BUZZER_PIN, 3000);
  delay(80);
  noTone(BUZZER_PIN);
}

void beepError() {
  for(byte i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 800);
    delay(100);
    noTone(BUZZER_PIN);
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);

  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  Serial.println(F("================================"));
  Serial.println(F("GRABADOR RFID BEEBOT"));
  Serial.println(F("Bloque utilizado: 4"));
  Serial.println(F("Comandos validos:"));
  Serial.println(F("A = Avanzar"));
  Serial.println(F("B = Retroceder"));
  Serial.println(F("D = Derecha"));
  Serial.println(F("I = Izquierda"));
  Serial.println(F("V = Ejecutar"));
  Serial.println(F("F = Reiniciar/Borrar"));
  Serial.println(F("================================"));
}

void loop() {

  if (!Serial.available()) return;

  String in = Serial.readStringUntil('\n');
  in.trim();

  if (in.length() == 0) return;

  char letra = toupper(in.charAt(0));

  if (!(letra=='A' || letra=='B' || letra=='D' ||
        letra=='I' || letra=='V' || letra=='F')) {

    Serial.println(F("Comando invalido"));
    beepError();
    return;
  }

  Serial.print(F("Comando seleccionado: "));
  Serial.println(letra);

  Serial.println(F("Acerque la tarjeta..."));

  while (!mfrc522.PICC_IsNewCardPresent()) {
    delay(20);
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println(F("Error leyendo tarjeta"));
    beepError();
    return;
  }

  Serial.print(F("UID: "));

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10)
      Serial.print("0");

    Serial.print(mfrc522.uid.uidByte[i], HEX);

    if (i < mfrc522.uid.size - 1)
      Serial.print(':');
  }

  Serial.println();

  byte blockAddr = 4;

  MFRC522::StatusCode status;

  status = mfrc522.PCD_Authenticate(
              MFRC522::PICC_CMD_MF_AUTH_KEY_A,
              blockAddr,
              &key,
              &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Error autenticacion: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    beepError();
    return;
  }

  byte buffer[16];

  for (byte i = 0; i < 16; i++)
    buffer[i] = 0;

  buffer[0] = letra;

  status = mfrc522.MIFARE_Write(blockAddr, buffer, 16);

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Error escritura: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    beepError();
    return;
  }

  Serial.println(F("Escritura OK"));

  byte readBuf[18];
  byte size = sizeof(readBuf);

  status = mfrc522.MIFARE_Read(blockAddr, readBuf, &size);

  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("Error verificacion"));
    beepError();
    return;
  }

  char ver = (char)readBuf[0];

  if (ver == letra) {
    Serial.println(F("VERIFICACION OK"));
    beepOK();
  } else {
    Serial.println(F("VERIFICACION FALLIDA"));
    beepError();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  Serial.println(F("Tarjeta lista."));
  Serial.println();
}