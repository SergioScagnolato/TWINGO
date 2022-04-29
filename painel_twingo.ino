
// INCLUIR BIBLIOTECA LiquidCrystal na IDE do arduino
// ir em SKETCH -> INCLUIR BIBLIOTECA -> GERENCIAR BIBLIOTECAS

// NOME TROCAR NA LINHA 123
// LISTA DE TAGS AUTORIZADOS NA LINHA 278

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

//Carrega a biblioteca do RTC DS1307
#include <DS1307.h>
//Modulo RTC DS1307 ligado as portas A4 e A5 do Arduino 
DS1307 rtc(A2, A3);

#include <SoftwareSerial.h>
#include <RDM6300.h>

//Inicializa a serial nos pinos 2 (RX) e 3 (TX)
SoftwareSerial RFID(3, 4);

int Led = 13;
uint8_t Payload[6]; // used for read comparisons

RDM6300 RDM6300(Payload);

const int LM35 = A0;
float temperatura,rpm,tick=0,maxtemp=0;
int x,sinal=0;
int inverterpm=0;

unsigned long prevtime=0,intervalo=1000,up_intervalo=1000;

//Array simbolo grau
byte grau[8] ={ B00001100,
                B00010010,
                B00010010,
                B00001100,
                B00000000,
                B00000000,
                B00000000,
                B00000000,};
                
/*
byte barra[8]={ B01110000,
                B11111000,               
                B11111100,
                B11111110,
                B11111110,
                B11111100,
                B11111000,
                B01110000,};
*/
byte barra[8]={ B01111110,
                B01111110,               
                B01111110,
                B11111110,
                B01111110,
                B01111110,
                B01111110,
                B01111110,};

byte shifrbr[8]={ B00000100,
                  B00001110,
                  B00011111,
                  B00000100,
                  B00000100,
                  B00000100,
                  B00000100,
                  B00000000,};

byte shifrpe[8]={ B11111011,
                  B11110001,
                  B11100000,
                  B11111011,
                  B11111011,
                  B11111011,
                  B11111011,
                  B11111111,};


                  
/*                
// Ícone de temperatura.
byte termo[8] = {
        B01000,
        B01110,
        B01000,
        B01110,
        B01000,
        B01110,
        B01100,
        B01110
};
*/                


//Sensor de tensão//
float Vo = 0.0, Vi = 0.0;
float value_aux = 0, value = 0;
//Valores dos Resistores//
float R1 = 99900.0; //Resistência R1 (100K) 
float R2 = 10500.0; //Resistência R2 (10K) 

void setup() {
 rtc.halt(false);
 pinMode(Led, OUTPUT);
 //Inicializa a serial para o leitor RDM6300
 RFID.begin(9600);
 lcd.begin(20,4);
 lcd.clear();
 
 lcd.setCursor(7,0);
 lcd.write("Twingo");
 lcd.setCursor(6,1);
 lcd.write("BRJ 3002");
 lcd.setCursor(4,2);
 lcd.write("Painel V1.21");
 lcd.setCursor(1,3);        // <-- AQUI POSICIONA O NOME PARA SAIR CENTRALIZADO (COLUNA, LINHA)
 lcd.write("**NOME**");     // < -- COLOQUE AQUI O NOME QUE VAI APARECER NA INICIALIZACAO ATE 20 CARACTERES
  
 pinMode(A1,INPUT);
 pinMode(2,INPUT_PULLUP);
 pinMode(3, INPUT_PULLUP);
 pinMode(5,OUTPUT);
 digitalWrite(5,HIGH);
 //Serial.begin(9600); // inicializa a comunicação serial
 //Cria o caractere customizado com o simbolo do grau
 lcd.createChar(0, grau);
 lcd.createChar(1, barra);
// lcd.createChar(2, shifrbr);
// lcd.createChar(3, shifrpe);
// lcd.createChar(2, termo);
 delay(2000);
 lcd.clear();

/****************************************/
// *****Ajusta o relogio *****
// rtc.setDOW(TUESDAY);      //Define o dia da semana
// rtc.setTime(14,48, 0);     //Define o horario
// rtc.setDate(21, 05, 2019);   //Define o dia, mes e ano
/***********************************************/

 //Definicoes do pino SQW/Out
 rtc.setSQWRate(SQW_RATE_1);
 rtc.enableSQW(true);
}

 //variaveis cronometro
int up_horas=0;
int up_minutos=0;
int up_segundos=0;
int up_atualiza=1;
unsigned long up_currtime=millis();   // cronometro

char leitura[15];
String numerofinal;
int leu=0;



void loop() {

temperatura=0; 
for(x=0;x<5;x++) {
 temperatura += ((float(analogRead(LM35))*5/(1023))/0.01);
}
temperatura=temperatura/5.0; 
if(temperatura>maxtemp) maxtemp=temperatura;
lcd.setCursor(0,0);
//lcd.print("Temp: ");
char buff[12];
sprintf(buff,"%03d",(int)temperatura);
//lcd.write((byte)2);
lcd.print(buff);
lcd.write((byte)0);
lcd.print("C");

//lcd.setCursor(7,0);
//lcd.print("Max ");
//sprintf(buff,"(%03d",(int)maxtemp);
//lcd.print(buff);
//lcd.write((byte)0);
//lcd.print("C)");

/*
// tensao bateria
lcd.setCursor(13,0);
value=0;
for(x=0;x<5;x++){
value_aux = analogRead(A1);
value += value_aux;
}
value = value/5.0;
Vo = (value * 5.0) / 1023.0;
Vi = Vo / (R2/(R1+R2));
if (Vi<0.09) {Vi=0.0;}
sprintf(buff,"%03.1f",Vi);
lcd.print(Vi);
lcd.print(" V");
*/

// cronometro
lcd.setCursor(12,0);
if(up_atualiza==1) {
 char buff[12];
 sprintf(buff,"%02d",(int)up_horas);
 lcd.print(buff);
 lcd.print(":");
 sprintf(buff,"%02d",(int)up_minutos);
 lcd.print(buff);
 lcd.print(":");
 sprintf(buff,"%02d",(int)up_segundos);
 lcd.print(buff);
 up_atualiza=0;
} 
if(millis() - up_currtime >= up_intervalo) {
  up_segundos++;
  if(up_segundos>59) { up_segundos=0;up_minutos++;}
  if(up_minutos>59) { up_segundos=0;up_minutos=0;up_horas++;}
  up_atualiza=1;
  unsigned long up_currtime=millis();   // cronometro
}

/**** HORAS E DATA *****/
lcd.setCursor(0,1);
lcd.print(rtc.getTimeStr(FORMAT_SHORT));
lcd.print("  ");

lcd.print(rtc.getDateStr(FORMAT_SHORT));
lcd.print("  ");
if(rtc.getDOWStr()=="Sunday")
 lcd.print("Dom");
if(rtc.getDOWStr()=="Monday")
 lcd.print("Seg");
if(rtc.getDOWStr()=="Tuesday")
 lcd.print("Ter");
if(rtc.getDOWStr()=="Wednesday")
 lcd.print("Qua");
rdm lcd.print("Qui");
if(rtc.getDOWStr()=="Friday")
 lcd.print("Sex");
if(rtc.getDOWStr()=="Saturday")
 lcd.print("Sab");
 
//lcd.print(" ");
//lcd.print(rtc.getDOWStr(FORMAT_SHORT));

if(leu==0) {
 while(1) {
  lcd.setCursor(0,2);
  lcd.print("** Aproxime o TAG **");
  lcd.setCursor(0,3);lcd.print("              ");
 // lcd.setCursor(0,4);lcd.print("             ");
  while (RFID.available() > 0)  {
   digitalWrite(Led, HIGH);
   uint8_t c = RFID.read();
   if (RDM6300.decode(c))  {
   //  Serial.print("ID TAG: ");
    lcd.setCursor(0,2);
    lcd.print("ID: ");
   //Mostra os dados no serial monitor
   //  for (int i = 0; i < 5; i++) {
   //      Serial.print(Payload[i], HEX);
   //      Serial.print(" ");
   //    }
   //    Serial.println(); 

    sprintf(leitura,"%02X%02X%02X%02X%02X",Payload[0],Payload[1],Payload[2],Payload[3],Payload[4]);
    numerofinal=leitura;
    lcd.print(numerofinal);
    lcd.print("      ");
    lcd.setCursor(0,3);
    if(numerofinal=="41003CEE8A") {  //  || numerofinal=="41003C7B3B") {      // <-- LISTA DE TAGS AUTORIZADOS A LIGAR
     lcd.print("TAG AUTORIZADO");
     leu=1;
    } else {
     lcd.print("TAG REJEITADO");
     leu=0;
     delay(1500);
    } 
   }
  }
  digitalWrite(Led, LOW);
  if(leu==0) continue;
  break;
 }
}
digitalWrite(5,LOW);
if(leu==1) {
 delay(1500);
 lcd.setCursor(0,3);lcd.print("              ");
}
leu=2;
//lcd.setCursor(0,4);lcd.print("             ");
lcd.setCursor(0,3);
tick=0;x=0;
unsigned long currtime=millis();
while(millis() -currtime <= intervalo) {
// sinal=digitalRead(3);
 //if(analogRead(A1)>1010) {tick++; }
 if(digitalRead(2)==LOW and x==0) {tick++; x=1;}
 if(digitalRead(2)==HIGH) x=0;
}

//lcd.setCursor(19,3);lcd.print("0");


rpm=(tick/4)*60;    // 4 pulsos no neg da bobina a cada volta
sprintf(buff,"%04d",(int)rpm);
lcd.print(buff);
lcd.print(" RPM");
lcd.print("            ");
lcd.setCursor(9,3);
x=(int)rpm/500;
for(sinal=0;sinal<x;sinal++) lcd.write((byte)1);

/*
if(rpm>=3500) {
 lcd.setCursor(19,3);
 if(inverterpm==1) {
  lcd.write((byte)2);
  inverterpm=0;
 } else {
  lcd.write((byte)3);
  inverterpm=1;
 }
// lcd.setCursor(9,3);
} 
*/


/*
//velocimetro
float raio=28;   // raio em cm
tick=0;x=0;
currtime=millis();
while(millis() -currtime <= intervalo) {
 if(digitalRead(3)==LOW and x==0) {tick++; x=1;}
 if(digitalRead(3)==HIGH) x=0;
}
float velocidade=(raio/100) * 2 * 3.1415926 * tick;  //velocidade em m/s
velocidade=velocidade * 3.6;
if(velocidade<=3) velocidade=0;
lcd.setCursor(0,2);
lcd.print(velocidade,0);
lcd.print(" KM/h   ");
*/


delay(40);
}

//void conta() {
//  tick++;
//}
