#define SPI_4_WIRE
//#define PAR_8080	  
//#define I2C
//------------------------------------------------------------------------------
//Allow the serial debugging monitor, but only if SPI (pin conflict with parallel)
#define SER_DEBUG
#ifdef PAR_8080
  #undef SER_DEBUG
#endif // PAR_8080
//==============================================================================
#define OLED_RESET_PIN 2
#define OLED_CS_PIN    3
#define OLED_DC_PIN    4

#define CLR_CS    digitalWrite(OLED_CS_PIN, LOW)
#define SET_CS    digitalWrite(OLED_CS_PIN, HIGH)

#define CLR_RESET digitalWrite(OLED_RESET_PIN, LOW)
#define SET_RESET digitalWrite(OLED_RESET_PIN, HIGH)

#define CLR_DC    digitalWrite(OLED_DC_PIN, LOW)
#define SET_DC    digitalWrite(OLED_DC_PIN, HIGH)

#define OLED_WIDTH 128
#define OLED_PAGES 7

uint8_t OLED_FrameBuffer[OLED_PAGES][OLED_WIDTH];

#include <avr/io.h>
#ifdef SPI_4_WIRE
  #include <SPI.h>
#endif
#ifdef I2C
  #include <Wire.h>
#endif
#include "CFAL12856A0_0151_B_Splash.h"
#include "DCU.h"
//================================================================================
#define MAX_BRIGHT (0x8F)
//================================================================================
void writeCommand(uint8_t command)
{
  // Select the LCD's command register
  CLR_DC;
  // Select the LCD controller
  CLR_CS;

  //Send the command via SPI:
  SPI.transfer(command);
  //deselect the controller
  SET_CS;
}
//================================================================================
void writeData(uint8_t data)
{
  //Select the LCD's data register
  SET_DC;
  //Select the LCD controller
  CLR_CS;
  //Send the command via SPI:
  SPI.transfer(data);

  // Deselect the LCD controller
  SET_CS;
}
//================================================================================

#ifdef I2C
//================================================================================
void writeCommand(uint8_t command)
{
	Wire.beginTransmission(0x3C); //send start & Slave address
	Wire.write(0x00);		          //Control Byte - Command
	Wire.write(command);	        //payload
	Wire.endTransmission();

}
//============================================================================
void writeData(uint8_t data)
{
	Wire.beginTransmission(0x3C);	//send start & Slave address
	Wire.write(0xC0);		          //Control Byte - Data (non-continued)
	Wire.write(data);          //payload
	Wire.endTransmission();
}
#endif
//================================================================================

#ifdef PAR_8080
void writeCommand(uint8_t command)
  {
  //select the LCD's command register
  CLR_DC;
  //select the LCD controller
  CLR_CS;
  //send the data via parallel
  PORTD = command;
  //clear the write register
  CLR_WR;
  //set the write register
  SET_WR;
  //deselct the LCD controller
  SET_CS;
  }
//================================================================================
void writeData(uint8_t data)
  {
  //select the LCD's data register
  SET_DC;
  //select the LCD controller
  CLR_CS;
  //send the data via parallel
  PORTD = data;
  //clear the write register
  CLR_WR;
  //set the write register
  SET_WR;
  //deselct the LCD controller
  SET_CS;
  }
//================================================================================
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Start_Column(uint8_t d)
{
  writeCommand(0x00 + d % 16);		// Set Lower Column Start Address for Page Addressing Mode
            //   Default => 0x00
  writeCommand(0x10 + d / 16);		// Set Higher Column Start Address for Page Addressing Mode
            //   Default => 0x10
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Column_Address(uint8_t a, uint8_t b)
{
  writeCommand(0x21);			// Set Column Address
  writeCommand(a);			//   Default => 0x00 (Column Start Address)
  writeCommand(b);			//   Default => 0x7F (Column End Address)
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Page_Address(uint8_t a, uint8_t b)
{
  writeCommand(0x22);			// Set Page Address
  writeCommand(a);			//   Default => 0x00 (Page Start Address)
  writeCommand(b);			//   Default => 0x07 (Page End Address)
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Start_Page(uint8_t d)
{
  writeCommand(0xB0 | d);			// Set Page Start Address for Page Addressing Mode
            //   Default => 0xB0 (0x00)
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM(uint8_t Data)
{
uint8_t
  i;
uint8_t
  j;

for (i = 0; i < 8; i++)
  {
  Set_Start_Page(i);
  Set_Start_Column(0x00);

  for (j = 0; j < 128; j++)
    {
    writeData(Data);
    }
  }
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM_CheckerBoard(void)
{
uint8_t
  page;
uint8_t
  column;

for(page = 0; page < 8; page++)
  {
  Set_Start_Page(page);
  Set_Start_Column(0x00);

  for (column= 0; column < 128; column++)
    {
    if(0 == (column&0x01))
      {
      writeData(0x55);
      }
    else
      {
      writeData(0xAA);
      }
    }
  }
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define XLevelL    0x00
#define XLevelH    0x10
#define XLevel     ((XLevelH&0x0F)*16+XLevelL)
#define HRES       128
#define VRES       56
#define Brightness 0xBF

void OLED_Init()
{
//The CFA10105 has a power-on reset circuit, 
//you can use the following code if you are using GPIO for reset
CLR_RESET;
delay(1);
SET_RESET;
delay(120);

writeCommand(0xFD);	// Set Command Lock
writeCommand(0X12);	//   Default => 0x12
                    //     0x12 => Driver IC interface is unlocked from entering command.
                    //     0x16 => All Commands are locked except 0xFD.

writeCommand(0XAE);	// Set Display On/Off
                    //   Default => 0xAE
                    //     0xAE => Display Off
                    //     0xAF => Display On

                          
writeCommand(0xD5);	// Set Display Clock Divide Ratio / Oscillator Frequency
//  writeCommand(0XA0);	// Set Clock as 116 Frames/Sec
writeCommand(0X30);  // Set Clock as 116 Frames/Sec
                    //   Default => 0x70
                    //     D[3:0] => Display Clock Divider
                    //     D[7:4] => Oscillator Frequency

writeCommand(0xA8);	// Set Multiplex Ratio
writeCommand(0X37); //   Default => 0x3F (1/56 Duty)

writeCommand(0xD3);	// Set Display Offset
writeCommand(0X08);	//   Default => 0x00

writeCommand(0x40); // Set Mapping RAM Display Start Line (0x00~0x3F)
                    //   Default => 0x40 (0x00)

//writeCommand(0xD8);	// Set Low Power Display Mode (0x04/0x05)
//writeCommand(0x05);	//   Default => 0x04 (Normal Power Mode)

writeCommand(0xA1); // Set SEG/Column Mapping (0xA0/0xA1)
                    //   Default => 0xA0
                    //     0xA0 => Column Address 0 Mapped to SEG0
                    //     0xA1 => Column Address 0 Mapped to SEG127

writeCommand(0xC8); // Set COM/Row Scan Direction (0xC0/0xC8)
                    //   Default => 0xC0
                    //     0xC0 => Scan from COM0 to 63
                    //     0xC8 => Scan from COM63 to 0

writeCommand(0xDA); // Set COM Pins Hardware Configuration
writeCommand(0x12); //   Default => 0x12
                    //     Alternative COM Pin Configuration
                    //     Disable COM Left/Right Re-Map

writeCommand(0x81); // Set SEG Output Current
writeCommand(0x8F); // Set Contrast Control for Bank 0

writeCommand(0xD9); // Set Pre-Charge as 2 Clocks & Discharge as 5 Clocks
writeCommand(0x25); //   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
                    //     D[3:0] => Phase 1 Period in 1~15 Display Clocks
                    //     D[7:4] => Phase 2 Period in 1~15 Display Clocks

writeCommand(0xDB); // Set VCOMH Deselect Level
writeCommand(0x34); //   Default => 0x34 (0.78*VCC)

writeCommand(0xA4); // Set Entire Display On / Off
                    //   Default => 0xA4
                    //     0xA4 => Normal Display
                    //     0xA5 => Entire Display On

writeCommand(0xA6); // Set Inverse Display On/Off
                    //   Default => 0xA6
                    //     0xA6 => Nc:\Users\wdunc\OneDrive\Documents\Arduino\Capstone Embedded\sample_oled_out\sample_oled_out.hormal Display
                    //     0xA7 => Inverse Display On

Fill_RAM(0x00);			// Clear Screen

writeCommand(0XAF); // Display On (0xAE/0xAF)
}
uint8_t reverseBits(uint8_t b)
{
    b = (b >> 4) | (b << 4);
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);

    return b;
}

void showImage(const uint8_t image[7][128])
{
  for (uint8_t y = 0; y < 7; y++)
  {
    Set_Start_Page(y);
    Set_Start_Column(0x00);

    for (uint8_t x = 0; x < 128; x++)
    {
      uint8_t data = image[6 - y][127 - x];

      writeData(reverseBits(data));
    }
  }

  Set_Start_Page(7);
  Set_Start_Column(0x00);

  for (uint8_t x = 0; x < 128; x++)
  {
      writeData(0x00);
  }
}

// void printNumber(int number, uint8_t page, uint8_t column)
// {
//     char buffer[12];

//     itoa(number, buffer, 10);

//     Set_Start_Page(page);
//     Set_Start_Column(column);

//     for (uint8_t i = 0; buffer[i] != '\0'; i++)
//     {
//         char c = buffer[i];

//         if (c >= '0' && c <= '9')
//         {
//             uint8_t digit = c - '0';

//             for (uint8_t x = 0; x < 5; x++)
//             {
//                 writeData(pgm_read_byte(&numberFont[digit][x]));
//             }

//             // Space between digits
//             writeData(0x00);
//         }
//         else if (c == '-')
//         {
//             for (uint8_t x = 0; x < 5; x++)
//                 writeData(0x08);

//             writeData(0x00);
//         }
//     }
// }

// void printChar(char character, uint8_t page, uint8_t column)
// {
//     // Convert lowercase letters to uppercase
//     if (character >= 'a' && character <= 'z') {
//         character -= 32;
//     }

//     // Check valid alphabet range
//     if (character < 'A' || character > 'Z') {
//         return;
//     }

//     uint8_t index = character - 'A';

//     Set_Start_Page(page);
//     Set_Start_Column(column);

//     // Send the 5 columns of the character
//     for (uint8_t x = 0; x < 5; x++) {
//         writeData(pgm_read_byte(&alphabetFont[index][x]));
//     }

//     // Add one blank column between letters
//     writeData(0x00);
// }

// void printMisc(char character, uint8_t page, uint8_t column)
// {
//   int selected_c = 0;
//   if (character == ':')
//     selected_c = 0;
//   else if (character == '/')
//     selected_c = 1;
//   else
//     return;
  
//   Set_Start_Page(page);
//   Set_Start_Column(column);
//   for (uint8_t x = 0; x < 5; x++) {
//     writeData(pgm_read_byte(&miscFont[selected_c][x]));
//   }
//   writeData(0x00);
// }

void LoadFullFrameTemplate()
{
    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {
        for (uint8_t column = 0; column < OLED_WIDTH; column++)
        {
            OLED_FrameBuffer[page][column] =
                pgm_read_byte(&FullFrameLeftOled[page][column]);
        }
    }
}

void FramePrintChar(char character, uint8_t page, uint8_t column)
{
  if (page >= OLED_PAGES || column >= OLED_WIDTH)
    return;

  if (character >= 'a' && character <= 'z')
    character -= 32;

  const uint8_t* font = nullptr;

  if (character >= 'A' && character <= 'Z')
  {
    font = alphabetFont[character - 'A'];
  }
  else if (character >= '0' && character <= '9')
  {
    font = numberFont[character - '0'];
  }
  else if (character == ':')
  {
    font = miscFont[0];
  }
  else if (character == '/')
  {
    font = miscFont[1];
  }

  for (uint8_t x = 0; x < 6; x++)
  {
    uint8_t targetColumn = column + x;

    if (targetColumn >= OLED_WIDTH)
        break;

    uint8_t data = 0x00;

    if (font != nullptr && x < 5)
    {
        data = pgm_read_byte(font + x);
    }
    else if (character == '-' && x < 5)
    {
        data = 0x08;
    }

    OLED_FrameBuffer[page][targetColumn] = data;
  }
}

void FramePrintNumber(int number, uint8_t page, uint8_t column)
{
    char buffer[12];

    itoa(number, buffer, 10);

    FramePrintText(buffer, page, column);
}

void FramePrintText(const char* text, uint8_t page, uint8_t column)
{
    while (*text != '\0')
    {
        if (column >= OLED_WIDTH)
            break;

        FramePrintChar(*text, page, column);

        column += 6;
        text++;
    }
}

void HandleOLEDSetup () {
  //Set up port directions
  DDRD = 0xff;
  DDRC = 0xff;
  DDRB = 0x03;
  //Idle the lines in a reasonable state
  PORTD = 0xff;
  CLR_RESET;
  SET_RESET;
  SET_CS;
  CLR_DC;

  pinMode(OLED_RESET_PIN, OUTPUT);
  pinMode(OLED_CS_PIN, OUTPUT);
  pinMode(OLED_DC_PIN, OUTPUT);

  #ifdef SER_DEBUG
    Serial.begin(9600);
    Serial.println();
    Serial.println("Serial Initialized");
  #endif // SER_DEBUG

  #ifdef SPI_4_WIRE
    //SPI begin transactions takes ~2.5us
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  #endif
  
    //OLED_Init takes ~120ms
    OLED_Init();
  #ifdef SER_DEBUG
    Serial.println("Init done");
  #endif // SER_DEBUG
  Fill_RAM(0x00);
}

void HandleOLEDUpdate(int RPM_Data)
{
  static int temp_RPM = RPM_Data;
  int digit_4[4];
  int digit_3[3];
  
  LoadFullFrameTemplate();

  FramePrintText("RPM:", 1, 5);

  if (RPM_Data > 9999) {
    RPM_Data = 9999;
    FramePrintNumber(RPM_Data, 1, 30);
  }
  if (RPM_Data >= 100 && RPM <= 999) {
    FramePrintNumber(RPM_Data, 1, 30);
  }
  else if (RPM_Data >= 1000 && RPM <= 9999)
  {
    FramePrintNumber(RPM_Data, 1, 30);
  }
  else
    FramePrintText("N/A", 1, 30);

  showImage(OLED_FrameBuffer);
}
