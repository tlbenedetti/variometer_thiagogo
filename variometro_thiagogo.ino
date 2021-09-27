#include <Wire.h>                   // Biblioteca de interface I2C

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>                     // Biblioteca Piezo Tone
#include "U8glib.h"

#define tone_out1 8
#define tone_out2 9


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send AC
Adafruit_BMP280 sensor_bmp;

short speaker_pin1 = 8;                //arduino speaker output -
short speaker_pin2 = 9;                //arduino speaker output +

float vario_down = -1.1;               // Definir a redução
float vario_up; 
float alt[51];
float tim[51];
float beep;
float Beep_period;
float mux;

float Alt0;                 // Altura zero, no momento da inclusão +!
float vario=0;

float Altitude = 0;
float Temperature = 0;
unsigned long bounseInput4P = 0UL;
unsigned long time;

unsigned char samples=10;
unsigned char maxsamples=50;
unsigned char countPressVal = 0;
unsigned char tela = 0;

bool tmp1 = 0;
bool countPress = 0;
bool bounseInput4S = 0;
bool bounseInput4O = 0;

//relogio
unsigned char relogio=0;
unsigned char segundo = 0, minuto = 0, hora = 0;
unsigned long UtlTime;
unsigned long lastMillis;

boolean  thermalling = false;

void play_welcome_beep()                 // Saudações de áudio
{
    for (int aa=100;aa<=800;aa=aa+100)
    {
        tone(tone_out1,aa,200);       
        tone(tone_out2,aa+3,200);
        delay(50);
    }
}

void draw(void) {
  
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on.
  u8g.setFont(u8g_font_8x13Br); //para o alfabeto completo com caracteres tirar o r do font_8x13Br
  u8g.setColorIndex(0);
  u8g.setColorIndex(1);

  unsigned char posVar = 0;
  unsigned char posAlt = 0;
//temperatura

    u8g.setPrintPos(0, 14);
    u8g.print(Temperature,1);
    u8g.drawStr( 35, 14, "C"); //acentos http://www.ascii-code.com/


//caixa e m/s vario

    u8g.drawFrame(50,0,78,35); //quadrado vario
    u8g.drawStr(103, 30, "M\057s"); //m/s

    u8g.drawStr(120, 64, "M"); //M altimetro
    
    
//sensibilidade e relogio
  if(relogio==1){
    u8g.drawStr(0, 30, "v");
    u8g.setPrintPos(10, 30);
    u8g.print(mux);
  }
  else{
    String timex = "";
    if(hora < 10){
    timex += "0";
    }
    timex += hora;
     if (segundo%2==0)
        {
    timex += ":";
        }
        else{
    timex += " "; 
        }
    if(minuto < 10){
    timex += "0";
    }
    timex += minuto;
    
    u8g.setPrintPos(0, 34);
    u8g.print(timex);
  }


//altimetro
if (Altitude >= 1000)
    {
      posAlt = 28;
    }
    else
    {
      posAlt = 48;
    }
    u8g.setFont(u8g_font_fub20n); //se for so numeros usar o n no fim u8g.setFont(u8g_font_fub20n);
    u8g.setPrintPos(posAlt, 64);
    u8g.print(Altitude, 1);

    //vario
    if (vario < 0)
    {
      posVar = 52;
      if(vario < -0.1){
      u8g.drawTriangle(113,15, 122,7, 105,7);
      }
    }
    else
    {
      posVar = 61;
      if(vario > 0.1){
      u8g.drawTriangle(114,6, 124,15, 104,15);
      }
    }
    u8g.setPrintPos(posVar, 27);
    u8g.print(vario,1);


  //bateria
    int bateria = analogRead(A0);

    u8g.drawFrame(4,42,4,2);
    u8g.drawFrame(2,44,8,18);
    if(bateria > 716){
     u8g.drawStr( 1, 60, "."); 
    }
    if(bateria > 770){
     u8g.drawStr( 1, 55, "."); 
    }
    if(bateria > 830){
     u8g.drawStr( 1, 50, "."); 
    }

// conta para conversão voltagem*1024:5= valor arduino analog
/*
 * usar divisor de tensão vcc--10k -A0-100K--gnd
 */
    
  }

void setup()
{
    Wire.begin();                   // Inicializa i2c
    pinMode(4, INPUT);
    digitalWrite(4, HIGH);
    bounseInput4O =  digitalRead(4);// Resistor Pullup
    pinMode(tone_out1, OUTPUT);  // Dinâmica pin8 de saída -
    pinMode(tone_out2, OUTPUT);  // Speaker pin9 saída +
    sensor_bmp.begin(); // Sensibilidade do sensor de pressão
    play_welcome_beep();
}

void loop(void)
{

    //oled display

  u8g.firstPage();  


    bool  bounceTmp =  (digitalRead (4));
    
    if (bounseInput4S)          // Bounce protecção
    {
        if (millis() >= (bounseInput4P + 40))
        {bounseInput4O= bounceTmp; bounseInput4S=0;}
    }
    else{
        if (bounceTmp != bounseInput4O )
        {bounseInput4S=1; bounseInput4P = millis();}
    }
    if (!(bounseInput4O))
    {
        if (! countPress)
        {
            countPressVal++;
            
//mostra relogio e variaçaõ
lastMillis = millis();
relogio = 1;
            
            for (int i = 0; i < countPressVal; i++)  // Perder o número do menu
            {
                tone(tone_out2,1800,40); 
                delay(80);                
            }
            countPress = 1;
        }   
    }
    else{
        countPress=0;
    }
    if (countPressVal < 0 ) countPressVal = 0;
    if (tmp1) countPressVal = 0;            // == MENU SENSIBILIDADE PARA LEVANTAR ==
    if((countPressVal) == 0) {mux = 0.3;}  // 1 sinal
    if((countPressVal) == 1) {mux = 0.5;} // 2 sinal
    if((countPressVal) == 2) {mux = 0.8;}  // 3 sinal
    tmp1 =  countPressVal  >=  3;
    vario_up = mux;

    //mostra relogio e variaçaõ
            if (lastMillis > 0 && (millis() - lastMillis > 3000))
        {
                relogio = 0;
                lastMillis = 0;        
         }
    
    float tempo=millis();
    float N1=0;
    float N2=0;
    float N3=0;
    float D1=0;
    float D2=0;
    Altitude = (sensor_bmp.readAltitude(1013.25));
    Temperature = (sensor_bmp.readTemperature());
   
    
        
    for(int cc=1;cc<=maxsamples;cc++){                                   // averager
        alt[(cc-1)]=alt[cc];
        tim[(cc-1)]=tim[cc];
    };
    alt[maxsamples]=Altitude;
    tim[maxsamples]=tempo;
    float stime=tim[maxsamples-samples];
    for(int cc=(maxsamples-samples);cc<maxsamples;cc++){
        N1+=(tim[cc]-stime)*alt[cc];
        N2+=(tim[cc]-stime);
        N3+=(alt[cc]);
        D1+=(tim[cc]-stime)*(tim[cc]-stime);
        D2+=(tim[cc]-stime);
    };
    
    vario=1000*((samples*N1)-N2*N3)/(samples*D1-D2*D2); // Cálculo de som

    if ((tempo-beep)>Beep_period)
    {
        beep=tempo;
        if (vario>vario_up && vario<15 )
        {
            Beep_period=350-(vario*5);
            tone(tone_out1,(1000+(100*vario)),300-(vario*5)); // Som em ascensão
            tone(tone_out2,(1003+(100*vario)),300-(vario*5));
            thermalling = true;
        }
        else if ((vario < 0 ) && (thermalling == true))
        {
            thermalling = false; 
         // tone_out2.play(200, 800); Predpotok // Som (é opcional)
            
        }
        else if (vario< vario_down){         // Som a afundar
            Beep_period=200;
            tone(tone_out1,(300-(vario)),340);
            tone(tone_out2,(303-(vario)),340);
            thermalling = false;
        }
    }

//relogio
if(millis()-UtlTime<0)   
{     
UtlTime=millis();   
}   
else{     
segundo=int((millis()-UtlTime)/1000);   
}   
if(segundo>59)   
{     
segundo=0;     
minuto++;     
UtlTime=millis();     
if(minuto>59)     
{       
hora++;       
minuto=0;
}
}


      do {
    draw();
  } while( u8g.nextPage() );
    //fim oled display
}
