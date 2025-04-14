#include "EPD_My_sdfat.h"
#include "GUI_Paint.h"
#include "EPD_SDCard.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

char temp128[128];
const int triggerDelay = 300;

#include <Wire.h>
#define disk 0x50  //адрес чипа FM24C

#define WEMOS_PMOS_PIN 11

// Which pin on the Arduino is connected to the NeoPixels?
#define NEO_PIN 10  // todo: change to 14/15
// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(1, NEO_PIN, NEO_GRB + NEO_KHZ800);

#include "Adafruit_EEPROM_I2C.h"
#include "Adafruit_FRAM_I2C.h"

Adafruit_EEPROM_I2C i2ceeprom;

bool applyRevert = false;

#define EEPROM_ADDR 0x50  // the default address!

int FramReadInt(int addr) {
  uint8_t buffer[2];  // floats are 4 bytes!
  int ret = 0;
  i2ceeprom.read(addr, buffer, 2);
  memcpy((void*)&ret, buffer, 2);
  return ret;
}

long FramReadLong(int addr) {
  uint8_t buffer[4];  // floats are 4 bytes!
  long ret = 0;
  i2ceeprom.read(addr, buffer, 4);
  memcpy((void*)&ret, buffer, 4);
  return ret;
}
void FramWriteInt(int addr, int val) {
  uint8_t buffer[2];  // floats are 4 bytes!
  memcpy(buffer, (void*)&val, 2);
  i2ceeprom.write(addr, buffer, 2);
}
void FramWriteLong(int addr, long val) {
  uint8_t buffer[4];  // floats are 4 bytes!
  memcpy(buffer, (void*)&val, 4);
  i2ceeprom.write(addr, buffer, 4);
}
//#define USE_DEBUG 0

//FRAM addresses
const int TotalPagesFramAddress = 0;
const int CounterPagesFramAddress = 4;
const int BookmarkFlagsRegisterFramAddress = 8;
const int BookmarkPageFramAddress = 10;
const int BookmarkFilenameFramAddress = 12;

///////

void FM24C_write(unsigned int startAddress, void* data, unsigned int len) {  // адрес, указатель на первый байт, длина в байтах
  Wire.beginTransmission(disk);
  Wire.write((byte*)&startAddress, 2);
  Wire.write((byte*)data, len);
  Wire.endTransmission(true);
  delay(1);
}

int FM24C_read(unsigned int startAddress, void* data, unsigned int len) {  // возвращаем кол-во считанных байт
  byte rdata;
  byte* p;
  Wire.beginTransmission(disk);
  Wire.write((byte*)&startAddress, 2);
  Wire.endTransmission();
  //Wire.beginTransmission(disk);
  Wire.requestFrom(disk, len);
  for (rdata = 0, p = (byte*)data; Wire.available() && rdata < len; rdata++, p++) {
    *p = Wire.read();
  }
  return (rdata);
}

void WriteString(int addr, String str) {


  // Serial.print("strlen:");
  int len = str.length();
  // Serial.println(len);
  FM24C_write(addr, &len, sizeof(len));  // адрес 10

  const char* ss = str.c_str();
  for (int i = 0; i < len / 16; i++) {
    FM24C_write(addr + sizeof(len) + i * 16, ss + i * 16, 16 + 1);  // записываем по адресу 1500*/
  }
  if (len % 16 > 0) {
    FM24C_write(addr + sizeof(len) + (len / 16) * 16, ss + (len / 16) * 16, len % 16 + 1);  // записываем по адресу 1500*/
  }
}

String ReadString(int addr) {
  int len;
  FM24C_read(addr, &len, sizeof(len));  // из тех же адресов, что записывали!
  //Serial.println(b);

  char hhh[256];
  for (int i = 0; i < len / 16; i++) {
    int br = FM24C_read(addr + sizeof(len) + i * 16, hhh + i * 16, 16 + 1);  // считываем из того же адреса
    //Serial.print("bytes readed:");
    // Serial.println(br);
  }
  if (len % 16 > 0) {
    int br = FM24C_read(addr + sizeof(len) + (len / 16) * 16, hhh + (len / 16) * 16, len % 16 + 1);  // считываем из того же адреса
    //Serial.print("bytes readed:");
    //Serial.println(br);
  }
  return String(hhh);
}
//Analog port 4 (A4) = SDA (serial data)
//Analog port 5 (A5) = SCL (serial clock)
#define SIGNAL_PATH_RESET 0x68
#define I2C_SLV0_ADDR 0x37
#define ACCEL_CONFIG 0x1C
#define MOT_THR 0x1F  // Motion detection threshold bits [7:0]
#define MOT_DUR 0x20  // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define MOT_DETECT_CTRL 0x69
#define INT_ENABLE 0x38
#define WHO_AM_I_MPU6050 0x75  // Should return 0x68
#define INT_STATUS 0x3A
//when nothing connected to AD0 than address is 0x68
#define ADO 0
#if ADO
#define MPU6050_ADDRESS 0x69  // Device address when ADO = 1
#else
#define MPU6050_ADDRESS 0x68  // Device address when ADO = 0
#endif
//#include "EPD_5in83.h"
//#include "utility/EPD_5in83.h"

bool wifiMode = false;
#include <Arduino.h>
#include <U8g2lib.h>

//#ifdef U8X8_HAVE_HW_SPI
//#include <SPI.h>
//#endif
//#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
//#endif

#include <INA219_WE.h>
#define I2C_ADDRESS 0x40
/*U8G2_SSD1306_128X64_NONAME_F_HW_I2C*/
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);  // All Boards without Reset of the Display
INA219_WE ina219 = INA219_WE(I2C_ADDRESS);

const int max_characters = 80;
char f_name[max_characters];

//const int buttonPin3 = 19;  // the number of the pushbutton pin
const int buttonPin = 19;  // 19 for old panel driver2

byte format1 = 0;  //format of opened cb book
long bookInfoSectionOffset;
long pagesArraySectionOffset;
long pagesTableSectionOffset;

const int wemosPin = 30;
const int wemosRstPin = 31;
//const int buttonPin2 = 35;     // the number of the pushbutton pin
File file;

bool outdoorMode = false;

String root_dir = "/";
sFONT* font = &Font12;

int fontHeight = 12;
//int cols=54;//35 for font24; 54 for font16;85 for font12
//int rows=22;//17 for font24; 27 for font16; 29 for font12(gap=3)
int cols = 80;  //35 for font24; 54 for font16;85 for font12
int rows = 25;  //17 for font24; 27 for font16; 29 for font12(gap=3)
int page = 1;
int pages = 778;
//int gap=2;
int gap = 3;
//int fontWidth=11;//11 for font16, 17 for font24, 7 for font12
int fontWidth = 7;  //11 for font16, 17 for font24, 7 for font12

int bookMenuPos = 0;      //0- back to book,1-goto page menu, 2- close book
int bootMenuPos = 0;      //0 - load bookmarks list, 1- load files list, 2- settings, 3- wifi
int settingsMenuPos = 0;  //
int ledMenuIdx = 0;       //
int chaptersMenuIdx = 0;  //
int statisticsMenuIdx = 0;
int bookmarkMenuPos = 0;
int totalBookmarks = 0;
int totalFastBookmarks = 0;
int fastbookmarkLevels = 0;  // level of binary tree
int maxPriority = 0;
int minPriority = 0;
int minPriorityIdx = 0;

UWORD Image_Width_Byte;
UWORD Bmp_Width_Byte;
int width;
int bookWidth;
int height;
int bookHeight;
unsigned long tocRawSize;
int tocItems;


extern SdFat sd;

#define MFILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
#define MFILE_READ (O_RDONLY)

const size_t LINE_DIM = 550;
char line[LINE_DIM];

int cbPage = 0;

String currentBook = "";
int currentBookIdx = 0;

enum class menuModeEnum {
  files,
  textBook,      //1-text book inside,
  CB,            //2- CB book inside
  bookMenu,      //3-book menu(goto page,back to files),
  gotoPageMenu,  //4-goto page menu,
  bootMenu,      // 5-boot menu,
  bookmarks,     // 6 - bookmarks list
  bmp,           // 7- bmp image,
  settings,      //8- settings list
  statistics,    //9- statistics menu
  led

} menuMode;


int gotoPage = 0;
int gotoPageMenu = 0;
//bool hasBookmark=false;

void saveBookmark(int page) {
  //Serial.println("saveb");
  char buffer[380];
  maxPriority++;
  sprintf(buffer, "%s;%6d;%6d", (root_dir + currentBook).c_str(), page, maxPriority);
  //Serial.print(buffer);
  int p = getBookmark();
  //Serial.println(p);

  if (p == -1) {
    File fileB = sd.open("bookmarks.txt", MFILE_WRITE);
    fileB.println(buffer);
    //Serial.print("puts line");
    fileB.close();
    //   hasBookmark=true;
  } else {
    replaceBookmark(page);
  }
}

int replaceBookmark(int page) {
  //Serial.println("replaceBookmark");
  char buffer[320];
  sprintf(buffer, "%s", (root_dir + currentBook).c_str());
  //Serial.println(buffer);
  File fileB = sd.open("bookmarks.txt", MFILE_READ);
  size_t n;
  // if the file opened okay, write to it:
  if (fileB) {
    long long pos = -1;
    char rep[20];
    while ((n = fileB.fgets(line, sizeof(line))) > 0) {
      char* pch;
      pch = strtok(line, ";");

      //  Serial. printf ("%s\n",pch);
      //   pch = strtok(NULL, ";");
      if (!strcmp(pch, buffer)) {

        maxPriority++;
        auto len = sprintf(rep, "%6d;%6d", page, maxPriority);
        rep[len] = '\0';
        //Serial.print("sprintf len:");
        //Serial.println(len);

        pos = fileB.position() - n + strlen(pch);
        //Serial.print("pos:");
        /*Serial.println((int)(pos));
Serial.print("n:");
Serial.println(n);
Serial.print("rep:");
Serial.println(rep);
Serial.print("strlen(rep):");
Serial.println(strlen(rep));
Serial.print("strlen(pch):");
Serial.println(strlen(pch));*/
        //fileB.seek(pos);  //6+newline
        //fileB.write(rep);


        break;
      }
    }

    fileB.close();
    File fileB = sd.open("bookmarks.txt", O_WRONLY);
    fileB.seek(pos);  //6+newline
    fileB.write(rep, 13);
    fileB.close();
  } else
    // Serial.println("arduino_bookmarks.txt not found");
    return -1;
}

//UDOUBLE SDCard_Read32(File f);
long GetSectionOffset(File& f, byte searchType) {
  auto temp = f.position();
  f.seek(4);
  long pos = 4;
  long ret = -1;
  auto flen = f.size();
  while (pos < flen) {
    auto len = SDCard_Read32(f);
    auto type = file.read();

    if (type == searchType) {
      ret = pos;
      break;
    }


    pos += len;
    f.seek(pos);
  }
  f.seek(temp);
  return ret;
}

int printFile(const char* path) {
  //Serial.print("print file: ");
  // Serial.println(path);
  char buffer[333];

  File fileB = sd.open(path, MFILE_READ);
  int n;
  //Serial.print(buffer);
  if (fileB) {
    while ((n = fileB.fgets(buffer, sizeof(line))) > 0) {
      // Serial.println(buffer);
    }
    fileB.close();
  } else {
    // Serial.println("file not found");
  }
  return -1;
}
int getBookmark() {
  // Serial.println("getb");

  char buffer[280];
  sprintf(buffer, "%s", (root_dir + currentBook).c_str());
  File fileB = sd.open("bookmarks.txt", MFILE_READ);
  size_t n;
  //Serial.print(buffer);
  if (fileB) {
    while ((n = fileB.fgets(line, sizeof(line))) > 0) {
      /*char* pch;
      pch = strtok(line, ";");
      int cntr = 0;*/

      char* pch;
      char* pch2;
      pch = strtok(line, ";");
      pch2 = strtok(NULL, ";");
      int page = atoi(pch2);
      //Serial.println(pch);
      if (!strcmp(pch, buffer)) {
        // Serial.print("found");
        pch = strtok(NULL, ";");
        // Serial.print(pch);


        //Serial.print(page);
        return page;
      }
      //cntr++;
    }
    fileB.close();
  } else {
    //Serial.println("bookmarks.txt not found");
  }
  return -1;
}

void SDCard_ReadCB(const char* Name) {
  //Serial.println("cp1");

#ifdef USE_DEBUG
  Serial.println("SDCard_ReadCB");
  Serial.println(Name);
#endif
  //file = sd.open(Name,O_READ );

  //Serial.println(file.size());
  if (!file.open(Name, O_RDONLY)) {
    //  Serial.println("not found");
    DEBUG("not find : ");
    DEBUG(Name);
    DEBUG("\n");
    return;
  } else {
    DEBUG("open bmp file : ");
    DEBUG(Name);
    DEBUG("\n");
  }

  //if (!SDCard_ReadBmpHeader(bmpFile)) {
  //  DEBUG("read bmp file error\n");
  //  return;
  // }
  UWORD File_Type;
  File_Type = SDCard_Read16(file);  //0000h 2byte: file type

  // Serial.println(File_Type);

  if (File_Type != 0x4243) {
    // magic bytes missing
    return false;
  }

  format1 = file.read();
  auto format2 = file.read();

  //reset toc
  tocItems = 0;
  tocRawSize = 0;

  if (format1 == 0x2) {
    //sections format
    long pos = 4;
    bookInfoSectionOffset = GetSectionOffset(file, 0x10) + 5;
    pagesArraySectionOffset = GetSectionOffset(file, 0xC0);
    pagesTableSectionOffset = GetSectionOffset(file, 0xB0) + 9;
    file.seek(bookInfoSectionOffset);
  }
  int pages = SDCard_Read32(file);
  //  Serial.print("pages: ");
  // Serial.println(pages);

  width = SDCard_Read16(file);
  height = SDCard_Read16(file);

  if (format2 == 0x1) {  //simple toc section
    tocItems = SDCard_Read32(file);
    tocRawSize = SDCard_Read32(file);
    file.seek(12ul + tocRawSize);
  }

  bookWidth = width;
  bookHeight = height;

  UWORD X, Y;
  Image_Width_Byte = (width % 8 == 0) ? (width / 8) : (width / 8 + 1);
  Bmp_Width_Byte = (Image_Width_Byte % 4 == 0) ? Image_Width_Byte : ((Image_Width_Byte / 4 + 1) * 4);
  //unsigned long offset=12ul+5ul*76ul*448ul;
  //file.seek(offset);

  WriteString(BookmarkFilenameFramAddress, String(Name));
  //String str( Name);
  //i2ceeprom.writeObject(BookmarkFilenameFramAddress, String(Name));

  int a = 0;  //0 page
  a = FramReadInt(BookmarkPageFramAddress);
  //FM24C_write(BookmarkPageFramAddress, &a, sizeof(a));

  a = 1;  //flag setted to 1. indicate that book appear
  //  FM24C_write(BookmarkFlagsRegisterFramAddress, &a, sizeof(a));
  FramWriteInt(BookmarkFlagsRegisterFramAddress, a);
}

void resetCB() {
  file.seek(12);
  cbPage = 0;
}

//const unsigned long bytesPerPageCB = 76ul * 448ul;
void skipPagesCB(int cnt) {

  int stride = 4 * (int)ceil(bookWidth / 8 / 4.0f);  //aligned 4

  unsigned long bytesPerPageCB = (unsigned long)stride * (unsigned long)bookHeight;
  //unsigned long bytesPerPageCB = 76ul * 448ul;
  unsigned long offset = (12ul + tocRawSize) + ((unsigned long)cnt) * bytesPerPageCB;

  file.seek(offset);
  cbPage += cnt;
}

extern UBYTE _readBuff[801];

long GetPageOffset(File& file, int page) {
  file.seek(pagesTableSectionOffset + page * 4);
  auto pos = SDCard_Read32(file);
  return pagesArraySectionOffset + pos;
}

void fastNextPageCB() {

  if (format1 == 0x2) {
    //seek to page first to all

    //get page offset
    auto pOffset = GetPageOffset(file, cbPage);
    file.seek(pOffset);
    auto pageType = file.read();
  }
  UWORD X, Y;
  cbPage++;
  UBYTE Data_Black, Data;
  EPD_5IN83_V2_SendCommand(0x10);
  UBYTE ReadBuff[1] = { 0 };


  int Width = (EPD_5IN83_V2_WIDTH % 8 == 0) ? (EPD_5IN83_V2_WIDTH / 8) : (EPD_5IN83_V2_WIDTH / 8 + 1);
  int Height = EPD_5IN83_V2_HEIGHT;

  //EPD_5IN83_V2_SendCommand(0x10);
  for (int i = 0; i < Height; i++) {
    for (int j = 0; j < Width; j++) {
      EPD_5IN83_V2_SendData(0x00);
    }
  }
  EPD_5IN83_V2_SendCommand(0x13);

  for (Y = 0; Y < height; Y++) {  //Total display column
    file.read(_readBuff, Bmp_Width_Byte);
    for (X = 0; X < Bmp_Width_Byte; X++) {  //Show a line in the line
      //file.read(ReadBuff, 1);


      //ReadBuff[0]=reverse(ReadBuff[0]);
      if (X < Image_Width_Byte) {  //bmp
        if (Paint_Image.Image_Color == IMAGE_COLOR_POSITIVE) {

          Data_Black = _readBuff[X];
        } else {
          Data_Black = ~_readBuff[X];
        }
        EPD_5IN83_V2_SendData(Data_Black);
      }
    }
    //bmpFile.read(ReadBuff, 1);
  }

  EPD_5IN83_V2_Power(false);


  int temp = cbPage - 2;
  //FM24C_write(BookmarkPageFramAddress, &temp, sizeof(temp));
  FramWriteInt(BookmarkPageFramAddress, temp);

  //total counter upd
  long total = 0;
  //FM24C_read(TotalPagesFramAddress, &total, sizeof(total));
  total = FramReadLong(TotalPagesFramAddress);
  total++;
  //FM24C_write(TotalPagesFramAddress, &total, sizeof(total));
  FramWriteLong(TotalPagesFramAddress, total);
}



void fastDisplayBuffer() {
  EPD_5IN83_V2_Power(true);
  EPD_5IN83_V2_TurnOnDisplay();
}
void displayBuffer() {
  EPD_5IN83_V2_Display();
  //EPD_5IN83_Sleep();
}
void clearOled() {
  u8g2.clearBuffer();  // clear the internal memory
  u8g2.sendBuffer();   // transfer internal memory to the display
}

void pageBack() {
  if (cbPage == 0) return;
}

void nextPageCB() {
  UWORD X, Y;
  cbPage++;

  UBYTE ReadBuff[1] = { 0 };
  for (Y = 0; Y < height; Y++) {  //Total display column

    for (X = 0; X < Bmp_Width_Byte; X++) {  //Show a line in the line
      file.read(ReadBuff, 1);
      //ReadBuff[0]=reverse(ReadBuff[0]);
      if (X < Image_Width_Byte) {  //bmp
        if (Paint_Image.Image_Color == IMAGE_COLOR_POSITIVE) {
          SPIRAM_WR_Byte((X) + (Y)*Image_Width_Byte + Paint_Image.Image_Offset, ReadBuff[0]);
        } else {
          SPIRAM_WR_Byte((X) + (Y)*Image_Width_Byte + Paint_Image.Image_Offset, ~ReadBuff[0]);
        }
      }
    }
    //bmpFile.read(ReadBuff, 1);
  }
}


void skipPage() {

  u8g2.clearBuffer();  // clear the internal memory
  u8g2.sendBuffer();   // transfer internal memory to the display

  for (int i = 0; i < rows; i++) {

    //String data = file.readString();
    int j;
    for (j = 0; j < cols; j++) {
      unsigned char ch = file.read();

      if (ch == '\n' || ch == '\r') {

        break;
      }
    }
    if (j == 0 && i == 0) {
      i--;
      continue;
    }
  }

  page++;
}

void Paint_DrawString_Flow(UWORD Xstart, const unsigned char* pString,
                           sFONT* Font, UWORD Color_Background, UWORD Color_Foreground) {
  UWORD Xpoint = Xstart;


  if (Xstart > Paint_Image.Image_Width /*|| Ystart > Paint_Image.Image_Height*/) {
    DEBUG("Paint_DrawString_Flow Input exceeds the normal display range\r\n");
    return;
  }
  UBYTE Data_Black, Data;

  int indexb = 0;
  UBYTE accum = 0;
  for (int j = 0; j < Font->Height; j++)
    for (int i = 0; i < 81 * 8; i++) {
      int data = 0;
      if (i < Xstart || i >= (Xstart + strlen(pString) * Font->Width)) {
        data = WHITE;
      } else {

        data = WHITE;
        int symbolIdx = (i - Xstart) / Font->Width;
        int column = (i - Xstart) % Font->Width;
        char ch = pString[symbolIdx];
        int offset = (ch - ' ');
        if (ch > '~') {
          offset = ('~' - ' ') + (ch - 0xC0) + 1;
        }

        int Char_Offset = offset * Font->Height * (Font->Width / 8 + ((Font->Width % 8) != 0 ? 1 : 0));
        const unsigned char* ptr = &Font->table[Char_Offset];
        bool draw = false;
        UWORD Page, Column;
        for (Page = 0; Page < Font->Height; Page++) {
          for (Column = 0; Column < Font->Width; Column++) {
            if (Page == j && Column == column)
              draw = true;

            //To determine whether the font background color and screen background color is consistent
            if (FONT_BACKGROUND == Color_Background) {  //this process is to speed up the scan
              if (draw)
                if ((pgm_read_byte(ptr) & (0x80 >> (Column % 8))) != 0)

                {
                  //Paint_DrawPoint_Flow(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                  data = Color_Foreground;
                }
            } else {
              if (draw)
                if ((pgm_read_byte(ptr) & (0x80 >> (Column % 8))) != 0) {
                  //Paint_DrawPoint_Flow(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                  data = Color_Foreground;
                } else {
                  //Paint_DrawPoint_Flow(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                  data = Color_Background;
                }
            }
            //One pixel is 8 bits
            if (Column % 8 == 7) {
              ptr++;
            }

            if (draw)
              break;
          }
          // Write a line
          if (Font->Width % 8 != 0)
            ptr++;
          if (draw)
            break;
        }
      }

      //setBit(ref accum, indexb, data == BLACK ? 1 : 0);
      if (data == BLACK)
        accum |= (1 << (7 - indexb));
      indexb++;
      if (indexb == 8) {

        indexb = 0;
        EPD_5IN83_V2_SendData(accum);
        accum = 0;
      }
    }
}


void FillWhiteLine(int lines = 1) {


  for (int i = 0; i < Image_Width_Byte * lines; i++) {
    EPD_5IN83_V2_SendData(0x00);
  }
}
void FillBlackLine(int lines = 1) {


  for (int i = 0; i < Image_Width_Byte * lines; i++) {
    EPD_5IN83_V2_SendData(0xff);
  }
}
void nextPage() {
  u8g2.clearBuffer();  // clear the internal memory
  u8g2.sendBuffer();   // transfer internal memory to the display
  Paint_Clear(BLACK);
  for (int i = 0; i < rows; i++) {
    unsigned char data[cols];
    //String data = file.readString();
    int j;
    for (j = 0; j < cols; j++) {
      unsigned char ch = file.read();

      if (ch == '\n' || ch == '\r') {
        data[j] = '\0';
        break;
      }
      data[j] = ch;
    }
    if (j == 0 && i == 0) {
      i--;
      continue;
    }
    data[cols] = '\0';

    Paint_DrawString_Flow(0 /*, i * (fontHeight + gap)*/, data, font, BLACK, WHITE);
  }
  Paint_DrawLine(5, rows * (fontHeight + gap), 595, rows * (fontHeight + gap), BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1);
  String p = "Page: ";
  String p2 = " / ";

  String c = p + page + p2 + pages;

  Paint_DrawString_Flow(0 /*, rows * (fontHeight + gap) + 1*/, c.c_str(), font, BLACK, WHITE);
  EPD_5IN83_V2_Display();
  page++;
}

bool fileExist(const char* path) {

  File fileB = sd.open(path, MFILE_READ);
  if (fileB) {
    fileB.close();
  } else {
    return false;
  }
  return true;
}



bool volatile trigger = false;
void wakeUpNow() {  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP    (i.e. after getting interrupt)
  // execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  // we don't really need to execute any special functions here, since we
  // just want the thing to wake up
  //taps++;
  trigger = true;
  delay(500);
  //Serial.println("WOKEN UP !!!!!!!!!!");
  //delay(500);
  /*int count = 10;
  while (count != 0) {
    delay(1000);
    count--;
    Serial.println(count);
    delay(1000);
  }*/
  // precautionary while we do other stuff
  //detachInterrupt(0);
}
void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t* dest) {
  Wire.beginTransmission(address);  // Initialize the Tx buffer
  Wire.write(subAddress);           // Put slave register address in Tx buffer
  Wire.endTransmission(false);      // Send the Tx buffer, but send a restart to keep connection alive
  uint8_t i = 0;
  Wire.requestFrom(address, count);  // Read bytes from slave register address
  while (Wire.available()) {
    dest[i++] = Wire.read();
  }  // Put read results in the Rx buffer
}
int16_t gyroCount[3];  // Stores the 16-bit signed gyro sensor output
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48
void readGyroData(int16_t* destination) {
  uint8_t rawData[6];                                          // x/y/z gyro register data stored here
  readBytes(MPU6050_ADDRESS, GYRO_XOUT_H, 6, &rawData[0]);     // Read the six raw data registers sequentially into data array
  destination[0] = (int16_t)((rawData[0] << 8) | rawData[1]);  // Turn the MSB and LSB into a signed 16-bit value
  destination[1] = (int16_t)((rawData[2] << 8) | rawData[3]);
  destination[2] = (int16_t)((rawData[4] << 8) | rawData[5]);
}

/*    Example for using write byte
      Configure the accelerometer for self-test
      writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0xF0); // Enable self test on all three axes and set accelerometer range to +/- 8 g */
void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
  Wire.begin();
  Wire.beginTransmission(address);  // Initialize the Tx buffer
  Wire.write(subAddress);           // Put slave register address in Tx buffer
  Wire.write(data);                 // Put data in Tx buffer
  Wire.endTransmission();           // Send the Tx buffer
  //  Serial.println("mnnj");
}

//example showing using readbytev   ----    readByte(MPU6050_ADDRESS, GYRO_CONFIG);
uint8_t readByte(uint8_t address, uint8_t subAddress) {
  uint8_t data;                           // `data` will store the register data
  Wire.beginTransmission(address);        // Initialize the Tx buffer
  Wire.write(subAddress);                 // Put slave register address in Tx buffer
  Wire.endTransmission(false);            // Send the Tx buffer, but send a restart to keep connection alive
  Wire.requestFrom(address, (uint8_t)1);  // Read one byte from slave register address
  data = Wire.read();                     // Fill Rx buffer with result
  return data;                            // Return data read from slave register
}

float loadVoltage_V = 0.0;
unsigned char ledBrightness = 64;
bool ledEnabled = false;
unsigned char ledColor = 0;

int eepromApplyRevertAddr = 0x10;
int eepromLedBrightnessAddr = 0x11;
int eepromLedColorAddr = 0x12;

void readSettingsFromEEPROM() {

  applyRevert = EEPROM.read(eepromApplyRevertAddr) == 1;
  ledBrightness = EEPROM.read(eepromLedBrightnessAddr);
  ledColor = EEPROM.read(eepromLedColorAddr);
}

void (*applyButton)(int dir);
void (*menuButton)(int dir);

void defaultApplyButtonHandler(int dir);
void defaultMenuButtonHandler(int dir);

void setup() {
  applyButton = defaultApplyButtonHandler;
  menuButton = defaultMenuButtonHandler;

  width = EPD_5IN83_V2_WIDTH;
  height = EPD_5IN83_V2_HEIGHT;

  Image_Width_Byte = (width % 8 == 0) ? (width / 8) : (width / 8 + 1);

  // clock_prescale_set(clock_div_2);

  //ADCSRA = 0;
  ADCSRA &= ~(1 << ADEN);


  rows = (448 - 1) / (gap + fontHeight) - 1;
  cols = 600 / fontWidth;

  rows -= 5;
  cols -= 5;

  //pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, LOW);
  if (!ina219.init()) {
    //Serial.println("INA219 not connected!"); b=false;
  }
  float shuntVoltage_mV = 0.0;

  float busVoltage_V = 0.0;



  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();

  loadVoltage_V = busVoltage_V + (shuntVoltage_mV / 1000);
  ina219.powerDown();


  pixels.begin();
  pixels.show();

  if (loadVoltage_V < 3.05) {
    //shut off

    noInterrupts();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // PowerDown - самый экономный режим
    sleep_mode();                         // Переводим МК в сон
    return;
  }

  u8g2.begin();
  u8g2.setContrast(0x5);
  u8g2.setFlipMode(1);
  u8g2.clearBuffer();  // clear the internal memory
                       /*
   
  u8g2.setFont(u8g2_font_logisoso18_tr);  // choose a suitable font
  u8g2.drawStr(0, 25, "V");

  u8g2.setCursor(55, 25);
  u8g2.print(loadVoltage_V);

  u8g2.sendBuffer();
  delay(1000);*/


  /*u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_logisoso18_tr       );	// choose a suitable font
  
  u8g2.drawStr(0,25,"tt World!");	// write something to the internal memory
  u8g2.sendBuffer();*/
  menuMode = menuModeEnum::bootMenu;

  readSettingsFromEEPROM();
  drawBootMenu(true);
  /*if (fileExist("bookmarks.txt")) {
    menuMode = 5;
    drawBootMenu();
  } else {

    menuMode = 0;
    drawFileList();
  }*/

  //pinMode(wemosPin, OUTPUT);

  //pinMode(wemosRstPin, OUTPUT);
  //Serial3.begin(115200);

//digitalWrite(wemosPin,LOW);
//digitalWrite(wemosRstPin,LOW);
//pinMode(buttonPin, INPUT_PULLUP);
//pinMode(buttonPin2, INPUT);
//pinMode(buttonPin3, INPUT_PULLUP);

/////
/*File fileB = sd.open("bookmarks.txt", MFILE_WRITE);         
        if(fileB){
          //Serial.print("remove file");
         fileB.remove();
         }*/
/////////
#ifdef USE_DEBUG
  Serial.begin(115200);
  Serial.println("test");

#endif

  writeByte(MPU6050_ADDRESS, 0x6B, 0x00);
  writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07);  //Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
  writeByte(MPU6050_ADDRESS, I2C_SLV0_ADDR, 0x20);      //write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
  writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x01);       //Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
  writeByte(MPU6050_ADDRESS, MOT_THR, 5);               //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
  writeByte(MPU6050_ADDRESS, MOT_DUR, 40);              //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate
  writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15);    //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
  writeByte(MPU6050_ADDRESS, INT_ENABLE, 0x40);         //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.
  writeByte(MPU6050_ADDRESS, 0x37, 160);                // now INT pin is active low

  pinMode(buttonPin, INPUT_PULLUP);                                       // wakePin is pin no. 2
                                                                          //attachInterrupt(digitalPinToInterrupt(buttonPin), myISR2, RISING);
                                                                          //attachInterrupt(digitalPinToInterrupt(buttonPin3), myISR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin), wakeUpNow, FALLING);  //

  /*
  //root_dir = "/sb/test/test.cb";
  String str = getBookmarkName(0);
  root_dir = str;
  Serial.println(root_dir);
  for (int j = root_dir.length() - 2; j >= 0; j--) {
    if (root_dir[j] == '/') {
      root_dir = root_dir.substring(0, j + 1);
      currentBook = str.substring(j + 1);
      break;
    }
  }
  Serial.println(root_dir);
  Serial.println(currentBook);
*/
  /*
    for(int j=root_dir.length()-2;j>=0;j--){
          if(root_dir[j]=='/'){
            root_dir = root_dir.substring(0,j);
            break;
          }
        
        }
    
    Serial.println(root_dir);*/
}

void initShield() {
  //digitalWrite(wemosPin,HIGH);
  //Serial3.println("sleep");


  //digitalWrite(wemosRstPin,HIGH);
  delay(500);
  DEV_Module_Init();

  EPD_5IN83_V2_Init();
  //EPD_5IN83_Clear();
  //Paint_NewImage(IMAGE_BW, EPD_5IN83_WIDTH, EPD_5IN83_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
  Paint_NewImage(IMAGE_BW, EPD_5IN83_V2_WIDTH, EPD_5IN83_V2_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
  SDCard_Init();




  if (i2ceeprom.begin(EEPROM_ADDR)) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
    //Serial.println("Found I2C EEPROM");
  } else {
    //Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1) delay(10);
  }
}
const int BOOKS_LIMIT = 30;
long poss[BOOKS_LIMIT];
int priorities[BOOKS_LIMIT];
byte sorted_priorities[BOOKS_LIMIT];

String getBookmarkName(int idx) {
  File fileB = sd.open("bookmarks.txt", MFILE_READ);


  size_t n;
  int cntr = 0;
  //Serial.print(buffer);
  if (fileB) {
    //Serial.print("poss:");
    // Serial.println(poss[sorted_priorities[idx]]);
    fileB.seek(poss[sorted_priorities[idx]]);
    //while ((n = fileB.fgets(line, sizeof(line))) > 0)
    n = fileB.fgets(line, sizeof(line));
    {
      char* pch;
      char* pch2;
      pch = strtok(line, ";");
      pch2 = strtok(NULL, ";");
      int page = atoi(pch2);

      //if (page == -1)  //deleted bookmark
      // continue;

      //if (cntr == idx)
      {
        String str = String(pch);
        //Serial.println(str);
        fileB.close();
        return str;
      }
      cntr++;
    }
    fileB.close();
  } else {
    //Serial.println("bookmarks.txt not found");
  }
}


void loadBookmarks() {
  File fileB = sd.open("bookmarks.txt", MFILE_READ);
  size_t n;
  int cntr = 0;

  if (fileB) {
    while ((n = fileB.fgets(line, sizeof(line))) > 0) {
      char* pch;
      char* pch2;
      char* pch3;
      pch = strtok(line, ";");

      pch2 = strtok(NULL, ";");
      int page = atoi(pch2);

      pch3 = strtok(NULL, ";");
      int priority = atoi(pch3);

      if (page == -1)  //deleted bookmark
        continue;

      maxPriority = max(priority, maxPriority);
      minPriority = min(priority, minPriority);
      if (minPriority == priority)
        minPriorityIdx = cntr;

      poss[cntr] = fileB.position() - n - 1;

      priorities[cntr] = priority;
      cntr++;
      if (cntr > BOOKS_LIMIT)
        break;
    }

    //sort priorities

    fileB.seek(0);

    ////

    int maxp = maxPriority;
    int secondp = minPriority;
    int printed = 0;
    while (true) {
      for (int i = 0; i < cntr; i++) {

        fileB.seek(poss[i]);
        n = fileB.fgets(line, sizeof(line));
        char* pch;
        char* pch2;
        char* pch3;
        pch = strtok(line, ";");

        pch2 = strtok(NULL, ";");
        int page = atoi(pch2);

        pch3 = strtok(NULL, ";");
        int priority = atoi(pch3);
        if (priority == maxp) {
          sorted_priorities[printed] = i;
          printed++;
        } else if (priority < maxp)
          secondp = max(secondp, priority);
        //Serial.println(names[sorted_priorities[i]]);
      }

      if (printed == cntr)
        break;

      maxp = secondp;
      secondp = minPriority;
    }
    fileB.close();

    /* draw according priority
    */
  } else {
    //Serial.println("bookmarks.txt not found");
  }


  totalBookmarks = cntr;
  if (cntr > 8) {
    fastbookmarkLevels = 1;
    totalFastBookmarks = 1;
  }
}

void preClear() {

  //2.Drawing on the image
  EPD_5IN83_V2_SendCommand(0x10);
  UBYTE ReadBuff[1] = { 0 };

  int Width = (EPD_5IN83_V2_WIDTH % 8 == 0) ? (EPD_5IN83_V2_WIDTH / 8) : (EPD_5IN83_V2_WIDTH / 8 + 1);
  int Height = EPD_5IN83_V2_HEIGHT;

  //EPD_5IN83_V2_SendCommand(0x10);
  for (int i = 0; i < Height; i++) {
    for (int j = 0; j < Width; j++) {
      EPD_5IN83_V2_SendData(0x00);
    }
  }
  EPD_5IN83_V2_SendCommand(0x13);
}

void flowDrawEnd() {

  //fastDisplayBuffer();
  EPD_5IN83_V2_TurnOnDisplay();
}
void drawBookmarksList() {


  u8g2.clearBuffer();  // clear the internal memory
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic); // choose a suitable font
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);  // choose a suitable font
  //u8g2_font_cu12_t_cyrillic
  u8g2.setContrast(0x5);
  // Serial.print("getb");
  EPD_5IN83_V2_Power(true);
  Paint_Clear(BLACK);

  preClear();

  File fileB = sd.open("bookmarks.txt", MFILE_READ);
  size_t n;
  int cntr = 0;
  //Serial.print(buffer);

  int printed = 0;
  if (fileB) {
    while ((n = fileB.fgets(line, sizeof(line))) > 0) {
      char* pch;
      char* pch2;
      char* pch3;
      pch = strtok(line, ";");

      pch2 = strtok(NULL, ";");
      int page = atoi(pch2);

      pch3 = strtok(NULL, ";");
      int priority = atoi(pch3);

      if (page == -1)  //deleted bookmark

      {
        //Serial.println("skip");
        continue;
      }

      maxPriority = max(priority, maxPriority);
      minPriority = min(priority, minPriority);
      if (minPriority == priority)
        minPriorityIdx = cntr;

      poss[cntr] = fileB.position() - n - 1;
      /*Serial.print("prior ");
      Serial.print(priority);
      Serial.print("poss ");
      Serial.print(poss[cntr]);
      Serial.println();*/
      priorities[cntr] = priority;
      cntr++;
      if (cntr > BOOKS_LIMIT)
        break;
    }

    //sort priorities

    /*for (int i = 0; i < cntr; i++) {
      sorted_priorities[i] = i;
    }

    for (int i = 0; i < cntr; i++) {
      bool was = false;
      int maxp=minPriority;

      for (int j = i + 1; j < cntr; j++) {
        if (priorities[sorted_priorities[i]] < priorities[sorted_priorities[j]]) {
          byte temp = sorted_priorities[i];
          sorted_priorities[i] = sorted_priorities[j];
          sorted_priorities[j] = temp;
          was = true;
        }
      }
      
    }*/

    //Serial.println("cp35");
    for (int i = 0; i < cntr; i++) {
      //   Serial.println(sorted_priorities[i]);
    }
    fileB.seek(0);

    //int pos = fileB.position() - n + strlen(pch);

    // fileB.seek(pos);  //6+newline



    ////
    //Serial.println("cp5");
    int maxp = maxPriority;
    int secondp = minPriority;
    printed = 0;
    while (true) {
      for (int i = 0; i < cntr; i++) {

        fileB.seek(poss[i]);
        n = fileB.fgets(line, sizeof(line));
        char* pch;
        char* pch2;
        char* pch3;
        pch = strtok(line, ";");

        pch2 = strtok(NULL, ";");
        int page = atoi(pch2);

        pch3 = strtok(NULL, ";");
        int priority = atoi(pch3);

        if (priority == maxp) {
          char temp[128];
          sprintf(temp, "%80s   %d", pch, page);
          //Paint_DrawString_Flow(0 /*, printed * fontHeight*/, pch, &Font12, BLACK, WHITE);
          Paint_DrawString_Flow(0 /*, printed * fontHeight*/, temp, &Font12, WHITE, BLACK);

          if (printed == 0)
            u8g2.drawStr(0, 16, pch);  // write something to the internal memory

          sorted_priorities[printed] = i;
          //Serial.println(filename);
          //Paint_DrawString_Flow(500, /*printed * fontHeight, */ String(page).c_str(), &Font12, WHITE, BLACK);
          printed++;
        } else if (priority < maxp)
          secondp = max(secondp, priority);
        //Serial.println(names[sorted_priorities[i]]);
      }

      if (printed == cntr)
        break;

      maxp = secondp;
      secondp = minPriority;
    }
    fileB.close();

    /* draw according priority
    */
  } else {
    //Serial.println("bookmarks.txt not found");
  }


  totalBookmarks = cntr;
  if (cntr > 8) {
    fastbookmarkLevels = 1;
    totalFastBookmarks = 1;
  }
  //display.display();
  u8g2.sendBuffer();  // transfer internal memory to the display
  //EPD_5IN83_V2_Display();

  //fastDisplayBuffer();
  FillWhiteLine(EPD_5IN83_V2_HEIGHT - printed * fontHeight);
  flowDrawEnd();


  EPD_5IN83_V2_Power(false);
}


void drawStatisticsMenu() {
  u8g2.clearBuffer();

  long total = 0;
  //FM24C_read(TotalPagesFramAddress, &total, sizeof(total));
  total = FramReadLong(TotalPagesFramAddress);



  long counter = 0;
  //FM24C_read(CounterPagesFramAddress, &counter, sizeof(counter));
  counter = FramReadLong(CounterPagesFramAddress);

  //u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic); // choose a suitable font
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic);  // choose a suitable font
  u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font
  //u8g2.setFont(u8g2_font_5x8_t_cyrillic);  // choose a suitable font

  //u8g2_font_cu12_t_cyrillic
  u8g2.setContrast(0x5);
  if (statisticsMenuIdx == 1) {
    u8g2.drawStr(0, 12, "reset counter");


    sprintf(temp128, "%d", total - counter);
    u8g2.drawStr(0, 12 + 12 + 4, temp128);

  } else if (statisticsMenuIdx == 2) {
    u8g2.drawStr(0, 12, "back");
  }
  if (statisticsMenuIdx == 0) {

    sprintf(temp128, "All time pages:");
    u8g2.drawStr(0, 12, temp128);
    sprintf(temp128, "%d", total);
    u8g2.drawStr(0, 12 + 12 + 4, temp128);
  }



  //1. total pages  all time
  //2. total pagees from last reset
  //3. reset pages counter (just store current oages all time and then subtract)
  //4. voltage ?
  u8g2.sendBuffer();
}

void drawSettingsList() {
  //EEPROM

  //???3. startup behaviour : automatic resume, menu ?????
  //4.back
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic);

  if (settingsMenuPos == 0)
    u8g2.drawStr(0, 16, "oled brightness");  //1. oled brightness
  else if (settingsMenuPos == 1)
    u8g2.drawStr(0, 16, "resume mode");  //2. resume : from Fram (quick mem) , from sd bookmarks
  else if (settingsMenuPos == 2) {
    u8g2.drawStr(0, 16, "gyr4o X revert");

    sprintf(temp128, "%s", applyRevert ? "true" : "false");
    u8g2.drawStr(0, 32, temp128);

  } else if (settingsMenuPos == 3) {
    u8g2.drawStr(0, 16, "gyro Y revert");
  } else if (settingsMenuPos == 4) {
    u8g2.drawStr(0, 16, "back");
  }

  u8g2.sendBuffer();
}

void drawLedMenu() {
  //EEPROM

  //???3. startup behaviour : automatic resume, menu ?????
  //4.back
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic);

  if (ledMenuIdx == 0) {
    u8g2.drawStr(0, 16, "switch");  // turn on/off
    sprintf(temp128, "%s", ledEnabled ? "on" : "off");
    u8g2.drawStr(0, 32, temp128);

  } else if (ledMenuIdx == 1) {
    u8g2.drawStr(0, 16, "brightness");  // brightness
    sprintf(temp128, "%d", ledBrightness / 16);
    u8g2.drawStr(0, 32, temp128);

  } else if (ledMenuIdx == 2) {
    u8g2.drawStr(0, 16, "color");
    sprintf(temp128, "%d", ledColor);
    u8g2.drawStr(0, 32, temp128);
  } else if (ledMenuIdx == 3) {
    u8g2.drawStr(0, 16, "back");
  }
  u8g2.sendBuffer();
}

void drawChaptersMenu() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic);

  if (chaptersMenuIdx == 0) {
    u8g2.drawStr(0, 16, "[back]");

  } else {
    //read toc item
    //chaptersMenuIdx

    auto ch_name = GetChapterName(chaptersMenuIdx - 1);

    sprintf(temp128, "%s", ch_name.c_str());
    u8g2.drawStr(0, 16, temp128);
  }
  u8g2.sendBuffer();
}

void drawFileList() {


  u8g2.clearBuffer();  // clear the internal memory
  //u8g2.setFont(u8g2_font_4x6_t_cyrillic); // choose a suitable font
  u8g2.setFont(u8g2_font_4x6_t_cyrillic);  // choose a suitable font
  //u8g2_font_cu12_t_cyrillic
  u8g2.setContrast(0x5);
  EPD_5IN83_V2_Power(true);
  File root = sd.open(root_dir.c_str());
  Paint_Clear(BLACK);
  preClear();
  int cntr = 0;
  if (root_dir != "/") {

    Paint_DrawString_Flow(0, /*cntr * fontHeight,*/ "..", &Font12, WHITE, BLACK);
    u8g2.drawStr(0, cntr * fontHeight, "..");
    cntr++;
  }

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }

    entry.getName(f_name, max_characters);
    String filename = String(f_name);
    //Serial.println(filename);

    Paint_DrawString_Flow(0, /*cntr * fontHeight,*/ filename.c_str(), &Font12, WHITE, BLACK);
    //u8g2.drawStr(0, cntr * fontHeight, filename.c_str());  // write something to the internal memory

    cntr++;


    if (entry.isDirectory()) {
      //Serial.println("/");
      //printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      // Serial.print("\t\t");
      //Serial.println(entry.size(), DEC);
    }

    entry.close();
  }

  root.close();
  u8g2.clearBuffer();                       // clear the internal memory
  u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font
  currentBook = getFileN(0);

  u8g2.drawStr(0, 16, currentBook.c_str());  // write something to the internal memory

  //display.display();
  u8g2.sendBuffer();  // transfer internal memory to the display
  //EPD_5IN83_V2_Display();
  FillWhiteLine(EPD_5IN83_V2_HEIGHT - cntr * 12);
  flowDrawEnd();
  EPD_5IN83_V2_Power(false);
}



int getTotalFiles() {

  File root = sd.open(root_dir.c_str());

  int cntr = 0;
  if (root_dir != "/") {
    cntr++;
  }
  while (true) {

    File entry = root.openNextFile();
    if (!entry) {
      break;
    }



    cntr++;

    if (entry.isDirectory()) {

    } else {
    }

    entry.close();
  }
  root.close();
  return cntr;
}

bool isDir = false;

String GetChapterName(int idx) {
  file.seek(12ul + 8ul);
  for (int i = 0; i <= idx; i++) {
    //4 page
    SDCard_Read32(file);
    //2 ident
    SDCard_Read16(file);
    //2 len
    int len = SDCard_Read16(file);
    //string[len]
    if (i == idx) {
      memset(temp128, 0, sizeof(temp128));
      file.read(temp128, len);
    } else {
      file.seek(file.position() + len);
    }
    //2 len
    int len2 = SDCard_Read16(file);
    //string[len]
    file.seek(file.position() + len2);
  }

  String name = String(temp128);
  return name;
}

int GetChapterPage(int idx) {
  file.seek(12ul + 8ul);
  int page = 0;
  for (int i = 0; i <= idx; i++) {
    //4 page
    page = SDCard_Read32(file);
    //2 ident
    SDCard_Read16(file);
    //2 len
    int len = SDCard_Read16(file);
    //string[len]
    file.read(temp128, len);

    int len2 = SDCard_Read16(file);
    //string[len]
    file.seek(file.position() + len2);
  }

  return page;
}

String getFileN(int n) {
  isDir = false;
  File root = sd.open(root_dir.c_str());

  int cntr = 0;
  if (root_dir != "/") {
    if (n == 0) {
      isDir = true;
      return "..";
    }
    cntr++;
  }
  while (true) {

    File entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }


    /////////////
    if (cntr == n) {
      if (entry.isDirectory()) {
        isDir = true;
      }

      entry.getName(f_name, max_characters);
      String filename = String(f_name);

      entry.close();
      //return entry.name();
      return filename;
    }
    cntr++;

    if (entry.isDirectory()) {

      //Serial.println("/");
      //printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      //Serial.print("\t\t");
      //Serial.println(entry.size(), DEC);
    }

    entry.close();
  }
  root.close();
  return "";
}

void drawGotoPageMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_t_cyrillic);
  if (gotoPageMenu == 0)
    u8g2.drawStr(0, 16, "up +1");
  if (gotoPageMenu == 1)
    u8g2.drawStr(0, 16, "up +5");
  if (gotoPageMenu == 2)
    u8g2.drawStr(0, 16, "up +10");
  if (gotoPageMenu == 3)
    u8g2.drawStr(0, 16, "up +100");
  if (gotoPageMenu == 4)
    u8g2.drawStr(0, 16, "up -1");
  if (gotoPageMenu == 5)
    u8g2.drawStr(0, 16, "goto");
  if (gotoPageMenu == 6)
    u8g2.drawStr(0, 16, "back");


  u8g2.drawStr(0, 32, "Page: ");
  u8g2.setCursor(64, 32);
  u8g2.print(gotoPage + 1);
  u8g2.sendBuffer();
}

void onePageBack() {
  menuMode = menuModeEnum::CB;
  u8g2.clearBuffer();  // clear the internal memory
  u8g2.sendBuffer();
  int temp = cbPage - 3;
  cbPage = 0;
  skipPagesCB(temp);
  fastNextPageCB();
  fastDisplayBuffer();
  fastNextPageCB();
}

void processBookMenu() {
  if (bookMenuPos == 2) {
    u8g2.clearBuffer();                       // clear the internal memory
    u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font

    u8g2.drawStr(0, 16, "Wait..");  // write something to the internal memory
    u8g2.sendBuffer();              // transfer internal memory to the display

    menuMode = menuModeEnum::files;
    file.close();
    if (outdoorMode) {
      EPD_5IN83_V2_Reset();
      EPD_5IN83_V2_Init();
      Paint_NewImage(IMAGE_BW, EPD_5IN83_V2_WIDTH, EPD_5IN83_V2_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
      SDCard_Init();
    }

    drawFileList();
  } else if (bookMenuPos == 1) {
    menuMode = menuModeEnum::gotoPageMenu;
    gotoPage = 0;
    gotoPageMenu = 0;
    drawGotoPageMenu();
    /*
       u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_cu12_t_cyrillic); // choose a suitable font
  
   u8g2.drawStr(0,16,"Wait..");  // write something to the internal memory
   u8g2.sendBuffer();          // transfer internal memory to the display

   //resetCB();
   skipPagesCB(25);
   nextPageCB();
    menuMode=2;*/
  } else if (bookMenuPos == 0) {
    menuMode = menuModeEnum::CB;
    u8g2.clearBuffer();  // clear the internal memory

    u8g2.sendBuffer();
  } else if (bookMenuPos == 3) {  //print page info
    menuMode = menuModeEnum::CB;
    u8g2.clearBuffer();  // clear the internal memory
    u8g2.sendBuffer();
    int temp = cbPage - 2;
    cbPage = 0;
    skipPagesCB(temp);
    nextPageCB();
    Paint_DrawString_Flow(10, /*310, */ currentBook.c_str(), &Font24, BLACK, WHITE);

    Paint_DrawNum(10, 360, cbPage, &Font24, BLACK, WHITE);

    //3.Refresh the picture in RAM to e-Paper
    //  DEBUG("EPD_5IN83_Display\r\n");
    EPD_5IN83_V2_Display();

  } else if (bookMenuPos == 4) {  //one page back
    onePageBack();

  } else if (bookMenuPos == 5) {  //goto bookmark
    int p = getBookmark();
    if (p != -1) {
      menuMode = menuModeEnum::CB;
      u8g2.clearBuffer();  // clear the internal memory
      u8g2.sendBuffer();

      int temp = p;
      cbPage = 0;
      skipPagesCB(temp);
      fastNextPageCB();
      fastDisplayBuffer();
      fastNextPageCB();
    }

  } else if (bookMenuPos == 6) {  //save bookmark
    saveBookmark(cbPage - 2);
    menuMode = menuModeEnum::CB;
    u8g2.clearBuffer();  // clear the internal memory

    u8g2.sendBuffer();
  } else if (bookMenuPos == 7) {  //remove this book bookmark

    saveBookmark(-1);
    /*  File fileB = sd.open("bookmarks.txt", MFILE_WRITE);
    if (fileB) {
      fileB.remove();
    }*/

    menuMode = menuModeEnum::CB;
    u8g2.clearBuffer();  // clear the internal memory

    u8g2.sendBuffer();
  } else if (bookMenuPos == 8) {  //chapters
    if (tocItems > 0) {
      menuButton = chaptersMenuButtonHandler;
      applyButton = chaptersApplyButtonHandler;
      drawChaptersMenu();
    }
  }
}

void FastReadBMP(const char* BmpName, UWORD Xstart, UWORD Ystart) {
  File bmpFile;
  //bmpFile = sd.open(BmpName,FILE_READ);

  if (!bmpFile.open(BmpName, O_RDONLY)) {
    DEBUG("not find : ");
    DEBUG(BmpName);
    DEBUG("\n");
    return;
  } else {
    DEBUG("open bmp file : ");
    DEBUG(BmpName);
    DEBUG("\n");
  }

  if (!SDCard_ReadBmpHeader(bmpFile)) {
    DEBUG("read bmp file error\n");
    return;
  }
  DEBUG("out\n");
  DEBUG("BMP_Header index\n");
  DEBUG(BMP_Header.Index);
  bmpFile.seek(BMP_Header.Index);

  UWORD X, Y;
  UWORD Image_Width_Byte = (BMP_Header.BMP_Width % 8 == 0) ? (BMP_Header.BMP_Width / 8) : (BMP_Header.BMP_Width / 8 + 1);
  UWORD Bmp_Width_Byte = (Image_Width_Byte % 4 == 0) ? Image_Width_Byte : ((Image_Width_Byte / 4 + 1) * 4);
  DEBUG(Image_Width_Byte);
  DEBUG(Bmp_Width_Byte);

  UBYTE Data_Black, Data;
  UWORD Width, Height;
  Width = (EPD_5IN83_V2_WIDTH % 8 == 0) ? (EPD_5IN83_V2_WIDTH / 8) : (EPD_5IN83_V2_WIDTH / 8 + 1);
  Height = EPD_5IN83_V2_HEIGHT;

  //UBYTE ReadBuff[1] = {0};
  EPD_5IN83_V2_SendCommand(0x10);

  //EPD_5IN83_V2_SendCommand(0x10);
  for (int i = 0; i < Height; i++) {
    for (int j = 0; j < Width; j++) {
      EPD_5IN83_V2_SendData(0x00);
    }
  }

  for (Y = Ystart; Y < BMP_Header.BMP_Height; Y++) {  //Total display column
    bmpFile.read(_readBuff, Bmp_Width_Byte);
    for (X = Xstart / 8; X < Bmp_Width_Byte; X++) {  //Show a line in the line
      //bmpFile.read(ReadBuff, 1);
      if (X < Image_Width_Byte) {  //bmp

        if (Paint_Image.Image_Color == IMAGE_COLOR_POSITIVE) {
          Data_Black = _readBuff[X];
          //SPIRAM_WR_Byte(X + ( BMP_Header.BMP_Height - Y - 1 ) * Image_Width_Byte , _readBuff[X]);
        } else {
          Data_Black = ~_readBuff[X];
          //  SPIRAM_WR_Byte(X + ( BMP_Header.BMP_Height - Y - 1 ) * Image_Width_Byte , ~_readBuff[X]);
        }
        EPD_5IN83_V2_SendData(Data_Black);
      }
    }
  }
  DEBUG("cp3\n");
  bmpFile.close();
  DEBUG("cp4\n");
  EPD_5IN83_V2_TurnOnDisplay();
}

bool b = true;
volatile bool btn1 = false;
volatile bool btn2 = false;

int getFastBookmark(int pos) {
  if (fastbookmarkLevels == 0)
    return -1;
  if (fastbookmarkLevels == 1) {
    if (pos == 0) return totalBookmarks / 2;
  }
  return -1;
}
unsigned long last_interrupt_time_1 = 0;

void myISR() {
  if (wifiMode || btn1) return;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  //if (interrupt_time - last_interrupt_time_1 > 200)
  {
    btn1 = true;
  }
  last_interrupt_time_1 = interrupt_time;
}

bool hasFramBookmark() {
  int b;
  b = FramReadInt(BookmarkFlagsRegisterFramAddress);
  //FM24C_read(BookmarkFlagsRegisterFramAddress, &b, sizeof(b));
  return (b != 0);
}

void loadBookmark(String str) {
  root_dir = str;

  for (int j = root_dir.length() - 2; j >= 0; j--) {
    if (root_dir[j] == '/') {
      root_dir = root_dir.substring(0, j + 1);
      currentBook = str.substring(j + 1);
      break;
    }
  }
  //set currentBook
  SDCard_ReadCB((root_dir + currentBook).c_str());
  menuMode = menuModeEnum::CB;
  int p = getBookmark();

  u8g2.clearBuffer();  // clear the internal memory
  u8g2.sendBuffer();

  int temp = p;
  cbPage = 0;
  skipPagesCB(temp);
  fastNextPageCB();
  fastDisplayBuffer();
  fastNextPageCB();
}

void loadFramBookmark() {
  int page;
  page = FramReadInt(BookmarkPageFramAddress);
  //FM24C_read(BookmarkPageFramAddress, &page, sizeof(page));


  String str = ReadString(BookmarkFilenameFramAddress);
  //String str;
  //i2ceeprom.readObject(BookmarkFilenameFramAddress, str);

  root_dir = str;

#ifdef USE_DEBUG
  //Serial.print("hhh:");
  //Serial.println(hhh);
  Serial.println(str);
  Serial.println(page);
#endif



  for (int j = root_dir.length() - 2; j >= 0; j--) {
    if (root_dir[j] == '/') {
      root_dir = root_dir.substring(0, j + 1);
      currentBook = str.substring(j + 1);
      break;
    }
  }

#ifdef USE_DEBUG
  Serial.println(root_dir);
  Serial.println(currentBook);
#endif

  //set currentBook
  SDCard_ReadCB((root_dir + currentBook).c_str());
  menuMode = menuModeEnum::CB;

  u8g2.clearBuffer();  // clear the internal memory
  u8g2.sendBuffer();

  cbPage = 0;
  skipPagesCB(page);
  fastNextPageCB();
  fastDisplayBuffer();
  fastNextPageCB();
}

void loadBookmark(int bpos) {
  String str = getBookmarkName(bpos);
  loadBookmark(str);
}

void updLedPixels() {
  switch (ledColor) {
    case 0:
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      break;
    case 1:
      pixels.setPixelColor(0, pixels.Color(0, 0, 255));
      break;
    case 2:
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      break;
  }
  pixels.show();
}

void chaptersApplyButtonHandler(int dir) {
  applyButton = defaultApplyButtonHandler;
  menuButton = defaultMenuButtonHandler;
  if (chaptersMenuIdx == 0) {  //back

    //menuButton(1);
    //menuButton(-1);

  } else {
    gotoPage = GetChapterPage(chaptersMenuIdx - 1) - 1;

    GotoPage();
  }
}

void GotoPage() {
  u8g2.clearBuffer();                       // clear the internal memory
  u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font

  u8g2.drawStr(0, 16, "Wait..");  // write something to the internal memory
  u8g2.sendBuffer();              // transfer internal memory to the display
  if (currentBook.endsWith(".CB") || currentBook.endsWith(".cb")) {
    //resetCB();
    cbPage = 0;
    clearOled();
    skipPagesCB(gotoPage);
    fastNextPageCB();
    fastDisplayBuffer();
    fastNextPageCB();
    menuMode = menuModeEnum::CB;
  } else if (currentBook.endsWith(".TXT")) {
    page = gotoPage - 1;
    //file.seek(0);
    unsigned long offset = ((unsigned long)rows * (unsigned long)cols) * (gotoPage - 1);
    file.seek(offset);
    //for(int k=0;k<gotoPage-1;k++)
    //    skipPage();
    nextPage();
    menuMode = menuModeEnum::textBook;
  }
}

void defaultApplyButtonHandler(int dir) {
  //b=!b;
  //digitalWrite(LED_BUILTIN, b?HIGH:LOW);
  u8g2.clearBuffer();  // clear the internal memory

  //u8g2_font_9x15_t_cyrillic
  //u8g2_font_unifont_t_cyrillic
  //u8g2_font_10x20_t_cyrillic
  //u8g2_font_inr24_t_cyrillic
  //u8g2_font_inr27_t_cyrillic

  if (menuMode == menuModeEnum::CB) {
    /*u8g2.setFont(u8g2_font_10x20_t_cyrillic);  // choose a suitable font
    u8g2.drawStr(0, 24, "PAGE: ");             // write something to the internal memory
    u8g2.setCursor(64, 24);
    u8g2.print(cbPage + 1);*/
  } else if (menuMode == menuModeEnum::gotoPageMenu) {

  } else {
    u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font
    u8g2.drawStr(0, 16, "Wait..");            // write something to the internal memory
  }
  u8g2.sendBuffer();  // transfer internal memory to the display

  switch (menuMode) {
    case menuModeEnum::bookmarks:
      {

        /* int fb = getFastBookmark(bookmarkMenuPos);
    if (fb != -1) {
      bookmarkMenuPos = fb;
    } else */
        {
          loadBookmark(bookmarkMenuPos);
        }
        break;
      }
    case menuModeEnum::statistics:
      {
        if (statisticsMenuIdx == 2) {  //back
          menuMode = menuModeEnum::bootMenu;
          drawBootMenu(false);
        } else if (statisticsMenuIdx == 1) {  //reset page counter

          long total = 0;
          total = FramReadLong(TotalPagesFramAddress);
          //FM24C_read(TotalPagesFramAddress, &total, sizeof(total));
          //FM24C_write(CounterPagesFramAddress, &total, sizeof(total));
          FramWriteLong(CounterPagesFramAddress, total);

          drawStatisticsMenu();
        }
        break;
      }
    case menuModeEnum::led:
      {
        switch (ledMenuIdx) {
          case 0:
            ledEnabled = !ledEnabled;
            if (!ledEnabled) {
              pixels.clear();
              pixels.show();
            } else {
              pixels.setBrightness(ledBrightness);
              updLedPixels();
            }
            drawLedMenu();
            break;
          case 1:
            {
              ledBrightness += 16;

              if (ledBrightness > 160) {
                ledBrightness = 0;
              }
              if (ledBrightness == 0) {
                pixels.clear();  // Set all pixel colors to 'off'
              } else {
                pixels.setBrightness(ledBrightness);
              }

              updLedPixels();

              EEPROM.put(eepromLedBrightnessAddr, ledBrightness);
              drawLedMenu();
            }
            break;
          case 2:
            {
              ledColor++;
              ledColor %= 3;

              updLedPixels();
              EEPROM.put(eepromLedColorAddr, ledColor);

              drawLedMenu();
            }
            break;
          case 3:
            {
              menuMode = menuModeEnum::bootMenu;
              drawBootMenu(false);
            }
            break;
        }
        break;
      }
    case menuModeEnum::settings:
      {
        switch (settingsMenuPos) {
          case 0:
            break;
          case 1:
            break;
          case 2:  // gyro x revert
            applyRevert = !applyRevert;

            EEPROM.put(eepromApplyRevertAddr, (byte)(applyRevert ? 1 : 0));
            drawBootMenu(false);
            break;
          case 3:
            break;
          case 4:

            menuMode = menuModeEnum::bootMenu;
            drawBootMenu(false);
            break;
        }
        break;
      }
    case menuModeEnum::bootMenu:
      {
        if (bootMenuPos == 0) {  //resume top bookmark
          initShield();
          loadBookmarks();
          if (hasFramBookmark())
            loadFramBookmark();
          else
            loadBookmark(0);

        } else if (bootMenuPos == 1) {  //draw bookmarks
          menuMode = menuModeEnum::bookmarks;
          initShield();
          drawBookmarksList();

        } else if (bootMenuPos == 2) {  //files
          menuMode = menuModeEnum::files;
          initShield();
          drawFileList();
        } else if (bootMenuPos == 3) {  //settings
          //initShield();
          menuMode = menuModeEnum::settings;
          drawSettingsList();
        } else if (bootMenuPos == 4) {  //wifi
          //enable wifi mode
          wifiMode = true;
          //  digitalWrite(wemosPin,HIGH);
          //digitalWrite(wemosRstPin,HIGH);
          //  Serial3.println("run");
          pinMode(WEMOS_PMOS_PIN, OUTPUT);
          digitalWrite(WEMOS_PMOS_PIN, LOW);

          clearOled();
          noInterrupts();

          set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // PowerDown - самый экономный режим
          sleep_mode();                         // Переводим МК в сон
        } else if (bootMenuPos == 5) {          //statistics
          initShield();
          menuMode = menuModeEnum::statistics;
          statisticsMenuIdx = 0;
          drawStatisticsMenu();
        } else if (bootMenuPos == 6) {  //led

          menuMode = menuModeEnum::led;
          ledMenuIdx = 0;
          drawLedMenu();

          //ledEnabled = !ledEnabled;
        }
        break;
      }
    case menuModeEnum::textBook:
      {

        if (dir > 0) {
          nextPage();
        } else {
          onePageBack();
        }
        break;
      }
    case menuModeEnum::bookMenu:
      {
        processBookMenu();
        break;
      }
    case menuModeEnum::CB:
      {
        //EPD_5IN83_Init();
        if (dir > 0) {
          clearOled();
          //displayBuffer();

          if (outdoorMode) {
            EPD_5IN83_V2_Reset();
            EPD_5IN83_V2_Init();
            Paint_NewImage(IMAGE_BW, EPD_5IN83_V2_WIDTH, EPD_5IN83_V2_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
            SDCard_Init();
            SDCard_ReadCB((root_dir + currentBook).c_str());
            fastNextPageCB();
            fastDisplayBuffer();
            EPD_5IN83_V2_Sleep();
          } else {


            fastDisplayBuffer();
            //nextPageCB();
            //sendToDisplay();
            fastNextPageCB();
          }
        } else {
          //onePageBack();
        }

        break;
      }
    case menuModeEnum::files:
      {
        if (isDir) {
          if (currentBook == "..") {
            for (int j = root_dir.length() - 2; j >= 0; j--) {
              if (root_dir[j] == '/') {
                root_dir = root_dir.substring(0, j + 1);
                break;
              }
            }
          } else {
            root_dir += currentBook + "/";
          }

          currentBookIdx = 0;
          drawFileList();
        } else {


          if (currentBook.endsWith(".CB") || currentBook.endsWith(".cb")) {
            menuMode = menuModeEnum::CB;
            SDCard_ReadCB((root_dir + currentBook).c_str());
            cbPage = 0;
            clearOled();
            fastNextPageCB();
            // nextPageCB();
            fastDisplayBuffer();
            // nextPageCB();
            //sendToDisplay();
            fastNextPageCB();
          } else if (currentBook.endsWith(".TXT")) {
            file = sd.open((root_dir + currentBook), O_READ);
            pages = file.size() / (rows * cols);
            menuMode = menuModeEnum::textBook;
            nextPage();
          } else if (currentBook.endsWith(".bmp") || currentBook.endsWith(".BMP")) {
            menuMode = menuModeEnum::bmp;

            clearOled();

            Paint_Clear(BLACK);
            //FastReadBMP((root_dir + currentBook).c_str(), 0, 0);

            EPD_5IN83_V2_Reset();
            //EPD_5IN83_V2_Init(1); //todo: param add to API
            Paint_NewImage(IMAGE_BW, EPD_5IN83_V2_WIDTH, EPD_5IN83_V2_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
            SDCard_Init();

            SDCard_ReadBMP((root_dir + currentBook).c_str(), 0, 0);
            EPD_5IN83_V2_Display();

            EPD_5IN83_Sleep();
          }
        }
        break;
      }
    case menuModeEnum::gotoPageMenu:
      {
        if (gotoPageMenu == 0) {
          gotoPage++;
          drawGotoPageMenu();
        } else if (gotoPageMenu == 1) {
          gotoPage += 5;
          drawGotoPageMenu();
        } else if (gotoPageMenu == 2) {
          gotoPage += 10;
          drawGotoPageMenu();
        } else if (gotoPageMenu == 3) {
          gotoPage += 100;
          drawGotoPageMenu();
        } else if (gotoPageMenu == 4) {
          gotoPage -= 1;
          drawGotoPageMenu();
        } else if (gotoPageMenu == 5) {
          GotoPage();
        } else if (gotoPageMenu == 6) {
          menuMode = menuModeEnum::bookMenu;
          bookMenuPos = 0;
          drawBookMenu();
        }
        break;
      }
  }
}


void drawBootMenu(bool withVoltage) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  if (bootMenuPos == 0)
    u8g2.drawStr(0, 16, "resume");
  else if (bootMenuPos == 1)
    u8g2.drawStr(0, 16, "bookmarks");
  else if (bootMenuPos == 2) {
    u8g2.drawStr(0, 16, "files");
  } else if (bootMenuPos == 3) {
    u8g2.drawStr(0, 16, "settings");
  } else if (bootMenuPos == 4) {
    u8g2.drawStr(0, 16, "wifi");
  } else if (bootMenuPos == 5) {
    u8g2.drawStr(0, 16, "statistics");
  } else if (bootMenuPos == 6) {
    u8g2.drawStr(0, 16, "led");
  }
  if (withVoltage) {

    //u8g2.setFont(u8g2_font_logisoso18_tr);  // choose a suitable font
    u8g2.drawStr(0, 30, "V");

    u8g2.setCursor(55, 30);
    u8g2.print(loadVoltage_V);
  }
  u8g2.sendBuffer();
}

void drawBookmarkMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);

  String name = getBookmarkName(bookmarkMenuPos);

  u8g2.drawStr(0, 16, name.c_str());

  u8g2.sendBuffer();
}

void drawBookMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font
  if (bookMenuPos == 0)
    u8g2.drawStr(0, 16, "back to book");
  if (bookMenuPos == 1) {
    u8g2.drawStr(0, 16, "goto page");

    u8g2.drawStr(0, 32, "current: ");
    u8g2.setCursor(64, 32);
    u8g2.print(cbPage + 1);
  } else if (bookMenuPos == 2)
    u8g2.drawStr(0, 16, "close book");
  else if (bookMenuPos == 3)
    u8g2.drawStr(0, 16, "print page info");
  else if (bookMenuPos == 4)
    u8g2.drawStr(0, 16, "one page back");
  else if (bookMenuPos == 5)
    u8g2.drawStr(0, 16, "goto bookmark");
  else if (bookMenuPos == 6)
    u8g2.drawStr(0, 16, "save bookmark");
  else if (bookMenuPos == 7)
    u8g2.drawStr(0, 16, "remove bookmark");
  else if (bookMenuPos == 8)
    u8g2.drawStr(0, 16, "chapters");

  u8g2.sendBuffer();
}
unsigned long last_interrupt_time = 0;
void myISR2() {
  if (wifiMode || btn2) return;


  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  //if (interrupt_time - last_interrupt_time > 200)
  {
    btn2 = true;
  }
  last_interrupt_time = interrupt_time;
}

void chaptersMenuButtonHandler(int dir) {
  if (dir > 0) {
    chaptersMenuIdx++;
    if (chaptersMenuIdx == tocItems + 1) chaptersMenuIdx = 0;
  } else {
    chaptersMenuIdx--;
    if (chaptersMenuIdx < 0) chaptersMenuIdx = tocItems;
  }
  drawChaptersMenu();
}

void defaultMenuButtonHandler(int dir) {
  //b=!b;
  //digitalWrite(LED_BUILTIN, b?HIGH:LOW);


  switch (menuMode) {
    case menuModeEnum::textBook:
      {
        u8g2.clearBuffer();                       // clear the internal memory
        u8g2.setFont(u8g2_font_cu12_t_cyrillic);  // choose a suitable font

        u8g2.drawStr(0, 16, "Wait..");  // write something to the internal memory
        u8g2.sendBuffer();              // transfer internal memory to the display
        menuMode = menuModeEnum::bookMenu;
        bookMenuPos = 0;

        drawBookMenu();

        /*menuMode=0;
    file.close();
drawFileList();*/
        break;
      }
    case (menuModeEnum::bmp):
      {  //bmp

        menuMode = menuModeEnum::files;
        EPD_5IN83_V2_Reset();
        EPD_5IN83_V2_Init();

        Paint_NewImage(IMAGE_BW, EPD_5IN83_V2_WIDTH, EPD_5IN83_V2_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);


        SDCard_Init();
        drawFileList();
        break;
      }
    case (menuModeEnum::bookmarks):
      {
        if (dir > 0) {
          bookmarkMenuPos++;
          if (bookmarkMenuPos == totalBookmarks /*+ totalFastBookmarks*/) bookmarkMenuPos = 0;
        } else {
          bookmarkMenuPos--;
          if (bookmarkMenuPos < 0) bookmarkMenuPos = totalBookmarks - 1;
        }
        drawBookmarkMenu();
        break;
      }
    case (menuModeEnum::settings):
      {
        if (dir > 0) {
          settingsMenuPos++;
          if (settingsMenuPos == 5) settingsMenuPos = 0;
        } else {
          settingsMenuPos--;
          if (settingsMenuPos < 0) settingsMenuPos = 4;
        }
        drawSettingsList();
        break;
      }
    case (menuModeEnum::bootMenu):
      {
        if (dir > 0) {
          bootMenuPos++;
          if (bootMenuPos == 7) bootMenuPos = 0;
        } else {
          bootMenuPos--;
          if (bootMenuPos < 0) bootMenuPos = 6;
        }
        drawBootMenu(false);
        break;
      }
    case (menuModeEnum::led):
      {
        if (dir > 0) {
          ledMenuIdx++;
          if (ledMenuIdx == 4) ledMenuIdx = 0;
        } else {
          ledMenuIdx--;
          if (ledMenuIdx < 0) ledMenuIdx = 3;
        }
        drawLedMenu();
        break;
      }
    case (menuModeEnum::statistics):
      {
        if (dir > 0) {
          statisticsMenuIdx++;
          if (statisticsMenuIdx == 3) statisticsMenuIdx = 0;
        } else {
          statisticsMenuIdx--;
          if (statisticsMenuIdx < 0) statisticsMenuIdx = 2;
        }
        drawStatisticsMenu();
        break;
      }
    case (menuModeEnum::bookMenu):
      {
        if (dir > 0) {
          bookMenuPos++;
          if (bookMenuPos == 9) bookMenuPos = 0;
        } else {
          bookMenuPos--;
          if (bookMenuPos < 0) bookMenuPos = 8;
        }
        drawBookMenu();
        break;
      }
    case (menuModeEnum::CB):
      {
        //u8g2.clearBuffer();          // clear the internal memory
        // u8g2.setFont(u8g2_font_cu12_t_cyrillic); // choose a suitable font

        //  u8g2.drawStr(0,16,"Wait..");  // write something to the internal memory
        //  u8g2.sendBuffer();          // transfer internal memory to the display

        menuMode = menuModeEnum::bookMenu;
        bookMenuPos = 0;

        drawBookMenu();
        //file.close();
        //drawFileList();
        break;
      }
    case (menuModeEnum::files):
      {
        if (dir > 0) {
          currentBookIdx++;
          if (currentBookIdx >= getTotalFiles()) {
            currentBookIdx = 0;
          }
        } else {
          currentBookIdx--;
          if (currentBookIdx < 0) {
            currentBookIdx = getTotalFiles() - 1;
          }
        }
        currentBook = getFileN(currentBookIdx);
        //currentBook.toLowerCase();

        u8g2.clearBuffer();  // clear the internal memory
        //u8g2_font_9x15_t_cyrillic
        //u8g2_font_unifont_t_cyrillic
        //u8g2_font_10x20_t_cyrillic
        //u8g2_font_inr24_t_cyrillic
        //u8g2_font_inr27_t_cyrillic
        u8g2.setFont(u8g2_font_10x20_t_cyrillic);  // choose a suitable font
        u8g2.drawStr(0, 16, currentBook.c_str());  // write something to the internal memory
        u8g2.sendBuffer();
        break;
      }
    case (menuModeEnum::gotoPageMenu):
      {
        gotoPageMenu++;
        if (gotoPageMenu >= 7) {
          gotoPageMenu = 0;
        }
        drawGotoPageMenu();
        break;
      }
  }
}

// Set initial input parameters
enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};

enum Gscale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};
// Specify sensor full scale
int Gscale = GFS_250DPS;
int Ascale = AFS_2G;
float aRes, gRes;  // scale resolutions per LSB for the sensors

//===================================================================================================================
//====== Set of useful function to access acceleratio, gyroscope, and temperature data
//===================================================================================================================

void getGres() {
  switch (Gscale) {
      // Possible gyro scales (and their register bit settings) are:
      // 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11).
      // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case GFS_250DPS:
      gRes = 250.0 / 32768.0;
      break;
    case GFS_500DPS:
      gRes = 500.0 / 32768.0;
      break;
    case GFS_1000DPS:
      gRes = 1000.0 / 32768.0;
      break;
    case GFS_2000DPS:
      gRes = 2000.0 / 32768.0;
      break;
  }
}
float gyrox, gyroy, gyroz;  // Stores the real gyro value in degrees per seconds
uint16_t readdata;
int threshold = 100;
int lastTaps;
int t = 0;
void loop() {
  //return;



  interrupts();  //re-enables interrupts

#ifdef USE_DEBUG

  //if (hasFramBookmark())
  //loadFramBookmark();

#endif

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // PowerDown - самый экономный режим
  sleep_mode();                         // Переводим МК в сон

  readGyroData(gyroCount);
  getGres();
  // Calculate the gyro value into actual degrees per second
  gyrox = (float)gyroCount[0] * gRes;  // get actual gyro value, this depends on scale being set
  gyroy = (float)gyroCount[1] * gRes;
  gyroz = (float)gyroCount[2] * gRes;
  auto gyroApply = gyrox;  //gyroy for old panel driver2
  auto gyroMenu = gyroz;
  if (trigger) {
    if (abs(gyroApply) > abs(gyroMenu)) {
      if (gyroApply > threshold) {
        applyButton(applyRevert ? -1 : 1);
        trigger = false;
      }
      if (gyroApply < -threshold) {
        applyButton(applyRevert ? 1 : -1);
        trigger = false;
      }
    } else {
      if (gyroMenu > threshold) {

        menuButton(-1);
        trigger = false;
      }
      if (gyroMenu < -threshold) {

        menuButton(1);
        trigger = false;
      }
    }
    if (!trigger)
      delay(triggerDelay);
  }


  //  Serial.print(taps);
  //  Serial.print("         ,");  Serial.print(gyrox);
  // Serial.print(",");  Serial.print(gyroy);
  // Serial.print(",");  Serial.print(gyroz);  Serial.print(",");
  readdata = readByte(MPU6050_ADDRESS, 0x3A);
  //Serial.print(readdata);
  //Serial.println();

  /*if (btn1) {
    applyButton();
    btn1 = false;
   // delay(500);
  } else if (btn2) {
    btn2 = false;
    menuButton();
    //delay(500);
  }*/

  noInterrupts();  // to disable interrupts


  //delay(300);
  //detachInterrupt(digitalPinToInterrupt(buttonPin));
  //detachInterrupt(digitalPinToInterrupt(buttonPin3));
}
