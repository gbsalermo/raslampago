#include <QTRSensors.h>

#define MOTOR_A1 5
#define MOTOR_A2 7
#define MOTOR_B1 6
#define MOTOR_B2 8

// ===== PARÂMETROS DO PID ===== //
float KP = 0.24;
float KD = 0.0;
int velocidadeBase = 50;
int setpoint = 2500;

// ===== VARIÁVEIS GLOBAIS ===== //
int erroAnterior = 0;
uint16_t posicaoLinha;
QTRSensors qtr;
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

bool motoresAtivos = false; // ⬅️ Controle dos motores

void setup() {
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2, 3, 9, 10, 11, 12}, SensorCount);
  qtr.setEmitterPin(1);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (uint16_t i = 0; i < 200; i++) {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("Sistema Iniciado. Comandos:");
  Serial.println("'M' - Ativa/Desativa Motores");
  Serial.println("'D' - Mostra Dados de Debug");
}

void loop() {
  // ===== LEITURA E CÁLCULO (SEMPRE ATIVO) ===== //
  posicaoLinha = qtr.readLineBlack(sensorValues);
  int erro = posicaoLinha - setpoint;
  int derivativo = erro - erroAnterior;
  int ajuste = (KP * erro) + (KD * derivativo);
  //ajuste = constrain(ajuste, -velocidadeBase, velocidadeBase);

  

  int velocidadeEsq = velocidadeBase + ajuste;
  int velocidadeDir = velocidadeBase - ajuste;
  velocidadeEsq = constrain(velocidadeEsq, 0, 255);
  velocidadeDir = constrain(velocidadeDir, 0, 255);

  // ===== CONTROLE DOS MOTORES (SÓ SE ATIVADO) ===== //
  if (motoresAtivos) {
    analogWrite(MOTOR_A1, velocidadeEsq);
    digitalWrite(MOTOR_A2, LOW);
    analogWrite(MOTOR_B1, velocidadeDir);
    digitalWrite(MOTOR_B2, LOW);
    Serial.print("[MOTORES LIGADOS] "); // Aviso visual
  } else {
    // Se motores desativados, para os motores
    analogWrite(MOTOR_A1, 0);
    digitalWrite(MOTOR_A2, LOW);
    analogWrite(MOTOR_B1, 0);
    digitalWrite(MOTOR_B2, LOW);
  }

  // ===== COMANDOS VIA SERIAL ===== //
  if (Serial.available() > 0) {
    char comando = Serial.read();
    
    if (comando == 'M' || comando == 'm') {
      motoresAtivos = !motoresAtivos; // Liga/Desliga os motores
      Serial.print("Motores: ");
      Serial.println(motoresAtivos ? "LIGADOS" : "DESLIGADOS");
    }
    
    if (comando == 'D' || comando == 'd') {
      // Envia um PACOTE de dados quando solicitado
      Serial.print(",Posicao:"); Serial.print(posicaoLinha);
      Serial.print("Erro:"); Serial.print(erro);
      Serial.print(",Ajuste:"); Serial.print(ajuste);
      Serial.print(",Setpoint:"); Serial.print(setpoint);
      Serial.print(",VelEsq:"); Serial.print(velocidadeEsq);
      Serial.print(",VelDir:"); Serial.println(velocidadeDir);
    }
  }

  erroAnterior = erro;
  delay(50); // Delay reduzido para resposta rápida aos comandos
}