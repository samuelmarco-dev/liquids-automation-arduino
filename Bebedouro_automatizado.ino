#include <neotimer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <HCSR04.h>

#define TRIGGER_CIMA 4
#define ECHO_CIMA 5
#define TRIGGER_BAIXO 6
#define ECHO_BAIXO 7
#define BOMBA 8
#define BOTAO 11
#define ENTER 12

#define TIMER 500

#define COPO_CHEIO 7
#define COPO_VAZIO 25

UltraSonicDistanceSensor sensorDistanciaCima(TRIGGER_CIMA, ECHO_CIMA);
UltraSonicDistanceSensor sensorDistanciaBaixo(TRIGGER_BAIXO, ECHO_BAIXO);
LiquidCrystal_I2C lcd(0x27, 16, 4);

Neotimer serialFreq;
Neotimer sensorFreq;
Neotimer bombaFreq;
Neotimer aproximeFreq;
Neotimer opcaoFreq;
Neotimer semSelecaoFreq;
Neotimer comSelecaoFreq;

Neotimer timer = Neotimer(400);

float distanciaCima = 0;
float distanciaBaixo = 0;

int clickBotao = 0;          
int arrayTempo[5] = {
  2300, 4200, 6500, 8400, 12680
};
bool selecionado = false;

void setup() {
  // coloque seu código de configuração aqui, para executar uma vez:
  pinMode(BOTAO, INPUT);
  pinMode(ENTER, INPUT);
  pinMode(BOMBA, OUTPUT);
  digitalWrite(BOMBA, LOW);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  serialFreq.set(TIMER);
  sensorFreq.set(TIMER - 250);
  aproximeFreq.set(TIMER);
  opcaoFreq.set(TIMER);
  semSelecaoFreq.set(TIMER);
  comSelecaoFreq.set(TIMER);
  bombaFreq.set(0);
  timer.start();

  Serial.begin(9600);
}

void loop() {
  // coloque seu código principal aqui, para ser executado repetidamente:
  if(clickBotao == 0) bombaFreq.set(0); 
  else bombaFreq.set(arrayTempo[clickBotao - 1]);
  
  if(sensorFreq.repeat()) calcularDistancia();
  if(serialFreq.repeat()) iniciarDadosLcd();  

  if(timer.debounce(digitalRead(BOTAO))) lerClickBotao();
  if(timer.debounce(digitalRead(ENTER))) lerSelecionado();

  if(distanciaBaixo <= 0 || distanciaCima <= 0) {
    if(aproximeFreq.repeat()) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Distancia invalida!");
      lcd.setCursor(0, 2);
      lcd.print("Centralize o copo.");

      digitalWrite(BOMBA, LOW);
    }
  }

  if(verificarDistanciaBaixo() && verificarDistanciaCima()) {
    if(verificarClickBotao()) {
      if(selecionado) {
        digitalWrite(BOMBA, HIGH);

        if(comSelecaoFreq.repeat()) {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Opcao selecionada:");

          lcd.setCursor(5, 2);
          lcd.print(opcaoSelecionada(clickBotao));
        }
        
        if(bombaFreq.repeat()) {
          digitalWrite(BOMBA, LOW);
          clickBotao = 0;
          selecionado = false;
        }
      } 
      else { if(semSelecaoFreq.repeat()) nenhumaSelecao(); }
    } 
    else { if(semSelecaoFreq.repeat()) nenhumaSelecao(); } 
  } 
  else { 
    digitalWrite(BOMBA, LOW);
    if(aproximeFreq.repeat()) exibirAlertaTela(); 
    clickBotao = 0;
    selecionado = false;
    bombaFreq.reset();
  }
}

void iniciarDadosLcd() {
  Serial.println("Distancia copo:");
  Serial.println(distanciaBaixo);

  Serial.println("Distancia liquido:");
  Serial.println(distanciaCima);
}

void calcularDistancia() {
  distanciaBaixo = sensorDistanciaBaixo.measureDistanceCm();
  distanciaCima = sensorDistanciaCima.measureDistanceCm();
}

void exibirAlertaTela() {
  if(!verificarDistanciaBaixo() && !verificarDistanciaCima()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Nao identificado");
    return;
  }

  if(!verificarDistanciaBaixo()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Aproxime o copo!");
    lcd.setCursor(0, 2);
    lcd.print("Copo esta fora.");
    return;
  }
  if(!verificarDistanciaCima()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Retire o copo!");
    lcd.setCursor(0, 2);
    lcd.print("Copo esta cheio.");
    return;
  }
}

int verificarDistanciaBaixo() {
  return distanciaBaixo >= 3 && distanciaBaixo <= 6.4;
}

int verificarDistanciaCima() {
  return distanciaCima >= COPO_CHEIO && distanciaCima <= COPO_VAZIO;
}

void lerClickBotao() {
  clickBotao++;
  if(clickBotao > 5) clickBotao = 1;
}

void lerSelecionado() {
  if(clickBotao > 0) selecionado = true;
  else selecionado = false;
}

int verificarClickBotao() {
  return clickBotao >= 1 && clickBotao <= 5;
}  

String opcaoSelecionada(int clickBotao) {
  String text = "";

  switch(clickBotao) {
    case 1:
      return text = "100 ml";
      break;
    case 2:
      return text = "200 ml";
      break;
    case 3:
      return text = "300 ml";
      break;
    case 4:
      return text = "400 ml";
      break;
    case 5:
      return text = "500 ml";
      break;
    default:
      return text; 
      break;
  }
}

void nenhumaSelecao() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Selecione opcao:");

  if(!clickBotao) {
    lcd.setCursor(0,2);
    lcd.print("Aperte selecao");
  } else {
    lcd.setCursor(5, 2);
    lcd.print(opcaoSelecionada(clickBotao));
  }
}
