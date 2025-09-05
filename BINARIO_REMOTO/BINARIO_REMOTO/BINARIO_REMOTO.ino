#include <QTRSensors.h>

// ===== DEFINIÇÕES DOS MOTORES ===== //
#define MOTOR_A1 5
#define MOTOR_A2 7
#define MOTOR_B1 6
#define MOTOR_B2 8

// ===== PARÂMETROS DO PID ===== //
float KP = 0.20;
float KD = 2.0;
int velocidadeBase = 45;
int setpoint = 2500;
int delaySerial = 100;

// ===== VARIÁVEIS GLOBAIS ===== //
int erroAnterior = 0;
uint16_t posicaoLinha;
QTRSensors qtr;
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

void setup() {
  // ===== INICIALIZA COMUNICAÇÃO SERIAL ===== //
  Serial.begin(9600);
  
  // ===== CONFIGURA SENSORES ===== //
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2, 3, 9, 10, 11, 12}, SensorCount);
  qtr.setEmitterPin(1);

  // ===== CALIBRAÇÃO DOS SENSORES ===== //
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

  Serial.println("SISTEMA INICIADO - MODO SEGUIDOR DE LINHA");
}

void loop() {
  // ===== LEITURA DOS SENSORES ===== //
  posicaoLinha = qtr.readLineWhite(sensorValues);
  int erro = posicaoLinha - setpoint;
  int derivativo = erro - erroAnterior;
  
  // ===== CÁLCULO DO AJUSTE PID ===== //
  int ajuste = (KP * erro) + (KD * derivativo);
  ajuste = constrain(ajuste, -velocidadeBase, velocidadeBase);

  // ===== ENVIA DADOS VIA SERIAL (CABO USB) ===== //
  Serial.print("Erro:"); Serial.print(erro);
  Serial.print(",Ajuste:"); Serial.print(ajuste);
  Serial.print(",Posicao:"); Serial.print(posicaoLinha);
  Serial.print(",Setpoint:"); Serial.print(setpoint);
  Serial.print(",VelEsq:"); Serial.print(velocidadeBase + ajuste);
  Serial.print(",VelDir:"); Serial.println(velocidadeBase - ajuste);

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