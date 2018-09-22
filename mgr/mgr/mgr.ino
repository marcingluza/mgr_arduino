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
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//POWER
#define VOLTAGE_PIN A8
int voltage;

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
  int voltageRead = analogRead(VOLTAGE_PIN);
  voltage = voltageRead * (5.0/1023.0) * 50;
  Serial.print(voltage);
  Serial.println(" V");
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
   screen.reset();
   screen.fillScreen(CYAN);
   screen.fillRect(0, 0, 50, 50, 0);
}
String DisplayIpAddress(IPAddress ip)
{
 return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}


