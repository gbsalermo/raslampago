#include <QTRSensors.h>

// ===== DEFINIÇÕES DOS MOTORES ===== //
#define MOTOR_A1 5
#define MOTOR_A2 7
#define MOTOR_B1 6
#define MOTOR_B2 8

// ===== PARÂMETROS DO PID ===== //
float KP = 0.24;     // Ajuste fino: aumente se não fazer curvas, diminua se oscilar
float KD = 2.0;      // Amortecedor: aumente se oscilar em velocidade
int velocidadeBase = 15; // Velocidade média nos motores
int setpoint = 2500;     // Centro da linha (0-5000)
int limiteAjuste = 10; // LIMITE MAXIMO DE CORREÇÃO

// ===== VARIÁVEIS GLOBAIS ===== //
int erroAnterior = 0;
uint16_t posicaoLinha;
float posicaoFiltrada = 2500.0;  // ⬅️ VARIÁVEL DO FILTRO (inicia no centro)
QTRSensors qtr;
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

// ===== PARÂMETROS DO FILTRO ===== //
float alpha = 0.7;   // Fator de suavização (0.1 a 0.9)
                     // 0.1 = Muita suavização (lento) | 0.9 = Pouca suavização (rápido)

void setup() {
  // ===== INICIALIZAÇÃO DOS SENSORES ===== //
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2, 3, 9, 10, 11, 12}, SensorCount);
  qtr.setEmitterPin(1);

  // ===== CALIBRAÇÃO (FAÇA SOBRE A PISTA!) ===== //
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (uint16_t i = 0; i < 200; i++) {
    qtr.calibrate();
    delay(5);
  }
  digitalWrite(LED_BUILTIN, LOW);

  // ===== INICIALIZAÇÃO DOS MOTORES ===== //
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  // ===== COMUNICAÇÃO SERIAL (OPCIONAL) ===== //
  Serial.begin(9600);
}

void loop() {
  // ===== LEITURA RÁPIDA + FILTRO DE SUAVIZAÇÃO ===== //
  uint16_t posicaoBruta = qtr.readLineWhite(sensorValues);  // Leitura bruta
  posicaoFiltrada = (alpha * posicaoBruta) + ((1 - alpha) * posicaoFiltrada); // ⬅️ FILTRO
  posicaoLinha = (uint16_t)posicaoFiltrada;  // Converte para inteiro


  // ===== DEBUG (OPCIONAL) ===== //
  // Serial.print("Bruta:"); Serial.print(posicaoBruta);
  // Serial.print(" | Filtrada:"); Serial.println(posicaoLinha);

  // ===== CÁLCULO DO PID ===== //
  int erro = posicaoLinha - setpoint;
  int derivativo = erro - erroAnterior;
  int ajuste = (KP * erro) + (KD * derivativo);
  
  ajuste = constrain(ajuste, -limiteAjuste, limiteAjuste);
  // ===== CONTROLE DE VELOCIDADE DOS MOTORES ===== //
  int velocidadeEsq = velocidadeBase + ajuste;
  int velocidadeDir = velocidadeBase - ajuste;
  
  velocidadeEsq = constrain(velocidadeEsq, 0, 255);
  velocidadeDir = constrain(velocidadeDir, 0, 255);

  // ===== COMANDO DOS MOTORES ===== //
  analogWrite(MOTOR_A1, velocidadeEsq);
  digitalWrite(MOTOR_A2, LOW);
  analogWrite(MOTOR_B1, velocidadeDir);
  digitalWrite(MOTOR_B2, LOW);

  // ===== ATUALIZA VARIÁVEL PARA O PRÓXIMO LOOP ===== //
  erroAnterior = erro;
}