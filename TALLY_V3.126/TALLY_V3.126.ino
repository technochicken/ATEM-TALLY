#include "FS.h"                   //F("text") Arbeitsspeicher retter Libary
#include "SPIFFS.h"               //Spiffs Libary(Dateien Hochladen
#include <TFT_eSPI.h>             //Libary fürs Display
#include <SPI.h>                  //Spi Libary fürs Display
#include <WiFi.h>                 //Wifi Libary
#include <SkaarhojPgmspace.h>     //Atem Libary
#include <ATEMbase.h>             //Atem Libary
#include <ATEMstd.h>              //Atem Libary

TFT_eSPI tft = TFT_eSPI();

IPAddress clientIp(192, 168, 10, 200);
IPAddress switcherIp(192, 168, 10, 23);
ATEMstd AtemSwitcher;

int DisplayUpdateIf = 12345 ;
String HasH;
String OLDHasH = "nix";
String PRGRMpos = F("Start");


void Display(int Mode){                           //Displayschleife Normalbetrieb
  if (DisplayUpdateIf != Mode){
    DisplayUpdateIf = Mode ;
    if (Mode == 0){
      tft.setTextSize(2);
      tft.fillScreen(TFT_BLUE);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(F("Verbinde"), 15, 45, 4);
    }else if(Mode == 1){
      tft.setTextSize(3);
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_BLUE);
      tft.drawString(F("FREE"), 22, 40, 4);
    }else if(Mode == 2){
      tft.setTextSize(3);
      tft.fillScreen(TFT_GREEN);
      tft.setTextColor(TFT_BLACK);
      tft.drawString(F("PREV."), 15, 35, 4);
    }else if(Mode == 3){
      tft.setTextSize(3);
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_BLACK);
      tft.drawString(F("ON AIR"), -2, 35, 4);
    }else if(Mode == 99){
      DisplayUpdateIf = 123456 ;
    }else{
      tft.setTextSize(1);
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("ERROR "+String(Mode-4), 0, 0, 4);
      tft.drawString(F("Backtrace: "), 0, 20, 4);
      tft.drawString(PRGRMpos, 0, 40, 4);
    }
 
  }
  
}

void MenueDisplay(String Info,String Mode) {                //Displayschleife Menuebetrieb
  PRGRMpos = F("MDisplay");
  String displaY = "";
  tft.setRotation(1);

  HasH = Info+Mode ;
  
  if (HasH != OLDHasH){
    if (Mode == F("LED")){
    displaY = Info ;
    tft.fillScreen(TFT_BLACK);
    if ( Info == F("OFF")){
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
    } else if ( Info == F("WHITE")) {
      tft.fillScreen(TFT_WHITE);
      tft.setTextColor(TFT_BLACK);
    } else if ( Info == F("RED")) {
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_BLACK);
    }else if ( Info == F("GREEN")) {
      tft.fillScreen(TFT_GREEN);
      tft.setTextColor(TFT_BLACK);
    }else if ( Info == F("BLUE")) {
      tft.fillScreen(TFT_BLUE);
      tft.setTextColor(TFT_WHITE);
    }else if ( Info == F("YELLOW")) {
      tft.fillScreen(TFT_YELLOW);
      tft.setTextColor(TFT_BLACK);
    }else if ( Info == F("AZURE")) {
      tft.fillScreen(TFT_SKYBLUE);
      tft.setTextColor(TFT_BLACK);
    } else if ( Info == F("PURPLE")) {
      tft.fillScreen(TFT_PURPLE);
      tft.setTextColor(TFT_BLACK);
    }
    tft.setTextSize(2);
    tft.drawString(Mode, 5, 5, 1);
    tft.setTextSize(5);
    tft.drawString(displaY, 20, 50, 1);
  } else if ( Mode == F("Listen to")){
    displaY = "IN" + Info;
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(Mode, 5, 5, 1);
    tft.setTextSize(7);
    tft.drawString(displaY, 55, 40, 1);
  }
  OLDHasH = HasH;
}
}

int StatusPull(){                                                   // Get current Cam
  PRGRMpos = F("StatusPull");
  AtemSwitcher.runLoop();
  int cameraNumber = readPmem(F("CAM")).toInt();
 /* Serial.print("Atemip: ");
  Serial.print(switcherIp);
  Serial.print(" Myip: ");
  Serial.print(clientIp);
  Serial.print(" CAM PRGRM: ");
  Serial.println(AtemSwitcher.getProgramTally(cameraNumber));
  */
  int ProgramTally = AtemSwitcher.getProgramTally(cameraNumber);
  int PreviewTally = AtemSwitcher.getPreviewTally(cameraNumber);
  
    if ((ProgramTally && !PreviewTally) || (ProgramTally && PreviewTally) ) { // only program, or program AND preview
      LED(true);
      return 3 ;
    } else if (PreviewTally && !ProgramTally) { // only preview
      LED(false);
      return 2 ;
    } else if (!PreviewTally || !ProgramTally) { // neither
      LED(false);
      if (ProgramTally == 0){
        return 0 ;
      } else {
        return 1 ;
      }
   }
}

void writePmem(String path, String message){                    //In Permanenten Speicher ( SPIFFS ) Schreiben
    PRGRMpos = F("wPmem");
    path = ("/"+path+".txt");
    File file = SPIFFS.open(path.c_str(), FILE_WRITE);
    
    if(!file.print(message.c_str())){
      Serial.println("Write failed");
      Display(14);
      delay(2000);
    }
}

String readPmem(String path){                                   //Aus Permanenten Speicher ( SPIFFS ) Lesen
    PRGRMpos = F("rPmem");
    path = ("/"+path+".txt");
    File file = SPIFFS.open(path.c_str());
    
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        Display(14);
        delay(2000);
    }

      char datarcv;
      String Rstr;
      while(file.available()) {
         datarcv = file.read();
         Rstr.concat(datarcv);
      }
      file.close();
      return Rstr;
}

void LED(bool pls){                                                   //Led farbe und an aus Schreiben
  if (pls == true){
      String MODE = readPmem(F("LED"));
      if (MODE != F("OFF")){
        int red = 0;
        int green = 0;
        int blue = 0;
        if (MODE = F("RED")){
          red = 1;
          green = 0;
          blue = 0;
        } else if (MODE = F("GREEN")){
          red = 0;
          green = 1;
          blue = 0;
        } else if (MODE = F("BLUE")){
          red = 0;
          green = 0;
          blue = 1;
        } else if (MODE = F("YELLOW")){
          red = 1;
          green = 1;
          blue = 0;
        } else if (MODE = F("AZURE")){
          red = 0;
          green = 1;
          blue = 1;
        } else if (MODE = F("PURPLE")){
          red = 1;
          green = 0;
          blue = 1;
        } else if (MODE = F("WHITE")){
          red = 1;
          green = 1;
          blue = 1;
        }
      if (red == 1 ){
        digitalWrite(5, HIGH);
      }else {
        digitalWrite(5, LOW);
      }
      
      if (green == 1 ){
        digitalWrite(17, HIGH);
      }else {
        digitalWrite(17, LOW);
      }
      
      if (blue == 1 ){
        digitalWrite(19, HIGH);
      }else {
        digitalWrite(19, LOW);
      }
    
    }
  }

}

int SerialMenue(){                                            //Serielles menue Anzeigen und verarbeiten
  while(Serial.available() > 0 ){
    String str = Serial.readString();

    int ATEMIPindex = str.indexOf("ATEMIP");
    int MYIPindex = str.indexOf("MYIP");
    int SSIDindex = str.indexOf("SSID");
    int PSKindex = str.indexOf("PSK");
    int HELPindex = str.indexOf("/?");
    int REBOOTindex = str.indexOf("/reboot");
    int CLEARindex = str.indexOf("/cls");
    
    int Astindexleading = str.indexOf("ATEMIP_");
    int Astindextrailing = str.indexOf("_ATEMIP");

    int Mstindexleading = str.indexOf("MYIP_");
    int Mstindextrailing = str.indexOf("_MYIP");

    int Sstindexleading = str.indexOf("SSID_");
    int Sstindextrailing = str.indexOf("_SSID");

    int Pstindexleading = str.indexOf("PSK_");
    int Pstindextrailing = str.indexOf("_PSK");


    if (HELPindex > -1){
      Serial.print(F("There are 5 commands to choose from. \n ATEMIP_192.168.178.123_ATEMIP \n MYIP_192.168.178.123_MYIP \n SSID_WIFI-Hotspot01_SSID \n PSK_SUPERSCHWERESPASSWORT_PSK \n /reboot\n /cls\n /?\nType them into the promt to change the given value.\nUnderscores must not be part of your value. Neither shall newline character be part of your value.\n"));
    }
    
    if (CLEARindex > -1){
      Serial.print(F("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"));
      Serial.write(12);
      Serial.print(F("\n"));
    }

     if (SSIDindex > -1){
      if(Sstindexleading > -1 and Sstindextrailing > -1 ){
        String SSIDS = str.substring(Sstindexleading+5,Sstindextrailing);
        Serial.print(F("Succsessfully updatet SSID to: "));
        writePmem(F("SSID"), SSIDS);
        Serial.println(readPmem(F("SSID")));
        Serial.println(F("Canges apply with next powercycle"));
      }else{
        Serial.println(F("Error. No valid SSID found. Format Should be:  SSID_WIFI-Hotspot01_SSID"));
      }
    }

    if (PSKindex > -1){
      if(Pstindexleading > -1 and Pstindextrailing > -1 ){
        String PSKS = str.substring(Pstindexleading+4,Pstindextrailing);
        Serial.print(F("Succsessfully updatet PSK to: "));
        writePmem(F("PSK"), PSKS);
        Serial.println(readPmem(F("PSK")));
        Serial.println(F("Canges apply with next powercycle"));
      }else{
        Serial.println(F("Error. No valid SSID found. Format Should be:  PSK_SUPERSCHWERESPASSWORT_PSK"));
      }
    }
    
    if (ATEMIPindex > -1){
      if(Astindexleading > -1 and Astindextrailing > -1 ){
      String ATEMIPADRESS = str.substring(Astindexleading+7,Astindextrailing);
      Serial.print(F("Succsessfully updatet ATEMIPADRESS to: "));
      writePmem(F("ATEMIP"), ATEMIPADRESS);
      Serial.println(readPmem(F("ATEMIP")));
      Serial.println(F("Canges apply with next powercycle"));
    }else{
      Serial.println(F("Error. No valid IP found. Format Should be:  ATEMIP_012.345.678.910_ATEMIP"));
    }
  }

  if (MYIPindex > -1){
      if(Mstindexleading > -1 and Mstindextrailing > -1 ){
      String MYIPADRESS = str.substring(Mstindexleading+5,Mstindextrailing);
      Serial.print(F("Succsessfully updatet MYIPADRESS to: "));
      writePmem(F("MYIP"), MYIPADRESS);
      Serial.println(readPmem(F("MYIP")));
      Serial.println(F("Canges apply with next powercycle"));
    }else{
      Serial.println(F("Error. No valid IP found. Format Should be:  MYIP_012.345.678.910_MYIP"));
    }

    if (REBOOTindex > -1){
      ESP.restart();
    }
  }  
}

  
}

void Menue(){                                                       //Menue berechnen
PRGRMpos = F("Menue CAM");
int myIN = readPmem(F("CAM")).toInt() ;
Serial.println("CAM loaded");
String myLED = readPmem(F("LED"));
Serial.println("LED loaded");
  //-------------------------------------------------CAM Menue
  MenueCountupStart:
  MenueDisplay(String(myIN), F("Listen to"));
  
  while(BTN() == false){
  }
  
  int long strttime = millis();
  
  while(BTN() == true){
   }
   int long endtime = millis();
  if (endtime >= strttime+600){
    writePmem(F("CAM"), String(myIN));
  }else if(endtime >= strttime+10){
   myIN++; 
   if (myIN >= 21){
    myIN = 0;
   }
   delay(30);
   goto MenueCountupStart;
  }
  PRGRMpos = F("Menue LED");
  //-------------------------------------------------LED Menue
  MenueLED:
  MenueDisplay(myLED,"LED");
 
  while(BTN() == false){
  }
  int long strttimeLED = millis();
  
  while(BTN() == true){
   }
  int long endtimeLED = millis();
  if (endtimeLED >= strttimeLED+600){
    writePmem(F("LED"), myLED);
  }else if(endtimeLED >= strttimeLED+10){
   if (myLED == F("OFF")){
    myLED = F("WHITE");
   } else if (myLED == F("WHITE")){
    myLED = F("RED");
   } else if (myLED == F("RED")){
    myLED = F("GREEN");
   } else if (myLED == F("GREEN")){
    myLED = F("YELLOW");
   } else if (myLED == F("YELLOW")){
    myLED = F("AZURE");
   } else if (myLED == F("AZURE")){
    myLED = F("PURPLE");
   } else {
    myLED = F("OFF");
   }
   goto MenueLED;
  }
 delay(1);
}

bool BTN(){                                 //Knopf auslesen
if (analogRead(35) <=10 ){
  return true ;
}else{
  return false ;
}
}

IPAddress strToIP(String INPUTstr){         //String variable in IP addresse umwandeln und zurückgeben
  uint8_t ipc[4];
  char cstringToParse[25];
  char RAWcstringToParse[25];
  INPUTstr.toCharArray(RAWcstringToParse, 25);
  strcpy(cstringToParse, RAWcstringToParse);
  char * item = strtok(cstringToParse, ".=");
  uint8_t index = 0;
  while (item != NULL) {
    if ((*item >= '0') && (*item <= '9')) {
      ipc[index++] = atoi(item);
    }
    item = strtok(NULL, ".=");
  }
  IPAddress ip(ipc[0], ipc[1], ipc[2], ipc[3]);
  return ip;
}

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(35, INPUT_PULLUP);
  tft.init();
  tft.setRotation(1);
  if(!SPIFFS.begin()){
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    clientIp = strToIP(readPmem(F("MYIP")));
    switcherIp = strToIP(readPmem(F("ATEMIP")));
  WiFi.begin(readPmem(F("SSID")).c_str(), readPmem(F("PSK")).c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Display(0);
    delay(50);
    SerialMenue();
  }
  AtemSwitcher.begin(switcherIp);
  //AtemSwitcher.serialOutput(0x80); //Auskomentieren um Atem debug infos über seriell zu schicken---------------------------------------------------------------------- <------ Atem Debug!
  AtemSwitcher.connect();

}

void loop() {
PRGRMpos = F("Main Loop");
if ( WiFi.status() != WL_CONNECTED ) {
  Display(0);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(readPmem(F("SSID")).c_str(), readPmem(F("PSK")).c_str());
      delay(200);
  }
}
SerialMenue();
int StartTime = millis();
while (BTN()){
}
if (millis()-600> StartTime){
  Serial.println(F("Menue was opened, close the Menue to restart programming!"));
  Menue();
  Display(99);
  delay(1);
}

Display(StatusPull());

}
