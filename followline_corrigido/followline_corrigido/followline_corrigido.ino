// =============================================================
//  FOLLOW LINE — CÓDIGO BASE CORRIGIDO
//  Placa: Arduino Uno / Nano
//  Sensores: QTR-RC (6 sensores)
//  Controle: PID completo (P + D)
// =============================================================

#include <QTRSensors.h>

// ===== PINOS DOS MOTORES =====
// PWM disponíveis no Uno/Nano: 3, 5, 6, 9, 10, 11
#define MOTOR_A_PWM  5   // Motor Esquerdo — velocidade
#define MOTOR_A_DIR  4   // Motor Esquerdo — direção (digital puro, ok)
#define MOTOR_B_PWM  6   // Motor Direito  — velocidade
#define MOTOR_B_DIR  7   // Motor Direito  — direção (digital puro, ok)

// ===== PINOS DOS SENSORES =====
// ATENÇÃO: pino 1 removido (é o TX do Serial — causava interferência!)
// Emitter movido para pino 13 (LED_BUILTIN, livre durante corrida)
#define EMITTER_PIN     13
#define BOTAO_CALIBRAR  A0   // Botão para iniciar calibração (pull-up interno)

const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];
QTRSensors qtr;

// ===== PARÂMETROS DO PID =====
// Ajuste KP e KD com o robô em pista — comece por KP, depois KD
float KP = 0.08;   // Proporcional: aumenta se o robô não corrige rápido o suficiente
float KD = 0.5;    // Derivativo:   aumenta se o robô oscila (estava 0.0 — era o problema!)
float KI = 0.0;    // Integral:     deixe 0 por enquanto, raramente necessário em follow line

// ===== VELOCIDADE =====
// Agora pode ser maior pois o PID e o Serial estão corretos
// Comece com 80, aumente aos poucos até o robô perder a linha
int VELOCIDADE_BASE = 80;   // 0–255 (era 15 — muito baixo por causa dos bugs)
int VELOCIDADE_MIN  = 0;    // Mínimo permitido por motor
int VELOCIDADE_MAX  = 200;  // Máximo permitido por motor (deixa margem de segurança)

// ===== SETPOINT =====
// Para 6 sensores o range é 0–5000, centro = 2500
int SETPOINT = 2500;

// ===== VARIÁVEIS DO PID =====
int   erroAnterior  = 0;
float integrador    = 0;
float integrador_max = 500; // Limita o integrador para evitar windup

// ===== CONTROLE DE DEBUG SERIAL =====
// Serial só imprime a cada N ciclos — não interfere no timing do PID
#define DEBUG_SERIAL      true   // false = desativa completamente na corrida
#define DEBUG_INTERVALO   100    // Imprime a cada 100 ciclos
unsigned long cicloAtual = 0;

// ===== PROTÓTIPOS =====
void calibrar();
void moverMotores(int velEsq, int velDir);
void pararMotores();
void debugSerial(int erro, int ajuste, int velEsq, int velDir);

// =============================================================
void setup() {
  // --- Serial ---
  Serial.begin(115200); // 115200 é muito mais rápido que 9600 — menos travamento
  Serial.println(F("Follow Line — iniciando..."));

  // --- Sensores ---
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2, 3, 8, 9, 10, 11}, SensorCount);
  qtr.setEmitterPin(EMITTER_PIN); // Pino 13 — LONGE do TX (pino 1)

  // --- Botão de calibração ---
  pinMode(BOTAO_CALIBRAR, INPUT_PULLUP);

  // --- Motores ---
  pinMode(MOTOR_A_PWM, OUTPUT);
  pinMode(MOTOR_A_DIR, OUTPUT);
  pinMode(MOTOR_B_PWM, OUTPUT);
  pinMode(MOTOR_B_DIR, OUTPUT);
  pararMotores();

  // --- LED interno como indicador ---
  pinMode(LED_BUILTIN, OUTPUT); // Pino 13 compartilha com LED, OK para sinalização

  // --- Aguarda botão para calibrar ---
  Serial.println(F("Pressione o botao (A0) para iniciar calibracao..."));
  Serial.println(F("Passe o robo sobre a linha durante a calibracao."));

  while (digitalRead(BOTAO_CALIBRAR) == HIGH) {
    // Pisca LED esperando o botão
    digitalWrite(LED_BUILTIN, millis() % 500 < 250);
  }
  delay(200); // Debounce do botão

  calibrar();

  Serial.println(F("Calibracao concluida! Pressione o botao novamente para iniciar."));
  while (digitalRead(BOTAO_CALIBRAR) == HIGH) {
    digitalWrite(LED_BUILTIN, millis() % 200 < 100); // Pisca mais rápido
  }
  delay(200);

  digitalWrite(LED_BUILTIN, HIGH); // LED fixo = rodando
  Serial.println(F("Iniciando corrida!"));
}

// =============================================================
void loop() {
  cicloAtual++;

  // ===== 1. LEITURA DOS SENSORES =====
  uint16_t posicaoLinha = qtr.readLineBlack(sensorValues);

  // ===== 2. DETECÇÃO DE PERDA DE LINHA =====
  // Se todos os sensores leram muito alto, o robô saiu da linha
  bool perdeuLinha = true;
  for (uint8_t i = 0; i < SensorCount; i++) {
    if (sensorValues[i] > 200) { // threshold: sensor vê linha
      perdeuLinha = false;
      break;
    }
  }

  if (perdeuLinha) {
    // Estratégia: usa o último erro para tentar recuperar
    // (gira para o lado onde estava a linha)
    int recuperacao = (erroAnterior > 0) ? 80 : -80;
    moverMotores(
      VELOCIDADE_BASE - recuperacao,
      VELOCIDADE_BASE + recuperacao
    );
    return; // Pula o resto do loop até achar a linha
  }

  // ===== 3. CÁLCULO DO PID =====
  int erro       = posicaoLinha - SETPOINT;
  int derivativo = erro - erroAnterior;

  // Integrador com anti-windup
  integrador += erro;
  integrador = constrain(integrador, -integrador_max, integrador_max);

  float ajuste = (KP * erro) + (KD * derivativo) + (KI * integrador);

  // ===== 4. APLICA VELOCIDADES =====
  // O ajuste agora tem range independente da velocidade base
  int velEsq = constrain((int)(VELOCIDADE_BASE + ajuste), VELOCIDADE_MIN, VELOCIDADE_MAX);
  int velDir = constrain((int)(VELOCIDADE_BASE - ajuste), VELOCIDADE_MIN, VELOCIDADE_MAX);

  moverMotores(velEsq, velDir);

  // ===== 5. ATUALIZA HISTÓRICO =====
  erroAnterior = erro;

  // ===== 6. DEBUG — só a cada N ciclos =====
  debugSerial(erro, (int)ajuste, velEsq, velDir);
}

// =============================================================
//  FUNÇÕES AUXILIARES
// =============================================================

// --- Calibração controlada com feedback visual ---
void calibrar() {
  Serial.println(F("Calibrando... mova o robo sobre a linha!"));
  digitalWrite(LED_BUILTIN, HIGH);

  // 400 amostras ~= 4 segundos de calibração
  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
    // Pisca LED durante calibração para confirmar que está rodando
    if (i % 50 == 0) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      Serial.print(F("."));
    }
  }

  Serial.println();
  Serial.println(F("Valores minimos calibrados:"));
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(F(" "));
  }
  Serial.println();
  Serial.println(F("Valores maximos calibrados:"));
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(F(" "));
  }
  Serial.println();
}

// --- Controle dos motores (frente apenas por enquanto) ---
void moverMotores(int velEsq, int velDir) {
  // Motor Esquerdo (A) — frente
  digitalWrite(MOTOR_A_DIR, LOW);
  analogWrite(MOTOR_A_PWM, velEsq);

  // Motor Direito (B) — frente
  digitalWrite(MOTOR_B_DIR, LOW);
  analogWrite(MOTOR_B_PWM, velDir);
}

// --- Para ambos os motores ---
void pararMotores() {
  analogWrite(MOTOR_A_PWM, 0);
  analogWrite(MOTOR_B_PWM, 0);
  digitalWrite(MOTOR_A_DIR, LOW);
  digitalWrite(MOTOR_B_DIR, LOW);
}

// --- Debug serial cadenciado ---
void debugSerial(int erro, int ajuste, int velEsq, int velDir) {
  if (!DEBUG_SERIAL) return;
  if (cicloAtual % DEBUG_INTERVALO != 0) return;

  Serial.print(F("Err:"));    Serial.print(erro);
  Serial.print(F(" Aj:"));    Serial.print(ajuste);
  Serial.print(F(" Esq:"));   Serial.print(velEsq);
  Serial.print(F(" Dir:"));   Serial.print(velDir);
  Serial.print(F(" Ciclo:")); Serial.println(cicloAtual);
}
