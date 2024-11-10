#include <Arduino.h>
#include <Stepper.h>

#define STEPS_PER_REV 4096          // Passos para uma rotação completa
#define TARGET_ROTATIONS 2.86       // Número de rotações alvo
#define TARGET_STEPS (int)(STEPS_PER_REV * TARGET_ROTATIONS) // vai ter um excedentezinho

#define BUTTON_PIN 2//16                // Pino do botão (D0)
#define LED_GREEN  3//5                 // Pino do LED verde (D1)
#define LED_RED 4//4                  // Pino do LED vermelho (D2)
#define LED_YELLOW 5//0               // Pino de LED amarelo (D3)

// Define os pinos do motor conectados ao módulo ULN2003
#define IN1 6//2  // (D4)
#define IN2 7//14  // (D5)
#define IN3 8//12  // (D6)
#define IN4 9//13  // (D7)

Stepper motor = Stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

const unsigned long ROTATE_INTERVAL = 10000;     // Intervalo de 10 segundos para tentar a rotação
const unsigned long WINDOW_DURATION = 3000;      // Janela de 3 segundos para apertar o botão
const unsigned long DEBOUNCE_DELAY = 50;
unsigned long lastRotationTime = 0;              // Marca o último momento em que a rotação foi tentada

bool motorRunning = false;
bool waitingForButton = false;
int currentSteps = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  motor.setSpeed(10);  // Define a velocidade do motor (em RPM)

  Serial.begin(9600);
  Serial.println("Sistema pronto. Aguardando o tempo para iniciar a rotação.");
  digitalWrite(LED_GREEN, HIGH);
}

void loop() {
  unsigned long currentTime = millis();

  // Verifica se é hora de iniciar a rotação
  if (currentTime - lastRotationTime >= ROTATE_INTERVAL && !motorRunning && !waitingForButton) {
    Serial.println("Tentando iniciar a rotação. Pressione o botão para sucesso.");
    digitalWrite(LED_YELLOW, HIGH);
    waitingForButton = true;   // Sinaliza que estamos na janela de tempo de apertar o botão
    lastRotationTime = currentTime;  // Atualiza o tempo da última tentativa de rotação
  }

  // Verifica se o botão foi pressionado durante a janela de tempo
  if (waitingForButton && (currentTime - lastRotationTime <= WINDOW_DURATION)) {
    if (digitalRead(BUTTON_PIN) == HIGH) {
      Serial.println("Botão pressionado a tempo. Iniciando rotações.");
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, HIGH);   // Acende o LED verde
      delay(1000);                     // Mantém o LED aceso por 1 segundo
      digitalWrite(LED_GREEN, LOW);    // Apaga o LED verde
      waitingForButton = false;        // Fecha a janela de tempo para apertar o botão
      motorRunning = true;             // Inicia a rotação
      currentSteps = 0;                // Reseta a contagem de passos
    }
  }

  // Verifica se a janela de tempo expirou sem apertar o botão
  if (waitingForButton && (currentTime - lastRotationTime > WINDOW_DURATION)) {
    Serial.println("Falha: botão não pressionado a tempo.");
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);       // Acende o LED vermelho
    delay(1000);                       // Mantém o LED aceso por 1 segundo
    digitalWrite(LED_RED, LOW);        // Apaga o LED vermelho
    waitingForButton = false;          // Fecha a janela de tempo

  }

  if (motorRunning && currentSteps < TARGET_STEPS) {
    motor.step(1);      // Move o motor um passo
    currentSteps++;     // Incrementa a contagem de passos
  }

  // Para o motor quando a quantidade de passos alvo é atingida
  if (motorRunning && currentSteps >= TARGET_STEPS) {
    Serial.println("Rotações concluídas. Motor parado.");
    motorRunning = false;
    currentSteps = 0;
  }
}
