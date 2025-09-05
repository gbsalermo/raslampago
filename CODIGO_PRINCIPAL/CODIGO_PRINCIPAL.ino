#include <QTRSensors.h>

// ===== DEFINIÇÕES DOS PINOS DOS MOTORES ===== //
#define MOTOR_A1 5  // Motor A (Esquerdo) - Frente
#define MOTOR_A2 7  // Motor A (Esquerdo) - Trás
#define MOTOR_B1 6  // Motor B (Direito) - Frente
#define MOTOR_B2 8  // Motor B (Direito) - Trás

// ===== PARÂMETROS DO PID ===== //
float KP = 0.24;     // Ganho Proporcional (suaviza correções)
float KD = 2.0;     // Ganho Derivativo (amortece oscilações)
int velocidadeBase = 20;  // Velocidade base (ajuste conforme piso)
int setpoint = 2500;      // Valor central dos sensores (0-5000)

// ===== VARIÁVEIS GLOBAIS ===== //
int erroAnterior = 0;
uint16_t posicaoLinha;
QTRSensors qtr;
const uint8_t SensorCount = 6;  // Número de sensores
uint16_t sensorValues[SensorCount];

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
  // ===== LEITURA DOS SENSORES ===== //
  posicaoLinha = qtr.readLineBlack(sensorValues);

  // ===== CÁLCULO DO PID ===== //
  int erro = posicaoLinha - setpoint;
  int derivativo = erro - erroAnterior;
  int ajuste = (KP * erro) + (KD * derivativo);

  // Limita o ajuste para não ultrapassar a velocidade base
  ajuste = constrain(ajuste, -velocidadeBase, velocidadeBase);

  // ===== APLICA AJUSTE AOS MOTORES ===== //
  int velocidadeEsq = velocidadeBase + ajuste;
  int velocidadeDir = velocidadeBase - ajuste;
  velocidadeEsq = constrain(velocidadeEsq, 0, 255);
  velocidadeDir = constrain(velocidadeDir, 0, 255);

  // ===== CONTROLE DOS MOTORES ===== //
  // Motor Esquerdo (A)
  analogWrite(MOTOR_A1, velocidadeEsq);  // Frente
  digitalWrite(MOTOR_A2, LOW);
  
  // Motor Direito (B)
  analogWrite(MOTOR_B1, velocidadeDir);  // Frente
  digitalWrite(MOTOR_B2, LOW);

  // ===== DEBUG (OPCIONAL) ===== //
  Serial.print("Erro:"); Serial.print(erro);
  Serial.print(" | Ajuste:"); Serial.println(ajuste);

  // ===== ATUALIZA VARIÁVEIS ===== //
  erroAnterior = erro;
  delay(5);  // Pequeno delay para estabilidade
}