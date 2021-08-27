/******************************************************************************/
// CO-Dos
// Para Utilizar en NANO Experimental!!!
// EE850 conectado al J5 pin 2 entrada analogica A6.
// 
//*****************************************************************************/

#include <LiquidCrystal.h>
#include <TimeLib.h> // RTC interno Arduino
#include <LiquidCrystal.h>
#include <PString.h>
LiquidCrystal lcd(12, 11,6 ,5, 4, 3, 2);
char LCD[81] =  "CO2:                " // 0-19 Primera línea
                "                    " // 20-39 Segunda línea
                "                    " // 40-59 Tercera línea
                "                    ";// 60-79 Cuarta línea
int LCDPtr = 0; //Puntero del array del LCD. Va de 0 a 79
int temp = 0; // Temporal
int aux = 0;
int multPpm = 1000;
char AuxStr[11] = "          "; // Array auxiliar para PString
int  AuxStrPtr = 0; //Puntero auxiliar para PString. Va de 0 a 10
float cteAdc = 0.004887585;
int adc1 = A6;
int adc2 = A7;
int lecturaEE850G = 0;
int lcdth =0;
String partpormill = "ppm";
String dangerzone = "RIESGO ALTO";
String safezone =   "RIESGO BAJO";
//
int LedWhite = 13; // Led blanco en pcb Arduino Nano
int Ledrojo = A5; // Led rojo (led bicolor)
int Ledverde = A4; // Led verde (led bicolor)
int buzzer = 9; // Buzzer
// Temporización de loop rápido y loop lento 
int Delta = 500;
int Delta_millis;
int millis_Ant;
int millis_Now;

/*******************************************************************************
leerADC: LECTURA ADC (J5)
*******************************************************************************/
void leerADC()
{
	lecturaEE850G = analogRead(adc1)* cteAdc * multPpm;// + 200 offset del sensor = [ppm]
	PString mystring(AuxStr, sizeof(AuxStr)); //zona sin mucho entendimiento, pero anda.
	mystring = lecturaEE850G; //mando un Pstring "mystring" con el valor medido

	AuxStrPtr=0;
	temp = 4;
	while (AuxStrPtr < mystring.length()){ //desde 0 a la longitud de mystring (que es el valor del ADC)
		LCD[temp] = AuxStr[AuxStrPtr]; //pasa numero a numero el valor del ADC a la posicion correspondiete en el LCD
		AuxStrPtr++;
		temp++;
	}
	if (mystring.length()<4){//borro caracteres que puedan quedar de mediciones de 4 digitos.
		LCD[10]=' ';
	}
	AuxStrPtr=0;
	while (AuxStrPtr < 3){
		LCD[temp] = partpormill[AuxStrPtr];
		AuxStrPtr++;
		temp++;
	}
}

/******************************************************************************
semaforo: Manejo alertas, led y texto en pantalla
*******************************************************************************/
void semaforo(void)
{
	if (lecturaEE850G < 800){
		digitalWrite(Ledverde, HIGH);
		digitalWrite(Ledrojo, LOW);
		AuxStrPtr=0;
		temp = 40;
		while (AuxStrPtr < 11){ //desde 0 a la longitud de mystring (que es el valor del ADC)
			LCD[temp] = safezone[AuxStrPtr];
			AuxStrPtr++;
			temp++;		
		}
    }
	else{
		digitalWrite(Ledrojo, HIGH);
		digitalWrite(Ledverde, LOW);
		AuxStrPtr=0;
		temp = 40;
		while (AuxStrPtr < 11){ //desde 0 a la longitud de mystring (que es el valor del ADC)
			LCD[temp] = dangerzone[AuxStrPtr];
			AuxStrPtr++;
			temp++;		
		}		
    }
}

/******************************************************************************
printSerial: Impresiones por puerto serie
*******************************************************************************/
void printSerial(void)
{
	Serial.print("Concentración =");
	Serial.print(lecturaEE850G);
	Serial.println(" ppm");
	Serial.println(" ");
}
/*******************************************************************************
LCD_Refresh: Manejo del LCD
*******************************************************************************/
void LCD_Refresh()
{
	/* Escribo la fila 0 */
	if (LCDPtr < 20){
		lcd.setCursor(LCDPtr,0);
		lcd.print(LCD[LCDPtr]);
	};
	/* Escribo la fila 1 */
	if ((LCDPtr >= 20) && (LCDPtr < 40)){
		lcd.setCursor(LCDPtr - 20,1);
		lcd.print(LCD[LCDPtr]);
	};
	/* Escribo la fila 2 */
	if ((LCDPtr >= 40) && (LCDPtr < 60)){
		lcd.setCursor(LCDPtr-40,2);
		lcd.print(LCD[LCDPtr]);
	};
	/* Escribo la fila 3 */
	if ((LCDPtr >= 60) && (LCDPtr < 80)){
		lcd.setCursor(LCDPtr-60,3);
		lcd.print(LCD[LCDPtr]);
	};
	/* Incremento el puntero y si llegó a 80 lo reseteo */
	LCDPtr = LCDPtr + 1;
	if (LCDPtr == 80){
		LCDPtr = 0;
	};
}

/******************************************************************************
The_Pac_Sound: el sonido de arranque
*******************************************************************************/
void The_Pac_Sound(void)
{
	digitalWrite(buzzer, HIGH); // ON
	delay(75);
	digitalWrite(buzzer, LOW); // OFF
	delay(75);
	digitalWrite(buzzer, HIGH); // ON
	delay(75);
	digitalWrite(buzzer, LOW); // OFF
	delay(75);
	digitalWrite(buzzer, HIGH); // ON
	delay(150);
	digitalWrite(buzzer, LOW); // OFF
}
/******************************************************************************
Scrol Inicio
*******************************************************************************/
void scrollInicio(void)
{
	for (int i = 0; i < 20; i++) 
	{
		lcd.setCursor(i,3);
		lcd.print("*");
		lcd.setCursor(i,2);
		lcd.print("*");
		lcd.setCursor(i,1);
		lcd.print("*");
		lcd.setCursor(i,0);
		lcd.print("*");
		delay(100);
	}
}
/*******************************************************************************
Setup
*******************************************************************************/

void setup()
{
	pinMode(LedWhite, OUTPUT); //Defino como salida la pata que tiene el led blanco
	pinMode(Ledrojo, OUTPUT); //Defino como salida la pata que tiene el led rojo
	pinMode(Ledverde, OUTPUT); //Defino como salida la pata que tiene el led verde
	pinMode(buzzer, OUTPUT);
	Serial.begin(9600);
	lcd.begin(20, 4); // Inicializo el lcd de 20 x 4
	Serial.println("CO2 EE850");
	lcd.clear();
	scrollInicio();
	The_Pac_Sound();
	lcd.clear();
	lcd.setCursor(2,0);
	lcd.print("Detector de CO2!");
	delay(1000);
}

/*******************************************************************************
Loop
*******************************************************************************/
void loop()
{
	millis_Now = millis();
	Delta_millis = millis_Now - millis_Ant;
	LCD_Refresh();
	if(Delta_millis >= Delta){// Loop lento (500ms)
		millis_Ant = millis_Now;
		leerADC();
		semaforo();
		printSerial();
	}
}
