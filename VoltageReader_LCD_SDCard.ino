/*

  Created by k3ntinhu
  07 Jan 2017

  --> Requires a Nokia 5110 LCD module connected to:
     (Nokia)  -> Arduino
     (1) RST  -> Pin 7
     (2) CE   -> Pin 6  
     (3) DC   -> Pin 5
     (4) DIN  -> Pin 3
     (5) CLK  -> Pin 2
     (6) VCC  -> 3.3V
     (7) LGHT -> Pin 8
     (8) GND  -> GND

  --> Requires a SD card read/write attached to SPI bus as follows:
       SDCard -> Arduino
      ** GND  -> GND
      ** VCC  -> 5V
      ** MISO -> pin 12
      ** MOSI -> pin 11
      ** SCK  -> pin 13
      ** CS   -> pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  --> Requires a TP4056 connected as follows:
      TP4056 -> Arduino
      ** In+ -> Analog 0
      ** In- -> GND
      ** Bat+ -> Battery Holder +
      ** Bat- -> Battery Holder -

// LCD font size library options: smallFont 6x8, mediumNumber 12x16, bigNumbers 14x24

*/

String filename = "test.txt"; // defines the name of the file to save data

#include <SPI.h>
#include <SD.h>
#include <LCD5110_Basic.h> // include LCD libraries


LCD5110 myGLCD(2, 3, 5, 7, 6);

File myFile;

// images and fonts
extern uint8_t arduino_logo[];
extern uint8_t save_img[];
extern uint8_t settings_img[];
extern uint8_t working1[];
extern uint8_t working2[];
extern uint8_t working3[];
extern uint8_t working4[];
extern uint8_t oshw_logo[];
extern uint8_t done_img[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

String conteudo_a_gravar;
String blstatus;
boolean backlight = true;
int leds = 0;
int dpval = 0;
float voltRef = 5.07; // Reference voltage (probe your 5V pin) 
float battVolt = 0.0;

void setup() {

  blstatus = "On";

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(10, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW); //Turn Backlight ON
  digitalWrite(10, LOW); //Turn TP4056 OFF

  myGLCD.InitLCD(); // initialize LCD

  // design Arduino Logo
  myGLCD.drawBitmap(0, 0, arduino_logo, 84, 48);
  myGLCD.invert(false);
  delay(500);

  myGLCD.clrScr();

  Serial.println("Battery Tester v01");
  Serial.print("SD Card ");
  myGLCD.setFont(SmallFont); // set SmallFont
  myGLCD.print("SD Card Init", CENTER, 16);

  if (!SD.begin(4)) {
    myGLCD.clrScr();
    Serial.println("[Off]");
    myGLCD.print("SD Card Fail", CENTER, 16);
    return;
  }
  myGLCD.clrScr();
  Serial.println("[On]");
  myGLCD.print("SD Card OK", CENTER, 16);

  Serial.println("");
  Serial.println("BATTERY TESTER MAIN MENU");
  Serial.println("1> Run Test");
  Serial.println("2> Light (On/Off)");
  Serial.println("3> Read File Content");
  Serial.println("4> Delete File");
  Serial.println("5> View Test");

  drawMenu(blstatus);

}

void loop() {

  // make a string for assembling the data to log:
  String conteudo_a_gravar = "";
  int sensorValue1 = analogRead(A0);
  int a = 0;

  if (Serial.available())
  {
    char ch = Serial.read();

    if (ch == '1')
    {
      readvoltage(ch);
    }

    if (ch == '2')
    {
      if (backlight)
      {
        backlight = false;
        blstatus = "Off";
        Serial.println("Backlight Off");
        turnBacklightOff();
      }
      else
      {
        backlight = true;
        blstatus = "On";
        Serial.println("Backlight On");
        turnBacklightOn();
      }

    }

    if (ch == '3')
    {
      readFileContent();
    }

    if (ch == '4')
    {
      deleteFile();
    }

    if (ch == '5')
    {
      readvoltage(ch);
    }

    if (ch == 's')
    {
      drawMenu(blstatus);
    }

  }
}
void deleteFile() {
  myFile.close();
  SD.remove(filename);
  Serial.print(filename);
  Serial.println(" deleted.");
}

void readFileContent() {

  // open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.print("Reading File Content of: ");
    Serial.println(filename);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error Opening File.");
    myGLCD.clrScr();
    myGLCD.print("SD Open Err", CENTER, 16);
  }
}

void writetocard(String conteudo_a_gravar) {

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.

  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened, write it:
  if (myFile) {
    //myGLCD.drawBitmap(0, 0, save_img, 84, 48);
    Serial.print("Writing to ");
    Serial.print(filename);
    Serial.print(" ");

    Serial.println(conteudo_a_gravar);
    myFile.println(conteudo_a_gravar);

    myFile.close(); // close the file

    //myGLCD.clrScr(); // clear the LCD
    //myGLCD.drawBitmap(0, 0, save_img, 84, 48);

    //myGLCD.clrScr();

  } else {
    // if the file didn't open, print an error:
    Serial.print("Error Opening File ");
    Serial.println (filename);

    //myGLCD.clrScr();
    myGLCD.print("SD Open Err", CENTER, 16);
  }

}

void readvoltage(char ch) {

  // define pins to be used reading voltage
  int sensorValue1 = analogRead(A0);
  int sensorValue2 = analogRead(A5);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  // replace 1023 with 1012 to adjust the cable power loss
  float voltage1 = sensorValue1 * (5.0 / 1023.0);
  float voltage2 = sensorValue2 * (5.0 / 1023.0);

  dpval = digitalRead(10); // lê o estado do pin10
  Serial.println("Begining Test");

  int a = 1;
  int aa = 1;
  int per_voltage1 = 0;
  int limite_timer = 10;
  float dif;

  //verifica se tem alguma pilha inserida
  if (voltage1 < 2.7) {
    myGLCD.clrScr();
    myGLCD.setFont(SmallFont); // set SmallFont
    myGLCD.print("Insert", CENTER, 16);
    myGLCD.print("Battery", CENTER, 24);    
    delay (2000);
    drawMenu(blstatus);
    return 0;
  }
  
  do {

    battVolt = analogRead(A0) * (voltRef / 1024.0);
    
    Serial.print("battVolt: ");
    Serial.println(battVolt);
  
    // define pins to be used reading voltage
    int sensorValue1 = analogRead(A0);
    int sensorValue2 = analogRead(A5);    
    
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    // replace 1023 with 1012 to adjust the cable power loss

    float voltage2 = sensorValue2 * (5.0 / 1023.0);
    float voltage1 = sensorValue1 * (5.0 / 1023.0);

    voltage1 = voltage1 + 0.025; // soma a diferença perdida nos cabos
   
    myGLCD.clrScr();
    myGLCD.setFont(SmallFont); // set SmallFont
    myGLCD.print("BATTERY TESTER", CENTER, 0);
    myGLCD.print("V:", LEFT, 16);
    myGLCD.setFont(MediumNumbers); // set BigNumbers font
    myGLCD.printNumF(voltage1 / 1, 2, RIGHT, 8);

    myGLCD.setFont(SmallFont); // set SmallFont
    //myGLCD.print("cnt:", LEFT, 32);
    myGLCD.printNumI(a, LEFT, 32);
    
    per_voltage1 = (voltage1 * 100) / 4.2;
    myGLCD.printNumI(per_voltage1, 60, 40);
    myGLCD.print("%", RIGHT, 40);

    myGLCD.print("Vin", LEFT, 40);
    myGLCD.printNumF(voltage2 / 1, 2, 24, 40);
    //myGLCD.printNumF(dif / 1, 2, 24, 40);

    delay(1000);
    a = a + 1;
    aa = aa + 1;
    
    conteudo_a_gravar = voltage1;

    if (ch == '1')
    {
      limite_timer = 120; // tempo limite do teste em segundos
      
      // verifica o estado do pin10
      if (dpval == 0) { 
        digitalWrite(10, HIGH); //Turn TP4056 ON
      }
      
      // grava o resultado a cada 60 seg.
      if (aa == 60) {
      aa = 0;
      writetocard(conteudo_a_gravar);
      }
      
    }

  } while ((voltage1 <= 4.20) && (a <= limite_timer));

  digitalWrite(10, LOW); //Turn TP4056 OFF

  myGLCD.clrScr();
  myGLCD.setFont(SmallFont); // set SmallFont
  myGLCD.print("Finished", CENTER, 16);
  myGLCD.print("Test", CENTER, 24);
  Serial.println("Finished Test");
  delay (5000);
  drawMenu(blstatus);
  return 0;

}

void turnBacklightOn()
{
  digitalWrite(8, LOW);
  drawMenu(blstatus);
}

void turnBacklightOff()
{
  digitalWrite(8, HIGH);
  drawMenu(blstatus);
}

void drawMenu(String blstatus)
{

  myGLCD.clrScr();
  myGLCD.setFont(SmallFont); // set SmallFont
  myGLCD.print("MAIN MENU", CENTER, 0);

  myGLCD.print("1> Run Test ", LEFT, 16);

  myGLCD.print("2> Light", LEFT, 24);
  myGLCD.print(blstatus, RIGHT, 24);

}
