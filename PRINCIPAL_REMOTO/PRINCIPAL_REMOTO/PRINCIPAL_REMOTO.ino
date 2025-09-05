#include <QTRSensors.h>
#include <SoftwareSerial.h>  // ⬅️ Biblioteca para Bluetooth

// ===== CONFIGURAÇÃO BLUETOOTH ===== //
#define BT_RX 0    // Pino RX do Arduino (conecta ao TX do HC-05)
#define BT_TX 1    // Pino TX do Arduino (conecta ao RX do HC-05)
SoftwareSerial bluetooth(BT_RX, BT_TX);

#define MOTOR_A1 5
#define MOTOR_A2 7
#define MOTOR_B1 6
#define MOTOR_B2 8

// ===== PARÂMETROS ===== //
float KP = 0.20;
float KD = 2.0;
int velocidadeBase = 45;
int setpoint = 2500;
int delaySerial = 100;

// ===== VARIÁVEIS ===== //
int erroAnterior = 0;
uint16_t posicaoLinha;
QTRSensors qtr;
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

void setup() {
  // ===== INICIALIZA BLUETOOTH ===== //
  bluetooth.begin(9600);  // Padrão do HC-05
  Serial.begin(9600);
  
  // ===== CONFIGURA SENSORES ===== //
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2, 3, 9, 10, 11, 12}, SensorCount);
  qtr.setEmitterPin(1);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (uint16_t i = 0; i < 200; i++) {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW);

  // ===== CONFIGURA MOTORES ===== //
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  bluetooth.println("BLUETOOTH INICIADO!");
}

void loop() {
  posicaoLinha = qtr.readLineWhite(sensorValues);
  int erro = posicaoLinha - setpoint;
  int derivativo = erro - erroAnterior;
  
  int ajuste = (KP * erro) + (KD * derivativo);
  ajuste = constrain(ajuste, -velocidadeBase, velocidadeBase);

  // ===== ENVIA DADOS VIA BLUETOOTH ===== //
  bluetooth.print("Erro:"); bluetooth.print(erro);
  bluetooth.print(",Ajuste:"); bluetooth.print(ajuste);
  bluetooth.print(",Posicao:"); bluetooth.print(posicaoLinha);
  bluetooth.print(",Setpoint:"); bluetooth.print(setpoint);
  bluetooth.print(",VelEsq:"); bluetooth.print(velocidadeBase + ajuste);
  bluetooth.print(",VelDir:"); bluetooth.println(velocidadeBase - ajuste);

  // ===== ENVIA TAMBÉM PARA SERIAL (OPCIONAL) ===== //
  Serial.print("Erro:"); Serial.print(erro);
  Serial.print(",Posicao:"); Serial.println(posicaoLinha);

  // ===== CONTROLE DOS MOTORES ===== //
  int velocidadeEsq = velocidadeBase + ajuste;
  int velocidadeDir = velocidadeBase - ajuste;
  
  velocidadeEsq = constrain(velocidadeEsq, 0, 255);
  velocidadeDir = constrain(velocidadeDir, 0, 255);

  analogWrite(MOTOR_A1, velocidadeEsq);
  digitalWrite(MOTOR_A2, LOW);
  analogWrite(MOTOR_B1, velocidadeDir);
  digitalWrite(MOTOR_B2, LOW);

  erroAnterior = erro;
  delay(delaySerial);
}