/* MANTA ELECTRICA INTELIGENTE
PASO 2: Programar la manta para que encienda o apague de acuerdo con la información recopilada en el paso 1
se han utilizado fragmentos de código ya existentes como los generados por Temboo y los de las librerías RTClib.h y Wire.h*/

#include <Wire.h>
#include "RTClib.h"
#include <Bridge.h>
#include <Temboo.h>
#include "Account.h" // continene informacion de las cuentas de google y de temboo

#define TS A2
#define RL 5
#define TEMP_IDEAL 20
#define TIME_WEAKEUP 8
RTC_DS1307 RTC;

// Variables de control para el ciclo de lectura de la Spreadsheet de Google
byte numRuns = 1;   // Execution count, so this doesn't run forever
const byte maxRuns = 3;   // Maximum number of times the Choreo should be executed

// Variables para el sensor de temperatura
float a=0.0, temperature=0.0;

// Variables reloj
int hora=0, minuto=0;
byte b=0;

// Variable para el promedio de hora de dormir
float HSleep=0.0;

float GetCellVal(String CellHour){
    
    TembooChoreo RetrieveCellValueChoreo;

    // Invoke the Temboo client
    RetrieveCellValueChoreo.begin();

    // Set Temboo account credentials
    RetrieveCellValueChoreo.setAccountName(TEMBOO_ACCOUNT);
    RetrieveCellValueChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    RetrieveCellValueChoreo.setAppKey(TEMBOO_APP_KEY);
    
    // Set Choreo inputs
    RetrieveCellValueChoreo.addInput("RefreshToken", GOOGLE_REFRESH_TOKEN);
    RetrieveCellValueChoreo.addInput("ClientSecret", GOOGLE_CLIENT_SECRET);
    RetrieveCellValueChoreo.addInput("SpreadsheetName", SPREADSHEET_TITLE);
    RetrieveCellValueChoreo.addInput("ClientID", GOOGLE_CLIENT_ID);
    RetrieveCellValueChoreo.addInput("CellLocation", CellHour);
    RetrieveCellValueChoreo.addInput("SpreadsheetKey", SPREADSHEET_KEY);
    RetrieveCellValueChoreo.addInput("WorksheetName", WORKSHEET_TITLE);
    
    // Identify the Choreo to run
    RetrieveCellValueChoreo.setChoreo("/Library/Google/Spreadsheets/RetrieveCellValue");
    
    // Run the Choreo; when results are available, print them to serial
    unsigned int returnCode = RetrieveCellValueChoreo.run();
    
   // a response code of 0 means success; print the API response
    if(returnCode == 0) {
      
      String ValorCelda; // a String to hold the CellValue
      String NewAccToken; // a String to hold the NewAccToken


      // choreo outputs are returned as key/value pairs, delimited with 
      // newlines and record/field terminator characters, for example:
      // Name1\n\x1F
      // Value1\n\x1E
      // Name2\n\x1F
      // Value2\n\x1E      
      
      // see the examples at http://www.temboo.com/arduino for more details
      // we can read this format into separate variables, as follows:
      
      while(RetrieveCellValueChoreo.available()) {
        // read the name of the output item
        String name = RetrieveCellValueChoreo.readStringUntil('\x1F');
        name.trim();

        // read the value of the output item
        String data = RetrieveCellValueChoreo.readStringUntil('\x1E');
        data.trim();

        // assign the value to the appropriate String
        if (name == "NewAccessToken") {
          NewAccToken = data;
        } else if (name == "CellValue") {
          ValorCelda = data;
        }
      }
     
      return ValorCelda.toFloat();    
      
    } else {
      // there was an error
      // print the raw output from the choreo
      
      while(RetrieveCellValueChoreo.available()) {
        char c = RetrieveCellValueChoreo.read();
        Serial.print(c);
      }
      return 0;
    }

  RetrieveCellValueChoreo.close();
  delay(30000); // wait 30 seconds between RetrieveCellValue calls
  }

  
void setup() {
  // Inicializar el Clock
   Wire.begin();
   RTC.begin();
    
   // Verificar si el RTC está corriendo.
   if (! RTC.isrunning()) {
    Serial.println(F("RTC is NOT running"));
   }
  // Compara el tiempo actual leido por el RTC y lo compara con 
  // el tiempo del compilador. Si es necesario el RTC se actualiza.
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println(F("RTC is older than compile time! Updating"));
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
    // Relay
  pinMode(RL, OUTPUT);
  
  // Sensor de Temperatura
  pinMode(TS, INPUT);
  
  Serial.begin(9600);
  
  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);
  Bridge.begin();

  for(numRuns; numRuns <= maxRuns; numRuns++) {
    Serial.println(F("\nRunning RetrieveCellValue"));
    String CellHour="B";
    CellHour += numRuns+1;
    float ValorCelda=GetCellVal(CellHour);
    Serial.println(F("Hora de dormir obtenida de SleepArduinoData: "));
    Serial.print(ValorCelda);
    if(ValorCelda==0){
      Serial.println(F("\nError al obtener el valor de la celda"));
      numRuns--;
    }  
    else{
      HSleep=HSleep+ValorCelda;
      Serial.println(F("\nSumatoria de Horas de dormir: "));
      Serial.print(HSleep);
    }
 }
 HSleep=HSleep/maxRuns;
 Serial.println(F("\nPromedio de Horas de dormir: "));
 Serial.print(HSleep);
}

void loop() {

 String fecha="";

  // Leemos la temperatura actual del ambiente

  a=analogRead(TS);
  temperature = ((a/1023) - 0.5)*100; // Info del sensor a grados celsius
  Serial.println(F("\nLa temperatura captada por el sensor es:  "));
  Serial.println(temperature);

  // Leemos la fecha y hora actual
  
  Serial.println(F("La fecha es:  "));
    DateTime now = RTC.now(); 
    fecha += now.year();
    fecha += '/';
    fecha += now.month();
    fecha += '/';
    fecha += now.day();
    Serial.println(fecha);
    
    Serial.println(F("La hora actual es:  "));
    hora=now.hour();
    Serial.print(hora);
    Serial.print(":");
    minuto=now.minute();
    Serial.print(minuto);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.println(F(" ")); 

    // Verificamos si la temperatura es inferior a la ideal menos 5 grados, esto para dar un margen tolerable
    // de cambio de estado de la manta para que no se encienda y apague demasiadas veces
    // También verificamos si la hora leida por el sensor es mayor o igual a la Hora promedio de dormir
    // menos quince minutos de cumplirse ambas condiciones se enciende la manta y permanece así mientras sean verdaderas
    // de lo contrario si la temperatura leida es mayor a la temperatura ideal mas 5 grados (margen tolerable) y
    // la hora leída es menor a la hora promedio de dormir o superior a la hora determinada como hora de levantarse
    // entonces la manta se apaga y continúa apagada hasta que la primera condición vuelva a ser verdadera
    
  if(temperature < (TEMP_IDEAL-5) && hora >= HSleep-0.25) {
      if(b==0){
        digitalWrite(RL, HIGH);
        Serial.println(F("ENCENDER MANTA ELECTRICA"));
        b=1;
      }
      Serial.print(F("Se mantiene encendida"));
    }
    if(temperature > (TEMP_IDEAL+5) && (hora < HSleep || hora > TIME_WEAKEUP)){
      if(b==1){
        
        digitalWrite(RL, LOW);
        Serial.println(F("APAGAR MANTA ELECTRICA"));
        b=0;
      }
      Serial.println(F("Se mantiene apagada"));
    }
    delay(30000); // Repetimos cada 30 segundos
}
