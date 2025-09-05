#include <QTRSensors.h>

// ===== DEFINIÇÕES DOS PINOS DOS MOTORES ===== //
#define MOTOR_A1 5  // Motor A (Esquerdo) - Frente
#define MOTOR_A2 7  // Motor A (Esquerdo) - Trás
#define MOTOR_B1 6  // Motor B (Direito) - Frente
#define MOTOR_B2 8  // Motor B (Direito) - Trás

// ===== PARÂMETROS DO PID ===== //
float KP = 0.30;     // Ganho Proporcional
float KI = 0.00;     // Ganho Integral (NOVO)
float KD = 0.0;      // Ganho Derivativo
int velocidadeBase = 15;
int setpoint = 2500;

// ===== VARIÁVEIS GLOBAIS ===== //
int erroAnterior = 0;
int erroAcumulado = 0;  // N O V O
uint16_t posicaoLinha;
QTRSensors qtr;
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

// ===== LIMITES DE SEGURANÇA ===== //
const int INTEGRAL_MAX = 1000;  // Limite anti-windup

void setup() {
  // ===== CONFIGURAÇÃO DOS SENSORES QTR ===== //
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

  // ===== CONFIGURAÇÃO DOS MOTORES ===== //
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  Serial.begin(9600);  // Inicia comunicação serial para debug

}

void loop() {
  posicaoLinha = qtr.readLineWhite(sensorValues);

  // ===== CÁLCULO DO PID COMPLETO ===== //
  int erro = posicaoLinha - setpoint;
  
  // Termo Integral (acumula erro com limite)
  erroAcumulado += erro;
  erroAcumulado = constrain(erroAcumulado, -INTEGRAL_MAX, INTEGRAL_MAX);
  
  int derivativo = erro - erroAnterior;
  int ajuste = (KP * erro) + (KI * erroAcumulado) + (KD * derivativo);

  ajuste = constrain(ajuste, -velocidadeBase, velocidadeBase);

  // ===== APLICA AJUSTE AOS MOTORES ===== //
  int velocidadeEsq = velocidadeBase + ajuste;
  int velocidadeDir = velocidadeBase - ajuste;
  velocidadeEsq = constrain(velocidadeEsq, 0, 255);
  velocidadeDir = constrain(velocidadeDir, 0, 255);

  // Controle dos motores...
  analogWrite(MOTOR_A1, velocidadeEsq);
  digitalWrite(MOTOR_A2, LOW);
  analogWrite(MOTOR_B1, velocidadeDir);
  digitalWrite(MOTOR_B2, LOW);

  // Debug
  Serial.print("Erro:"); Serial.print(erro);
  Serial.print(" | Integral:"); Serial.print(erroAcumulado);
  Serial.print(" | Ajuste:"); Serial.println(ajuste);

  erroAnterior = erro;
  delay(5);
}