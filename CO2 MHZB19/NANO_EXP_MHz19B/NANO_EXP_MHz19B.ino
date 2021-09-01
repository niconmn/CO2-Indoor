
/*----------------------------------------------------------
    MH-Z19 CO2 sensor
  ----------------------------------------------------------*/
#include <MHZ19_uart.h> // incluye librería para manejo del sensor de CO2
#include <Wire.h> 
#include <LiquidCrystal.h>
//#include <LiquidCrystal_I2C.h>

//Crear el objeto display  dirección  0x27 y 16 columnas x 2 filas
//LiquidCrystal_I2C display(0x27,16,2);  //
LiquidCrystal lcd(12, 11,6 ,5, 4, 3, 2);


const int rx1_pin = 7;  //Serial rx pin MH-Z19_1
const int tx1_pin = 8; //Serial tx pin MH-Z19_1
const int rx2_pin = A1;//Serial rx pin MH-Z19_2
const int tx2_pin = A2;//Serial tx pin MH-Z19_2

const int buzzer = 9 ;
int cnt = 0; // cuenta LOOPS

//****************************************************************************************
const int led_R = A5 ; // control LED ROJO
const int led_V = A4 ; // control LED VERDE
const int led_A = 13 ; // control LED ROJO Onboard

const int cal_pin = A0;  // entrada pulsador calibración
//*****************************************************************************************

MHZ19_uart mhz19_1; // asigna medidor al sensor
MHZ19_uart mhz19_2; // asigna medidor al sensor

//----------------------------------------------------------
//CALIBRACION
//----------------------------------------------------------
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
  mhz19_2.calibrateZero(); // Calibra
  lcd.setCursor(0,0);
  lcd.print("PRIMERA"); //
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("CALIBRACION");    // Escribe texto
  delay(60000); // Espera 60 segundos

  lcd.clear(); // borra pantalla  
  mhz19_1.calibrateZero();  // Calibra por segunda vez por las dudas
  mhz19_2.calibrateZero(); // Calibra por segunda vez por las dudas
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

void setup() {
  // empieza programa de medición

  pinMode(led_R, OUTPUT);  //Inicia LED ROJO
  pinMode(led_V, OUTPUT);  //Inicia LED VERDE
  pinMode(led_A, OUTPUT);  //Inicia LED AZUL

  pinMode(cal_pin, INPUT_PULLUP); // entrada pulsado para calibrar, seteada como pulluppara poder conectar pulsador sin poenr resistencia adicional
  
  Serial.begin(9600);
  
  // Inicializar el display
  lcd.begin(20,4);
  lcd.clear(); // borra pantalla
  scrollInicio();
  lcd.clear(); // borra pantalla
  //Encender la luz de fondo.
  //display.backlight();

  // Escribimos el Mensaje en el display.
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la segunda línea(fila:1)
  lcd.print("INICIANDO");
  delay(5000);
  lcd.clear(); // borra pantalla
/*----------------------------------------------------------
    calentando MH-Z19 CO2 sensor
  ----------------------------------------------------------*/

  mhz19_1.begin(rx1_pin, tx1_pin); // inicializa el sensor 1
  mhz19_2.begin(rx2_pin, tx2_pin); // inicializa el sensor 2
  /*   MH-Z19 has automatic calibration procedure. the MH-Z19 executing automatic calibration, its do zero point(stable gas environment (400ppm)) judgement.
       The automatic calibration cycle is every 24 hours after powered on.  
       If you use this sensor in door, you execute `setAutoCalibration(false)`.   */  
  mhz19_1.setAutoCalibration(false);
  mhz19_2.setAutoCalibration(false);
  
// muestra indicación de que está calentando 
  lcd.setCursor(0, 0); // Ubicamos el cursor en la primera posición(columna:0) de la primera línea(fila:0)
  lcd.print("Warm up");    // Escribe primera linea del cartel
  delay(60000); // espera 1 minuto            
  lcd.clear(); // borra pantalla  
}


/*----------------------------------------------------------
    MH-Z19 CO2 sensor  loop
  ----------------------------------------------------------*/
void loop() {
  if (digitalRead(cal_pin) == LOW) { // si detecta el botón de calibrar apretado, calibra
      calibrar();
  }
  
  int co2_1ppm = mhz19_1.getPPM(); // mide CO2
  int temp_1 = mhz19_1.getTemperature(); // mide temperatura
  
  int co2_2ppm = mhz19_2.getPPM(); // mide CO2
  int temp_2 = mhz19_2.getTemperature(); // mide temperatura

//  Muestra medición de CO2    
  lcd.clear(); // borra pantalla
  lcd.setCursor(0, 0); // Ubicamos el cursor en la primera posición(columna:0) de la primera línea(fila:0)
  lcd.print("CO2-1: ");    // Escribe texto
  lcd.setCursor(8, 0); // Ubicamos el cursor en la novena posición(columna:8) de la primera línea(fila:0)
  lcd.print(co2_1ppm); // Escribe CO2
  lcd.setCursor(12, 0); // Ubicamos el cursor en la treceava posición(columna:12) de la primera línea(fila:0)
  lcd.print("ppm"); // Escribe texto
  Serial.print ("CO2: ");
  Serial.print (co2_1ppm);
  Serial.println(" ppm");
  
  lcd.setCursor(0, 1); // Ubicamos el cursor en la primera posición(columna:0) de la primera línea(fila:0)
  lcd.print("CO2-2: ");    // Escribe texto
  lcd.setCursor(8, 1); // Ubicamos el cursor en la novena posición(columna:8) de la primera línea(fila:0)
  lcd.print(co2_2ppm); // Escribe CO2
  lcd.setCursor(12, 1); // Ubicamos el cursor en la treceava posición(columna:12) de la primera línea(fila:0)
  lcd.print("ppm"); // Escribe texto
  Serial.print ("CO2: ");
  Serial.print (co2_2ppm);
  Serial.println(" ppm"); 

  delay(5000); //demora 5 seg entre mediciones
}