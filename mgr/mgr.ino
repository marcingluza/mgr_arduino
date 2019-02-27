  #include <EmonLib.h>
#include <NTPClient.h>
#include <SPFD5408_Adafruit_GFX.h>
#include <SPFD5408_Adafruit_TFTLCD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <HttpClient.h>

const String devID = "magister2018";


#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4

#define BLACK   0x0000
#define LIGHTCYAN    0x6DFE
#define DARKCYAN    0x25BA
#define WHITE   0xFFFF

//POWER
#define VOLTAGE_PIN A8
#define L1AMPERE_PIN A9
#define L2AMPERE_PIN A10
#define L3AMPERE_PIN A11
int voltage = 0;
float ampere_L1 = 0.0;
float ampere_L2 = 0.0;
float ampere_L3 = 0.0;
int watt_L1 = 0;
int watt_L2 = 0;
int watt_L3 = 0;
float ampereSum = 0.0;
int watts = 0;

EnergyMonitor EnergyMonitorL1;
EnergyMonitor EnergyMonitorL2;
EnergyMonitor EnergyMonitorL3;

//LCD size
#define TFTWIDTH   240
#define TFTHEIGHT  320

Adafruit_TFTLCD screen(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

byte myserver[] = { 192, 168, 1, 211 };
String host = "192.168.1.211";
int serverPort = 64996;
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
EthernetClient client;
EthernetServer server(9095);
EthernetUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.pl.pool.ntp.org", 7200);
int connectionAttempts = 1;


String localIp;

void setup() {
  Serial.begin(115200);

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

  EnergyMonitorL1.current(9, 30);
  EnergyMonitorL2.current(10, 30);
  EnergyMonitorL3.current(11, 30);

  screen.setTextColor(WHITE);
  screen.setTextSize(2);
  screen.println("Kalibr. czujnikow");
  screen.setTextColor(WHITE);
  screen.setTextSize(1);

  for (int i = 0; i < 8; i++) {
    ampere_L1 = (EnergyMonitorL1.calcIrms(1480));
    ampere_L2 = (EnergyMonitorL2.calcIrms(1480));
    ampere_L3 = (EnergyMonitorL3.calcIrms(1480));
    for (int j = 0; j < 5; j++) {
      screen.print(".");
      //delay(50);
    }
  }
  screen.println();

  screen.setTextColor(WHITE);
  screen.setTextSize(2);
  screen.println("Konfiguracja IP");
  screen.setTextColor(WHITE);
  screen.setTextSize(1);
  LoadingBars();

  while (Ethernet.begin(mac) == 0) {
    screen.setTextColor(WHITE);
    screen.setTextSize(1);
    screen.print("Proba polaczenia: ");
    screen.println(connectionAttempts);
    connectionAttempts++;
    screen.println("Blad konfiguracji DHCP");
    delay(1000);
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

  screen.setTextColor(WHITE);
  screen.setTextSize(2);
  screen.println("Start WebSerwera");
  screen.setTextColor(WHITE);
  screen.setTextSize(1);
  LoadingBars();
  server.begin();

  LoadUI();
}

void LoadingBars() {
  screen.setTextColor(WHITE);
  screen.setTextSize(1);
  for (int i = 0; i < 40; i++) {
    screen.print(".");
    //delay(100);
  }
  screen.println();
}


void loop() {
  timeClient.update();
  int voltageRead = analogRead(VOLTAGE_PIN);
  voltage = voltageRead * (250.0 / 1023.0);
  Serial.print(voltage);
  Serial.println(" [V]");
  ampere_L1 = (EnergyMonitorL1.calcIrms(1480));
  watt_L1 = ampere_L1 * voltage;
  ampere_L2 = (EnergyMonitorL2.calcIrms(1480));
  watt_L2 = ampere_L2 * voltage;
  ampere_L3 = (EnergyMonitorL3.calcIrms(1480));
  watt_L3 = ampere_L3 * voltage;

  Serial.print(ampere_L1);
  Serial.println(" L1 [A]");
  Serial.print(ampere_L2);
  Serial.println(" L2 [A]");
  Serial.print(ampere_L3);
  Serial.println(" L3 [A]");
  ampereSum = ampere_L1 + ampere_L2 + ampere_L3;
  Serial.print(ampereSum);
  Serial.println(" sum [A]");
  watts = ampereSum * voltage;
  refreshUI();
  refreshWebServer();
  post();
}

void post()
{
  String PostString = "/Arduino/Add?id=" + devID + "&voltage=" + voltage + "&l1_current=" + ampere_L1 + "&l1_current=" + ampere_L1 + "&l1_current=" + ampere_L1;
   if (client.connect(myserver, 80)) {  
    Serial.println("connected");
    client.println("GET /Arduino/Add?id=" + devID + "&voltage=" + voltage + "&l1_current=" + ampere_L1 + "&l2_current=" + ampere_L2 + "&l3_current=" + ampere_L3 + " HTTP/1.1"); 
    Serial.println("connected");
    client.println("Host: " + host);
    client.println("Connection: close");
    client.println();
  } 
  else {
    Serial.println("connection failed");
    Serial.println();
  }

  while(client.connected() && !client.available()) delay(1);
  while (client.connected() || client.available()) 
  { 
    char c = client.read(); 
    Serial.print(c);
  }

  Serial.println();
  Serial.println("disconnecting.");
  Serial.println("==================");
  Serial.println();
  client.stop();
}


void refreshWebServer() {
  EthernetClient ethClient = server.available();
  if (ethClient) {
    bool currentLineIsBlank = true;
    while (ethClient.connected()) {
      if (ethClient.available()) {
        char c = ethClient.read();
        if (c == '\n' && currentLineIsBlank) {
          ethClient.println("HTTP/1.1 200 OK");
          ethClient.println("Content-Type: application/json;charset=utf-8");
          ethClient.println("Connection: close");
          ethClient.println();
          ethClient.print("{\"time\":\"" + String(timeClient.getEpochTime(), DEC) + "\",\"deviceid\":\"" + devID + "\",\"values\":{\"voltage\":\"");
          ethClient.print(voltage);
          ethClient.print("\",\"l1current\":\"");
          ethClient.print(ampere_L1);
          ethClient.print("\",\"l2current\":\"");
          ethClient.print(ampere_L2);
          ethClient.print("\",\"l3current\":\"");
          ethClient.print(ampere_L3);
          ethClient.print("\"}}");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    ethClient.stop();
  }
}

void LoadUI() {
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

void refreshUI() {

  screen.setTextSize(2);
  screen.setCursor(70, 15);
  screen.setTextColor(WHITE, LIGHTCYAN);
  screen.println(timeClient.getFormattedTime());

  screen.setTextSize(4);
  screen.setTextColor(WHITE, DARKCYAN);
  screen.setCursor(50, 45);
  screen.print(watts);
  screen.print(" ");
  screen.setCursor(50, 85);
  screen.print(voltage);
  screen.print(" ");
  screen.setCursor(50, 125);
  screen.print(ampereSum, 1);
  screen.print(" ");
  screen.setTextSize(3);
  screen.setTextColor(WHITE, LIGHTCYAN);
  screen.setCursor(80, 170);
  screen.print(watt_L1);
  screen.print(" ");
  screen.setCursor(80, 210);
  screen.print(watt_L2);
  screen.print(" ");
  screen.setCursor(80, 250);
  screen.print(watt_L3);
  screen.print(" ");
}

String DisplayIpAddress(IPAddress ip)
{
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}



