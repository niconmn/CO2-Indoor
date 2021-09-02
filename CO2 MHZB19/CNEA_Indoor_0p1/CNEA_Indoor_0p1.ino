/******************************************************************************/
// CNEA_Indoor_0p1
// Para Utilizar en NANO Experimental!!!
// Lectura MH-Z19
// 
// Faltantes:
// 			- RTC
// 			- SD 
// 
//*****************************************************************************/

#include <MHZ19_uart.h> // incluye librería para manejo del sensor de CO2
#include <Wire.h> 
#include <LiquidCrystal.h>
//#include <LiquidCrystal_I2C.h>
#include <TimeLib.h> // RTC interno Arduino
#include <PString.h>

//LiquidCrystal_I2C display(0x27,16,2);
LiquidCrystal lcd(12, 11,6 ,5, 4, 3, 2);
char LCD[81] =  "CO2:                " // 0-19 Primera línea
                "                    " // 20-39 Segunda línea
                "                    " // 40-59 Tercera línea
                "                    ";// 60-79 Cuarta línea

int LCDPtr = 0; //Puntero del array del LCD. Va de 0 a 79
char AuxStr[11] = "          "; // Array auxiliar para PString
int  AuxStrPtr = 0; //Puntero auxiliar para PString. Va de 0 a 10
int temp = 0;
int aux = 0;

const int rx1_pin = 7;  //Serial rx pin MH-Z19
const int tx1_pin = 8; //Serial tx pin MH-Z19

const int buzzer = 9 ;
int cnt = 0; // cuenta LOOPS

String partPormill = "ppm";
String riesgoAlto = "RIESGO ALTO";
String riesgoMedio = "RIESGO MEDIO";
String riesgoBajo =   "RIESGO BAJO";

const int led_R = A5 ; // control LED ROJO
const int led_V = A4 ; // control LED VERDE
const int led_A = 13 ; // control LED ROJO Onboard

const int cal_pin = A0;  // entrada pulsador calibración

MHZ19_uart mhz19_1; // asigna medidor al sensor
int datoMHZ19 = 0;
// Temporización de loop rápido y loop lento 
int Delta = 500;
int Delta_millis;
int millis_Ant;
int millis_Now;


/*******************************************************************************
Calibrar MH-Z19
*******************************************************************************/
void calibrar()
{
  const int waitingMinutes = 30;  //cantidad de minutos a esperar
  const long waitingSeconds = waitingMinutes * 60L; // cantidad de segundos a esperar
  long cnt_cal = 0; // cuenta segundos de calibración

  lcd.clear(); // borra pantalla  
  lcd.setCursor(0,0);
  lcd.print("CALIBRACION"); //
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("DEMORA 30 MIN");    // Escribe texto
  delay(30000); // Espera 30 segundos

  lcd.clear(); // borra pantalla  
  lcd.setCursor(0,0);
  lcd.print("PONER EL EQUIPO"); //
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("AL AIRE LIBRE");    // Escribe texto
  delay(30000); // Espera 30 segundos

  lcd.clear(); // borra pantalla  
  lcd.setCursor(0,0);
  lcd.print("COMIENZA"); //
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("CALIBRACION"); //
  delay(10000); // Espera 10 segundos
   while (cnt_cal <= waitingSeconds) { // espera media hora
    if ( ++cnt_cal % 60 == 0) { // Si cnt es múltiplo de 60
      lcd.clear(); // borra pantalla  
      lcd.setCursor(0,0);
      lcd.print(cnt_cal / 60); // Cada minuto muestra el valor
      lcd.setCursor(7,0);
      lcd.print("minutos"); // 
      lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
      lcd.print("co2: ");    // Escribe texto
      lcd.setCursor(8, 1); // Ubicamos el cursor en la novena posición(columna:8) de la segunda línea(fila:1)
      lcd.print(mhz19_1.getPPM()); // Escribe CO2
      lcd.setCursor(12, 1); // Ubicamos el cursor en la treceava posición(columna:12) de la segunda línea(fila:1)
      lcd.print("ppm"); // Escribe texto
    } else {
      lcd.clear(); // borra pantalla  
      lcd.setCursor(0,0);
      lcd.print("CALIBRANDO"); //
      lcd.setCursor(0,1);
      lcd.print(cnt_cal); // muestra los segundos transcurridos
      lcd.setCursor(7,1);
      lcd.print("segundos");    
    }
    delay(1000); // Espera 1 segundo
  }
  // paso media hora

  lcd.clear(); // borra pantalla  
  mhz19_1.calibrateZero(); // Calibra

  lcd.setCursor(0,0);
  lcd.print("PRIMERA"); //
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("CALIBRACION");    // Escribe texto
  delay(60000); // Espera 60 segundos

  lcd.clear(); // borra pantalla  
  mhz19_1.calibrateZero();  // Calibra por segunda vez por las dudas
  
  lcd.setCursor(0,0);
  lcd.print("SEGUNDA"); //
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("CALIBRACION");    // Escribe texto
  delay(10000); // Espera 10 segundos 

  lcd.clear(); // borra pantalla  
  lcd.setCursor(0,0);
  lcd.print("CALIBRACION");    // Escribe texto
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("TERMINADA");    // Escribe texto
  delay(10000); // Espera 10 segundos 

}
/*******************************************************************************
leerMHZ19
*******************************************************************************/
void leerMHZ19()
{
	datoMHZ19 = mhz19_1.getPPM();//
	PString mystring(AuxStr, sizeof(AuxStr)); //zona sin mucho entendimiento, pero anda.
	mystring = datoMHZ19; //mando un Pstring "mystring" con el valor medido

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
		LCD[temp] = partPormill[AuxStrPtr];
		AuxStrPtr++;
		temp++;
	}
}
/******************************************************************************
semaforo: Manejo alertas, led y texto en pantalla
*******************************************************************************/
void semaforo(void)
{
	if (datoMHZ19 < 700){
		digitalWrite(led_V, HIGH);
		digitalWrite(led_R, LOW);
		AuxStrPtr=0;
		temp = 40;
		while (AuxStrPtr < 11){ //desde 0 a la longitud de mystring (que es el valor del ADC)
			LCD[temp] = riesgoBajo[AuxStrPtr];
			AuxStrPtr++;
			temp++;		
		}
    }
	else{
		digitalWrite(led_R, HIGH);
		digitalWrite(led_V, LOW);
		AuxStrPtr=0;
		temp = 40;
		while (AuxStrPtr < 11){ //desde 0 a la longitud de mystring (que es el valor del ADC)
			LCD[temp] = riesgoAlto[AuxStrPtr];
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
	Serial.print(datoMHZ19);
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
/*******************************************************************************
Scroll Inicio
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
	pinMode(led_R, OUTPUT);  //Inicia LED ROJO
	pinMode(led_V, OUTPUT);  //Inicia LED VERDE
	pinMode(led_A, OUTPUT);  //Inicia LED AZUL
	pinMode(cal_pin, INPUT_PULLUP); // entrada pulsado para calibrar, seteada como pulluppara poder conectar pulsador sin poenr resistencia adicional
	Serial.begin(9600);
	lcd.begin(20,4);
	lcd.clear(); // borra pantalla
	scrollInicio();
	lcd.clear(); // borra pantalla
	lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
	lcd.print("INICIANDO");
	delay(1000);
	lcd.clear(); // borra pantalla
	mhz19_1.begin(rx1_pin, tx1_pin); // inicializa el sensor 1
	/*   MH-Z19 has automatic calibration procedure. the MH-Z19 executing automatic calibration, its do zero point(stable gas environment (400ppm)) judgement.
	  The automatic calibration cycle is every 24 hours after powered on.  
	  If you use this sensor in door, you execute `setAutoCalibration(false)`.   */  
	mhz19_1.setAutoCalibration(false);  
	lcd.setCursor(0, 0); // Ubicamos el cursor en la primera posición(columna:0) de la primera línea(fila:0)
	lcd.print("Warm up");    // Escribe primera linea del cartel
	delay(60000); // espera 1 minuto            
	lcd.clear(); // borra pantalla  
}
/*******************************************************************************
Loop
*******************************************************************************/
void loop() 
{
	millis_Now = millis();
	Delta_millis = millis_Now - millis_Ant;
	LCD_Refresh();
	if (digitalRead(cal_pin) == LOW) 
	{ // si detecta el botón de calibrar apretado, calibra
		calibrar();
	}
	if(Delta_millis >= Delta){// Loop lento (500ms)
		millis_Ant = millis_Now;
		leerMHZ19();
		semaforo();
		printSerial();
	}
}