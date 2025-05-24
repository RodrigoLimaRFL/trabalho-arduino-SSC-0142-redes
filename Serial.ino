
#define PINO_RTS 11 // RTS
#define PINO_RX 12 // Mudei pra 12, tava em 13, CTS
#define PINO_TX 13 // Dados
#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)

#include "Temporizador.h"

volatile byte dadoAtual = 0; // byte que armazena o dado atual
bool bitAtual = false; // bit atual
int bitsEnviados = 0; // num de bits enviados
bool paridade = false; // bit de paridade
bool enviando = false; // flag de envio

// Calcula bit de paridade - Par
bool bitParidade(char dado){
  int count = 0;

  for(int i = 0; i < 8; i++){
    if(dado & (1 << i)){
      count++; // conta o num de bits 1
    }
  }

  return !(count % 2 == 0); // retorna true se o num de bits 1 for par
}

// Rotina de interrupcao do timer1
// O que fazer toda vez que 1s passou?
ISR(TIMER1_COMPA_vect){
  if(bitsEnviados < 8) 
  {
    // Calcula o bit a ser enviado
    bitAtual = dadoAtual & (1 << (7 - bitsEnviados));

    digitalWrite(PINO_TX, bitAtual); // Envia o bit 
    Serial.print("Bit Enviado: ");
    Serial.println(bitAtual, BIN);

    bitsEnviados++;
  }
  else
  {
    digitalWrite(PINO_TX, paridade); // Envia o bit de paridade
    Serial.print(" Bit Paridade Enviado: ");
    Serial.println(paridade, BIN);

    digitalWrite(PINO_RTS, LOW); // RTS
    Serial.println("   Enviando RTS como 0");

    // Reseta variaveis
    bitsEnviados = 0; 
    enviando = false;
    paraTemporizador();
  }
}

// Executada uma vez quando o Arduino reseta
void setup(){
  //desabilita interrupcoes
  noInterrupts();
  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);
  // Inicializa TX ou RX
  pinMode(PINO_TX, OUTPUT);
  pinMode(PINO_RTS, OUTPUT);
  pinMode(PINO_RX, INPUT);

  digitalWrite(PINO_TX, LOW);
  digitalWrite(PINO_RTS, LOW);
  digitalWrite(PINO_RX, LOW);
  // Configura timer
  configuraTemporizador(BAUD_RATE);
  // habilita interrupcoes
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop () {
  if (!enviando && Serial.available() > 0) {
    // Lê o dado do Serial
    char dado = Serial.read();
    paridade = bitParidade(dado);
    dadoAtual = dado;

    Serial.print("Dado: ");
    Serial.println(dado, BIN);

    Serial.print(" Dado Atual: ");
    Serial.println(dadoAtual, BIN);

    Serial.print("  Paridade: ");
    Serial.println(paridade, BIN);

    digitalWrite(PINO_RTS, HIGH); // RTS
    Serial.println("   Enviando RTS como 1");
  }

  if(digitalRead(PINO_RX) == HIGH)
  {
    if(!enviando)
    {
      // Inicia o temporizador quando o CTS estiver em HIGH  
      enviando = true;
      iniciaTemporizador();
      Serial.println("Handshake recebido, enviando dados");
    }
    // Aguarda o tempo de espera
    delay(HALF_BAUD);
  }
}
