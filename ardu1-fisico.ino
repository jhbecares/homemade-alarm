// mirar https://www.arduino.cc/en/Tutorial/EEPROMWrite
// include the library code:
#include <LiquidCrystal.h>
#include <Keypad.h>


/*This is the Sender arduino*/
#include <EEPROM.h>

int rx = 0;
int tx = 1;

/*-------------------------------KEYPAD---------------------------------------*/
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
char keypressed;
char keymap[numRows][numCols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'},
};
//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {7,6,5,4};//Rows 0 to 3
byte colPins[numCols] = {A0,A1,A2,A3};//Columns 0 to 3             
//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

/*------------------------ FIN DEL KEYPAD---------------------------------------*/

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 13);

// LCD Screen Resolution.
int screenWidth = 16;
int screenHeight = 2;

// the two lines
String lineOne, lineTwo;

// reference flags
int stringStart, stringStop, displayMode, i = 0;
int scrollCursor = screenWidth;

String linea1 = "1 Set alarm";
String linea2 = "2 Set 1/2 alarm";
String linea3 = "3 Change passwd";
String linea4 = "4 Manual config";
String linea1Act = "Alarm set";
String linea2Act = "Enter passwd";
bool alarmSet = false;
bool startAlarm = true;

// estados de la alarma
enum State { NONE, ALARM_SET, CHOSEN_ONE, CHOSEN_TWO, CHOSEN_THREE, CHOSEN_FOUR };
State estado;

void changePass();

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(screenWidth, screenHeight);
  
  Serial.begin (112500);  
  //initialize UART pins
  pinMode (rx, OUTPUT);
  pinMode (tx, INPUT);
  
  lcd.clear();
  lcd.setCursor(0,0);     // situamos el cursor el la posición 2 de la linea 0.
  estado = NONE;
}

char codigoSecreto[4] = {'2','2','5','5'}; // Aqui va el codigo secreto
int cursor = 5;
int clave=0; // para el LCD
int posicion=0; // necesaria para la clave


void changePass() {
  bool wrong = false;
  int num1, num2, num3, num4;
  int cont = 0;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("New passwd:");
  lcd.setCursor(1,1);
  lcd.print(">>> ");
  
  while(cont < 4) {
    keypressed = myKeypad.getKey();	

    if (keypressed != 0) //Si el valor es 0 es que no se ha pulsado ninguna tecla
    { // descartamos almohadilla y asterisco
      codigoSecreto[cont] = keypressed;
      lcd.print(keypressed);
      cont++;
    }
  } 
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Passwd updated.");
  delay(2000);
  
  // lo guardamos en la eeprom para que la siguiente vez empiece con ese
  EEPROM.write(0, (int)codigoSecreto[0]);
  EEPROM.write(1, (int)codigoSecreto[1]);
  EEPROM.write(2, (int)codigoSecreto[2]);
  EEPROM.write(3, (int)codigoSecreto[3]);
}

void compruebaNumYActua(int num) {  
  if (keypressed == '1') {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Alarm set!");
    estado = CHOSEN_ONE;
    
    alarmSet = true;
    startAlarm = true;
    
    byte dataR = 1; // lo que le mandamos al otro arduino
    Serial.print(dataR);
    delay(300);
    dataR = 2;
    Serial.print(dataR);
    delay(300);
    
  } else if (keypressed == '2') {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Half alarm set!"); 
    estado = CHOSEN_TWO;
    
    alarmSet = true;
    startAlarm = true;
    
    byte dataR = 5;
    Serial.print(dataR);
    delay(100);
    
  } else if (keypressed == '3') {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Changing pass!");
    changePass();
    delay(100);
    estado = CHOSEN_THREE;
    
  } else if (keypressed == '4') {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("You pressed 4!");
    estado = CHOSEN_FOUR;
  }
}

void clearBuffer() {
 //clear out the serial buffer

 byte w = 0;

 for (int i = 0; i < 10; i++)
 {
   while (Serial.available() > 0)
   {
     char k = Serial.read();
     w++;
     delay(1);
   }
   delay(1);
 } 
}

void setupTeclado() 
{
  cursor = 5;
  clave = 0;
  posicion = 0;
  lcd.begin(16,2);      
  lcd.setCursor(0,0);     // situamos el cursor el la posición 2 de la linea 0.
  lcd.print("Introduzca clave"); // escribimos en LCD
  lcd.setCursor(cursor,1); // cursor en la posicion de la variable, linea 1
}

void loopTeclado() 
{  
  char pulsacion = myKeypad.getKey() ; // leemos pulsacion
  int numPulsaciones = 0;
  if (pulsacion != 0) //Si el valor es 0 es que no se ha pulsado ninguna tecla
  { // descartamos almohadilla y asterisco
    if (pulsacion != '#' && pulsacion != '*' && clave==0)
    { 
      lcd.print(pulsacion); // imprimimos pulsacion
      numPulsaciones++;
      cursor++;  
      delay(200);
      
      //--- Condicionales para comprobar la clave introducida -----------
      // comparamos entrada con cada uno de los digitos, uno a uno
      if (pulsacion == codigoSecreto[posicion]){
        posicion ++; // aumentamos posicion si es correcto el digito
      }
      
      if (posicion == 4)
      { 
        // se han introducido los 4 correctamente
        byte dataX = 0; // lo que le mandamos al otro arduino
        Serial.print(dataX);
        delay(300);
        
        lcd.setCursor(0,0);      // situamos el cursor el la pos 0 de la linea 0.
        lcd.print("Clave correcta  ");         // escribimos en LCD
        delay(300);
        
        dataX = 6; // lo que le mandamos al otro arduino
        Serial.print(dataX);
        delay(300);
        
        // Jenny 
        //startAlarm = true;
        alarmSet = false; // Disarmed!!!!! lo quitamos!
       
        clave=1; // indicamos que se ha introducido la clave
           
        dataX = 0;
        Serial.print(dataX);
        delay(300);
      } 
      if (numPulsaciones >=4 && posicion != 4) {
         delay(200);
        byte dataR = 3; // lo que le mandamos al otro arduino
         Serial.print(dataR);
         delay(200);
      }
     //--- En el caso de que este incompleta o no hayamos acertado ----------
     if(cursor>8)        // comprobamos que no pase de la cuarta posicion
     {  
       cursor=5;     // lo volvemos a colocar al inicio
       posicion=0;           // borramos clave introducida
       lcd.setCursor(5,1);
       lcd.print("    ");       // borramos la clave de la pantalla
       lcd.setCursor(5,1);
       if(clave==0)         // comprobamos que no hemos acertado
       { 
         delay(100);
         // notificamos al otro arduino del error
         byte dataR = 3; // lo que le mandamos al otro arduino
         Serial.print(dataR);
         delay(100);
         
       }
     }
    }
  } 

 //--- Condicionales para resetear clave introducida -------------
 if (pulsacion == '*')
 { // asterisco para resetear el contador
   posicion = 0;
   cursor = 5;
   clave=0;
   posicion=0;
   lcd.setCursor(0,0); // situamos el cursor el la posición 2 de la linea 0.
   lcd.print("Introduzca clave"); // escribimos en LCD
   lcd.setCursor(5,1);
   lcd.print("    "); // borramos de la pantalla los numeros
   lcd.setCursor(5,1);
 }
}

void loop() {
  
  int val1, val2, val3, val4;
  
  // leemos la clave que esta guardada
  val1 = EEPROM.read(0);
  val2 = EEPROM.read(1);
  val3 = EEPROM.read(2);
  val4 = EEPROM.read(3);
  
  if (val1 == 0 && val2 == 0 && val3 == 0 && val4 == 0) {
   	// guardamos en la eeprom el codigo secreto inicial
    EEPROM.write(0, (int)codigoSecreto[0]);
    EEPROM.write(1, (int)codigoSecreto[1]);
    EEPROM.write(2, (int)codigoSecreto[2]);
    EEPROM.write(3, (int)codigoSecreto[3]);
  }
  
  // leemos la clave que esta guardada
  val1 = EEPROM.read(0);
  val2 = EEPROM.read(1);
  val3 = EEPROM.read(2);
  val4 = EEPROM.read(3);
  
  if (!alarmSet) {
    setup();
    // 1. Activar alarma 
    // 2. Activar alarma (media alarma)
    // 3. Cambiar passwd
    // 4. Configuracion manual (activar/desactivar aparatos de uno en uno)

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(linea1);
    lcd.setCursor(0, 1);
    lcd.print(linea2);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(linea2);
    lcd.setCursor(0, 1);
    lcd.print(linea3);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(linea3);
    lcd.setCursor(0, 1);
    lcd.print(linea4);
    delay(1000);
    
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Chosen value: "); 
    lcd.setCursor(0,1);
    lcd.print(">>> ");

    bool wrong = false;
    while(true) {
      keypressed = myKeypad.getKey();	

      if (keypressed != 0) //Si el valor es 0 es que no se ha pulsado ninguna tecla
      { // descartamos almohadilla y asterisco
        if (keypressed != '#' && keypressed != '*' && keypressed <= '4') {   
          lcd.print(keypressed); 
          break;
        } 
        if (keypressed > '4') {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Wrong value!");
          lcd.setCursor(0,1);
          lcd.print(keypressed);
          lcd.setCursor(1,1);
          lcd.print(" not valid :(");
          wrong = true;
          break;
        }  
      } 
    }

    delay(200);

    // Si el numero es correcto, vemos que tenemos que hacer con el
    if (!wrong) {
      compruebaNumYActua(keypressed);
    } 
    delay(200); 
  }
  else {
    // parte de la alarma */
    if (startAlarm == true) {
      setupTeclado();
      startAlarm = false;
    }
    loopTeclado();
    if (alarmSet == false) {
      delay(200); 
    }
  }
}
