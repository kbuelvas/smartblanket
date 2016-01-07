/* MANTA ELECTRICA INTELIGENTE
PASO 1: Recopilar información y enviarla a Google Docs*/

#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h" // contains Temboo account information
#include "GoogleAccount.h" // contains Google account information
#include "RTClib.h"
#include <Wire.h>

#define BT 4      // Conectamos la entrada del botón al pin 4


RTC_DS1307 RTC;

int numRuns = 1;   // execution count, so this doesn't run forever
int maxRuns = 3; // the max number of times the Google Spreadsheet Choreo should run
int estadoBoton = 0;       // variable para ver el estado del boton 

void setup() {
  
  
   // Botón
   pinMode(BT, INPUT);  // pin 4 (boton) modo ENTRADA   

   // Inicializar el Clock
   Wire.begin();
   RTC.begin();
    
   // Check if the RTC is running.
   if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running");
   }

  // This section grabs the current datetime and compares it to
  // the compilation time.  If necessary, the RTC is updated.
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time! Updating");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
 
  Serial.println("Setup complete.");
  
  // for debugging, wait until a serial console is connected
  Serial.begin(9600);
  while(!Serial);
  Serial.print("Initializing the bridge... ");
  Bridge.begin();
  Serial.println("Done!\n");
}

void loop()
{

  // Código para obtener la información de la hora en la que la persona se va a la cama
  
  int hora=0, minuto=0, b=0;     // Variables reloj
  String fecha="";

  // Leemos el estado del botón
  estadoBoton = digitalRead(BT);
  Serial.println("\nEstado del Boton: ");
  Serial.print(estadoBoton);
  
  // Obtenemos la fecha y hora
  Serial.println("\nGetting time value...");
    DateTime now = RTC.now(); 
    fecha += now.year();
    fecha += "/";
    fecha += now.month();
    fecha += "/";
    fecha += now.day();
    Serial.println(fecha);
    Serial.print(' ');
    hora=now.hour();
    Serial.print(hora);
    Serial.print(':');
    minuto=now.minute();
    Serial.print(minuto);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println(" "); 

  // Si el botón está enciendido
  if (estadoBoton == HIGH) 
  {     
    // Llamamos al Choreo de Temboo para subir la información a la Spreadsheet de google 
    // que hemos creado con el nombre SleepArduinoData
    
  if (numRuns <= maxRuns) {

    Serial.println("Running AppendRow - Run #" + String(numRuns++));

    // get the number of milliseconds this sketch has been running
    unsigned long now = millis();

    
    Serial.println("Buenas noches...\n");
    
    Serial.println("\n Appending value to spreadsheet...");

    // we need a Process object to send a Choreo request to Temboo
    TembooChoreo AppendRowChoreo;

    // invoke the Temboo client
    // NOTE that the client must be reinvoked and repopulated with
    // appropriate arguments each time its run() method is called.
    AppendRowChoreo.begin();
    
    // set Temboo account credentials
    AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
    AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);
    
    // identify the Temboo Library choreo to run (Google > Spreadsheets > AppendRow)
    AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");
    
    // set the required Choreo inputs
    // see https://www.temboo.com/library/Library/Google/Spreadsheets/AppendRow/ 
    // for complete details about the inputs for this Choreo
    
    // your Google Client ID
    AppendRowChoreo.addInput("ClientID", GOOGLE_CLIENT_ID);

    // your Google Client Secret
    AppendRowChoreo.addInput("ClientSecret", GOOGLE_CLIENT_SECRET);

    // your Google Refresh Token
    AppendRowChoreo.addInput("RefreshToken", GOOGLE_REFRESH_TOKEN);

    // the title of the spreadsheet you want to append to
    AppendRowChoreo.addInput("SpreadsheetTitle", SPREADSHEET_TITLE);

    // convert the time and sensor values to a comma separated string
    String rowData=fecha;
    rowData += ",";
    rowData += hora;
    rowData += ",";
    rowData += minuto;

    // add the RowData input item
    AppendRowChoreo.addInput("RowData", rowData);

    // run the Choreo and wait for the results
    // The return code (returnCode) will indicate success or failure 
    unsigned int returnCode = AppendRowChoreo.run();

    // return code of zero (0) means success
    if (returnCode == 0) {
      Serial.println("Success! Appended " + rowData);
      Serial.println("");
    } else {
      // return code of anything other than zero means failure  
      // read and display any error messages
      while (AppendRowChoreo.available()) {
        char c = AppendRowChoreo.read();
        Serial.print(c);
      }
    }

    AppendRowChoreo.close();
  }

  Serial.println("\nWaiting...");
  delay(5000); // wait 5 seconds between AppendRow calls
  
  }else 
  {                              // si el boton no ha sido pulsado
    Serial.print('Esperando que se vaya a dormir...');         
    delay(5000);                  // Retardo de 0,5s
  }
}
