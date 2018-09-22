#include <NTPClient.h>                // time libary
#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>


#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define LIGHTCYAN    0x6DFE
#define DARKCYAN    0x25BA
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//POWER
#define VOLTAGE_PIN A8
int voltage;
float ampere = 0.1;
int watts;

//LCD size
#define TFTWIDTH   240
#define TFTHEIGHT  320

Adafruit_TFTLCD screen(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
EthernetClient client;
EthernetUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.pl.pool.ntp.org",7200);

String localIp;

void setup() {
  Serial.begin(9600);

  screen.reset();
  screen.begin(0x9341); // SDFP5408
  screen.setRotation(2); 
  screen.fillScreen(BLACK);
  screen.setCursor(0, 0);
  
  screen.setTextColor(WHITE);  
  screen.setTextSize(1);
  screen.println();
  screen.println("________________________________________");
  screen.println("   System Monitoringu Zuzycia Energii  ");
  screen.println("________________________________________");
  screen.println();
  
  screen.setTextColor(WHITE);  
  screen.setTextSize(2);
  screen.println("Inicjowanie");
  screen.setTextColor(WHITE);  
  screen.setTextSize(1);
  LoadingBars();
  
  screen.setTextColor(WHITE);  
  screen.setTextSize(2);
  screen.println("Konfiguracja IP");
  screen.setTextColor(WHITE); 
  screen.setTextSize(1);
  LoadingBars();
  if (Ethernet.begin(mac) == 0) {
    screen.setTextColor(WHITE);  
    screen.setTextSize(2);
    screen.println("Bład konfiguracji DHCP");
    for(;;)
      ;
  }
  
  screen.setTextColor(WHITE);  
  screen.setTextSize(2);
  screen.println("Adres IP");
  localIp = DisplayIpAddress(Ethernet.localIP());
  screen.println(localIp);
  screen.println();
  
  screen.setTextColor(WHITE);  
  screen.setTextSize(2);
  screen.println("Ustawienia czasu");
  screen.setTextColor(WHITE);  
  screen.setTextSize(1);
  LoadingBars();
  timeClient.update();
  screen.setTextSize(2);
  screen.println(timeClient.getFormattedTime());

  LoadUI();
}


void loop() {
  timeClient.update();
  int voltageRead = analogRead(VOLTAGE_PIN);
  voltage = voltageRead * (5.0/1023.0) * 50;
  ampere = ampere+0.1;
  watts = ampere*voltage;
  refreshUI();
  delay(1000);

}

void LoadingBars(){
  screen.setTextColor(WHITE); 
  screen.setTextSize(1);
   for(int i =0; i<40; i++){
    screen.print(".");
    //delay(100);
  }
  screen.println();
}

void LoadUI(){
   screen.fillScreen(WHITE);
   screen.fillRect(0, 0, TFTWIDTH, 40, LIGHTCYAN);
   screen.fillRect(0, 40, TFTWIDTH, 120, DARKCYAN);
   screen.fillRect(0, 160, TFTWIDTH, 120, LIGHTCYAN);
   screen.fillRect(0, 280, TFTWIDTH, TFTHEIGHT, DARKCYAN);
   screen.setTextColor(WHITE); 
   screen.setTextSize(4);
   screen.setCursor(180, 45);
   screen.print(" W ");
   screen.setCursor(180, 85);
   screen.print(" V ");
   screen.setCursor(180, 125);
   screen.print(" A ");
   screen.setTextSize(3);
   screen.setCursor(5, 170);
   screen.print(" L1 ");
   screen.setCursor(5, 210);
   screen.print(" L2 ");
   screen.setCursor(5, 250);
   screen.print(" L3 ");
   screen.setCursor(187, 170);
   screen.print(" W ");
   screen.setCursor(187, 210);
   screen.print(" W ");
   screen.setCursor(187, 250);
   screen.print(" W ");
   screen.setTextSize(2);
   screen.setCursor(40, 295);
   screen.println(localIp);
   screen.setCursor(70, 15);
   screen.println(timeClient.getFormattedTime());
}

void refreshUI(){

  screen.setTextSize(2);
  screen.setCursor(70, 15);
  screen.setTextColor(WHITE, LIGHTCYAN); 
  screen.println(timeClient.getFormattedTime());


  screen.setTextSize(4);
  screen.setTextColor(WHITE, DARKCYAN);
  screen.setCursor(75, 45);
  screen.print(watts);
  screen.setCursor(75, 85);
  screen.print(voltage);
  screen.setCursor(75, 125);
  screen.print(ampere,1);
}
String DisplayIpAddress(IPAddress ip)
{
 return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}


