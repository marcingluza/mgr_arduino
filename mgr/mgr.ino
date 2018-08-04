
#define WHITE 0xFFFF
#define BLACK 0x0000
#define CYAN 0xF5ED
#define DARKCYAN 0xD5A4
#define GREEN 0x4655
#define RED 0x425A

//LCD size
#define TFTWIDTH   240
#define TFTHEIGHT  320
//LCD pins
#define LCD_RD   A0 //A0 - read
#define LCD_WR   A1 //A1 - write    
#define LCD_RS   A2 //Define A2 as LCD_RS (0=Command/1=Data)       
#define LCD_CS   A3 //Define A3 as LCD_CS (Chip Select)     
#define LCD_RST  A4 //Define A4 as LCD_RST (Shield Reset, NOT to the Reset Button)

void Lcd_Writ_Bus(unsigned char d)
{
  PORTH &= ~(0x78);
  PORTH |= ((d & 0xC0) >> 3) | ((d & 0x3) << 5);
  PORTE &= ~(0x38);
  PORTE |= ((d & 0xC) << 2) | ((d & 0x20) >> 2);
  PORTG &= ~(0x20);
  PORTG |= (d & 0x10) << 1;
  *(portOutputRegister(digitalPinToPort(LCD_WR))) &=  ~digitalPinToBitMask(LCD_WR);//LCD_WR=0
  *(portOutputRegister(digitalPinToPort(LCD_WR))) |=  digitalPinToBitMask(LCD_WR); //LCD_WR=1
}

//-------------------------------------------------------------------------------------------
void Lcd_Write_Com(unsigned char VH)
{
  *(portOutputRegister(digitalPinToPort(LCD_RS))) &=  ~digitalPinToBitMask(LCD_RS);//LCD_RS=0 (Set Control line for Command)
  Lcd_Writ_Bus(VH);//Write Hex Value of COM Register passed to Lcd_Write_Com.
}

//-------------------------------------------------------------------------------------------
void Lcd_Write_Data(unsigned char VH)
{
  *(portOutputRegister(digitalPinToPort(LCD_RS))) |=  digitalPinToBitMask(LCD_RS); //LCD_RS=1 (Set control line for Data)
  Lcd_Writ_Bus(VH);//Write Hex Value of Data passed to Lcd_Write_Data.
}
//-------------------------------------------------------------------------------------------
void Address_set(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  Lcd_Write_Com(0x2a);//REG 2Ah=COLADDRSET Command 4 Values.
  Lcd_Write_Data(x1 >> 8); //Send Value of x1 with right bit shift x8.
  Lcd_Write_Data(x1);//Send Value of x1.
  Lcd_Write_Data(x2 >> 8); //Send Value of x2 with right bit shift x8.
  Lcd_Write_Data(x2);//Send Value of x2.
  Lcd_Write_Com(0x2b);//REG 2Bh=PAGEADDRSET Command 4 Values.
  Lcd_Write_Data(y1 >> 8); //Send Value of y1 with right bit shift x8.
  Lcd_Write_Data(y1);//Send Value of y1.
  Lcd_Write_Data(y2 >> 8); //Send Value of y2 with right bit shift x8.
  Lcd_Write_Data(y2);//Send Value of y2.
  Lcd_Write_Com(0x2c); // REG 2Ch = Memory Write
}
//-------------------------------------------------------------------------------------------
//Init commands for Shield.
void Lcd_Init(void)
{
  digitalWrite(LCD_RST, HIGH); // Take line HIGH.
  delay(5); //delay of 5mS.
  digitalWrite(LCD_RST, LOW); //Take line LOW to Reset Shield, Active LOW.
  delay(15);//delay of 15mS.
  digitalWrite(LCD_RST, HIGH); //Take Line HIGH allow Shield to function.
  delay(15);//delay of 15mS.

  digitalWrite(LCD_CS, HIGH); //Take Chip Select HIGH, Disable Shield Chip, Active LOW.
  digitalWrite(LCD_WR, HIGH); //Take Write line HIGH.
  digitalWrite(LCD_CS, LOW); //Take Chip Select LOW, Enable Shield Chip, Active LOW.

  Lcd_Write_Com(0xC0);    //Power control 1
  Lcd_Write_Data(0x23);   //VRH[5:0] 0010 0011 4.60 Volts, default is 4.50 Volts.

  Lcd_Write_Com(0xC1);    //Power control 2, Sets the factor used in the step-up circuits
  Lcd_Write_Data(0x10);   //BT[2:0] 0001 0000 default is 10

  Lcd_Write_Com(0xC5);    //C5h VCOM control 1 (Contrast?)
  Lcd_Write_Data(0x3e);   //VMH 0011 1110 +4.250 Volts
  Lcd_Write_Data(0x28);   //VML 0010 1000 -1.500 Volts

  Lcd_Write_Com(0xC7);    //C7h VCOM control2 Set the VCOM offset voltage.
  Lcd_Write_Data(0x86);   //1000 0110 VML=58 VMH=58

  Lcd_Write_Com(0x36);    //MADCTL, Memory Access Control.
  Lcd_Write_Data(0x40);   //Original Value 48, For RGB-BGR Flip D3 40h

  Lcd_Write_Com(0x3A);    //COLMOD (Interface Pixel Format)
  Lcd_Write_Data(0x55);   //RGB Interface, Color Format 16Bit,Control Interface Color Format 16bit.

  Lcd_Write_Com(0xB1);    //B1h, Frame Rate Control.
  Lcd_Write_Data(0x00);   //[1:0] DIVA division ratio 00 = fosc
  Lcd_Write_Data(0x18);   //[4:0] RTNA Frame Rate 18h 0001 1000 = 79Hz

  Lcd_Write_Com(0xB6);    //B6h Display Function Control.
  Lcd_Write_Data(0x08);   //08h PTG [3:2] & PT [1:0] 0000 1000
  
  Lcd_Write_Data(0x82);   //1000 0010 REV[D7] GS[D6] SS[D5] SM[D4] ISC[3:0]=5Frames 85mS.
  Lcd_Write_Data(0x27);   //0010 0111 NL[5:0]

  Lcd_Write_Com(0x11);    //Wake, Exit Sleep.
  delay(120);             //120mS Delay to allow shield to settle.

  Lcd_Write_Com(0x29);    //Display on

  Lcd_Write_Com(0x2c);    //Memory Write.
}

void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)
{
  unsigned int i, j;
  Lcd_Write_Com(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  Address_set(x, y, l, y);
  j = l * 2;
  for (i = 1; i <= j; i++)
  {
    Lcd_Write_Data(c);
  }
}
//-------------------------------------------------------------------------------------------
//Draw Filled Rectangle.
void Rectf(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c)
{
  unsigned int i;
  for (i = 0; i < h; i++)
  {
    H_line(x  , y  , w, c);
    H_line(x  , y + i, w, c);
  }
}
//-------------------------------------------------------------------------------------------

void setup()
{
  DDRH |= 0x78;//0111 1000
  DDRE |= 0x38;//0011 1000
  DDRG |= 0x20;//0010 0000
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);

  Lcd_Init();

  delay(1000);
    Rectf(0, 0, 240, 320, WHITE);
    Rectf(0, 0, 240, 304, DARKCYAN);
    Rectf(0, 0, 240, 208, WHITE);
    Rectf(0, 0, 240, 16, DARKCYAN);
  delay(1000);
}
//-------------------------------------------------------------------------------------------
void loop()
{



  //while(true);//Stop Sketch Here if required.
}
//-------------------------------------------------------------------------------------------
//End of Sketch.
